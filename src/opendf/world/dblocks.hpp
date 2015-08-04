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

struct ActionTranslate {
    enum Axis {
        Axis_X    = 0x01,
        Axis_NegX = 0x02,
        Axis_Y    = 0x03,
        Axis_NegY = 0x04,
        Axis_Z    = 0x05,
        Axis_NegZ = 0x06
    };
};
struct ActionRotate {
    enum Axis {
        Axis_X    = 0x01,
        Axis_NegX = 0x02,
        Axis_NegY = 0x03,
        Axis_Y    = 0x04,
        Axis_NegZ = 0x05,
        Axis_Z    = 0x06
    };
};


enum ObjectType {
    ObjectType_Model = 0x01,
    ObjectType_Light = 0x02,
    ObjectType_Flat = 0x03,
};

struct ObjectBase : public Referenceable {
    size_t mId;
    uint8_t mType;

    int32_t mXPos, mYPos, mZPos;
    int32_t mXRot, mYRot, mZRot;
    uint32_t mActionFlags;
    uint8_t mSoundId; // Played when activated
    int32_t mActionOffset;

    ObjectBase(size_t id, uint8_t type, int x, int y, int z);
    virtual ~ObjectBase();

    void loadAction(std::istream &stream, DBlockHeader &block);

    virtual void buildNodes(osg::Group *root) = 0;

    virtual void print(std::ostream &stream) const;
};

struct ModelObject : public ObjectBase {
    //int32_t mXRot, mYRot, mZRot;

    uint16_t mModelIdx;
    //uint32_t mActionFlags;
    //uint8_t mSoundId;
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

    void print(std::ostream &stream, int objtype=0) const;
};


} // namespace DF

#endif /* WORLD_DBLOCKS_HPP */
