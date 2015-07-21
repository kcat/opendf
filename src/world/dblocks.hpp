#ifndef WORLD_DBLOCKS_HPP
#define WORLD_DBLOCKS_HPP

#include <iostream>
#include <vector>
#include <set>

#include <osg/ref_ptr>
#include <osg/Referenced>

#include "referenceable.hpp"


namespace osg
{
    class Node;
    class Group;
}

namespace DF
{

class LogStream;

class ObjectRef : public osg::Referenced {
    size_t mId;

public:
    ObjectRef(size_t id) : mId(id) { }

    size_t getId() const { return mId; }
};

enum ObjectType {
    ObjectType_Model = 0x01,
    ObjectType_Light = 0x02,
    ObjectType_Flat = 0x03,
};

struct ObjectBase : public Referenceable {
    osg::ref_ptr<osg::Group> mBaseNode;

    uint8_t mType;
    int32_t mXPos, mYPos, mZPos;

    ObjectBase(uint8_t type, int x, int y, int z);

    virtual void buildNodes(osg::Group *root, size_t objid) = 0;
    virtual void print(LogStream &stream) const;
    virtual void print(std::ostream &stream) const;
};

struct ModelObject : public ObjectBase {
    int32_t mXRot, mYRot, mZRot;

    uint16_t mModelIdx;
    uint32_t mUnknown1;
    uint8_t  mUnknown2;
    int32_t  mActionOffset;

    std::array<char,8> mModelData;

    ModelObject(int x, int y, int z) : ObjectBase(ObjectType_Model, x, y, z) { }
    void load(std::istream &stream, const std::array<std::array<char,8>,750> &mdldata);

    virtual void buildNodes(osg::Group *root, size_t objid) final;
    virtual void print(LogStream &stream) const final;
    virtual void print(std::ostream &stream) const final;
};

struct FlatObject : public ObjectBase {
    uint16_t mTexture;
    uint16_t mGender; // Flags?
    uint16_t mFactionId;
    uint32_t mActionOffset; // Maybe?
    uint8_t mUnknown;

    FlatObject(int x, int y, int z) : ObjectBase(ObjectType_Flat, x, y, z) { }
    void load(std::istream &stream);

    virtual void buildNodes(osg::Group *root, size_t objid) final;
    virtual void print(LogStream &stream) const final;
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
    uint32_t mobjectRootOffset;
    uint32_t mUnknown2;

    std::array<std::array<char,8>,750> mModelData;
    std::array<uint32_t,750> mUnknown3;

    /* Objects are stored in the files as an array of (width*height) root
     * offsets, which contain a linked list of objects. We merely use an array
     * of pointers to polymorphic objects, using its offset as a lookup. The
     * offset is used as an ID lookup for Action records that specify targets
     * as a byte offset.
     */
    std::set<int32_t> mObjectIds;
    std::vector<ref_ptr<ObjectBase>> mObjects;

    osg::ref_ptr<osg::Group> mBaseNode;

    ~DBlockHeader();

    void load(std::istream &stream);

    void buildNodes(osg::Group *root, size_t blockid, int x, int z);
    void detachNode();

    ObjectBase *getObject(size_t id);

    /* Object types are (apparently) identified by what Texture ID they use.
     * For instance, 0x638A (the 10th entry of TEXTURE.199) is the "Start"
     * marker, and is used to identify where to spawn when entering a dungeon.
     */
    FlatObject *getFlatByTexture(size_t texid) const;

    void print(LogStream &stream, int objtype=0) const;
};


} // namespace DF

#endif /* WORLD_DBLOCKS_HPP */
