#ifndef WORLD_DBLOCKS_HPP
#define WORLD_DBLOCKS_HPP

#include <iostream>
#include <vector>
#include <set>

#include <osg/ref_ptr>
#include <osg/Referenced>

#include "misc/sparsearray.hpp"
#include "referenceable.hpp"


namespace osg
{
    class Node;
    class Group;
    class MatrixTransform;
}

namespace DF
{

struct ObjectBase;
struct DBlockHeader;


enum ActionType {
    // Maybe flags?
    Action_Translate = 0x01,
    Action_Rotate    = 0x08,
    Action_Linker    = 0x1e
};

struct ActionBase : public Referenceable {
    ActionType mType;
    ObjectBase *mLink; // Linked object's action updates with this action

    float mTimeAccum;

    ActionBase(ActionType type, ObjectBase *link) : mType(type), mLink(link), mTimeAccum(0.0f)
    { }

    virtual void load(const std::array<uint8_t,5> &data) = 0;
    virtual bool update(ObjectBase *target, float timediff) = 0;

    virtual void print(std::ostream &stream) const;
};

struct ActionMovable : public ActionBase {
    uint8_t mAxis;
    float mDuration;
    uint16_t mMagnitude;

    ActionMovable(ActionType type, ObjectBase *link) : ActionBase(type, link) { }
    virtual void load(const std::array<uint8_t,5> &data) final;

    virtual void print(std::ostream &stream) const final;
};

struct ActionTranslate : public ActionMovable {
    enum Axis {
        Axis_X    = 0x01,
        Axis_NegX = 0x02,
        Axis_Y    = 0x03,
        Axis_NegY = 0x04,
        Axis_Z    = 0x05,
        Axis_NegZ = 0x06
    };

    ActionTranslate(ObjectBase *link) : ActionMovable(Action_Translate, link) { }
    virtual bool update(ObjectBase *target, float timediff) final;
};
struct ActionRotate : public ActionMovable {
    enum Axis {
        Axis_X    = 0x01,
        Axis_NegX = 0x02,
        Axis_NegY = 0x03,
        Axis_Y    = 0x04,
        Axis_NegZ = 0x05,
        Axis_Z    = 0x06
    };

    ActionRotate(ObjectBase *link) : ActionMovable(Action_Rotate, link) { }
    virtual bool update(ObjectBase *target, float timediff) final;
};

/* Linker actions are used for actions that don't directly affect the object
 * it's attached to, but still trigger linked object actions.
 */
struct ActionLinker : public ActionBase {
    ActionLinker(ObjectBase *link) : ActionBase(Action_Linker, link) { }
    virtual void load(const std::array<uint8_t,5> &data) final;
    virtual bool update(ObjectBase *target, float timediff) final;
};

struct ActionUnknown : public ActionBase {
    std::array<uint8_t,5> mData;

    ActionUnknown(uint8_t type, ObjectBase *link) : ActionBase((ActionType)type, link) { }
    virtual void load(const std::array<uint8_t,5> &data) final;
    virtual bool update(ObjectBase *target, float timediff) final;

    virtual void print(std::ostream &stream) const final;
};


enum ObjectType {
    ObjectType_Model = 0x01,
    ObjectType_Light = 0x02,
    ObjectType_Flat = 0x03,
};

enum ActionFlags {
    // Specifies if the object can be directly activated (otherwise only via an action link)
    ActionFlag_Activatable = 0x02,
};

struct ObjectBase : public Referenceable {
    size_t mId;
    uint8_t mType;

    ref_ptr<ActionBase> mAction;
    bool mActive;
    bool mReverse;

    int32_t mXPos, mYPos, mZPos;
    int32_t mXRot, mYRot, mZRot;
    uint32_t mActionFlags;
    int32_t mActionOffset;

    ObjectBase(size_t id, uint8_t type, int x, int y, int z);
    virtual ~ObjectBase();

    void loadAction(std::istream &stream, DBlockHeader &block);
    bool updateAction(float timediff) { return mAction->update(this, timediff); }

    virtual void buildNodes(osg::Group *root) = 0;

    virtual void print(std::ostream &stream) const;
};

struct ModelObject : public ObjectBase {
    //int32_t mXRot, mYRot, mZRot;

    uint16_t mModelIdx;
    //uint32_t mActionFlags;
    uint8_t mSoundId; // Played when activated
    //int32_t  mActionOffset;

    std::array<char,8> mModelData;

    ModelObject(size_t id, int x, int y, int z) : ObjectBase(id, ObjectType_Model, x, y, z) { }
    void load(std::istream &stream, const std::array<std::array<char,8>,750> &mdldata);

    virtual void buildNodes(osg::Group *root) final;

    virtual void print(std::ostream &stream) const final;
};

struct FlatObject : public ObjectBase {
    uint16_t mTexture;
    uint16_t mGender; // Flags?
    uint16_t mFactionId;
    //int32_t mActionOffset; // Maybe?
    uint8_t mUnknown;

    FlatObject(size_t id, int x, int y, int z) : ObjectBase(id, ObjectType_Flat, x, y, z) { }
    void load(std::istream &stream);

    virtual void buildNodes(osg::Group *root) final;

    virtual void print(std::ostream &stream) const final;
};
enum {
    Marker_EnterID = 0x6388,
    Marker_StartID = 0x638A
};

struct DBlockHeader {
    uint32_t mUnknown1;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mObjectRootOffset;
    uint32_t mUnknown2;

    std::array<std::array<char,8>,750> mModelData;
    std::array<uint32_t,750> mUnknown3;

    /* Objects are stored in the files as an array of (width*height) root
     * offsets, which contain a linked list of objects. We merely use an array
     * of pointers to polymorphic objects, using its offset as a lookup. The
     * offset is used as an ID lookup for Action records that specify targets
     * as a byte offset.
     */
    Misc::SparseArray<ref_ptr<ObjectBase>> mObjects;

    osg::ref_ptr<osg::Group> mBaseNode;

    std::vector<std::pair<ref_ptr<ActionBase>,ObjectBase*>> mActiveObjects;

    ~DBlockHeader();

    void load(std::istream &stream, size_t blockid);

    void buildNodes(osg::Group *root, int x, int z);
    void detachNode();

    ObjectBase *getObject(size_t id);

    /* Object types are (apparently) identified by what Texture ID they use.
     * For instance, 0x638A (the 10th entry of TEXTURE.199) is the "Start"
     * marker, and is used to identify where to spawn when entering a dungeon.
     */
    size_t getObjectByTexture(size_t texid) const;

    void activate(size_t id);

    void update(float timediff);

    void print(std::ostream &stream, int objtype=0) const;
};


} // namespace DF

#endif /* WORLD_DBLOCKS_HPP */
