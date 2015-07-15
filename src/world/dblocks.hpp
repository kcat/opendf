#ifndef WORLD_DBLOCKS_HPP
#define WORLD_DBLOCKS_HPP

#include <iostream>
#include <vector>
#include <set>

#include <osg/ref_ptr>

#include "referenceable.hpp"


namespace osg
{
    class Node;
    class Group;
}

namespace DF
{

class LogStream;

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

    virtual void buildNodes(osg::Group *root) = 0;
    virtual void print(LogStream &stream) const;
};

struct ModelObject : public ObjectBase {
    int32_t mXRot, mYRot, mZRot;

    uint32_t mModelIdx; // Was uint16_t
    uint32_t mUnknown1;
    uint8_t  mUnknown2;
    int32_t  mActionOffset;

    ModelObject(int x, int y, int z) : ObjectBase(ObjectType_Model, x, y, z) { }
    void load(std::istream &stream, const std::array<int,750> &mdlidx);

    virtual void buildNodes(osg::Group *root) final;
    virtual void print(LogStream &stream) const final;
};

struct FlatObject : public ObjectBase {
    uint16_t mTexture;
    uint16_t mGender; // Flags?
    uint16_t mFactionId;
    uint8_t mUnknown[5];

    FlatObject(int x, int y, int z) : ObjectBase(ObjectType_Flat, x, y, z) { }
    void load(std::istream &stream);

    virtual void buildNodes(osg::Group *root) final;
    virtual void print(LogStream &stream) const final;
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

    void buildNodes(osg::Group *root, int x, int z);
    void detachNode();

    void print(LogStream &stream, int objtype=0) const;
};


} // namespace DF

#endif /* WORLD_DBLOCKS_HPP */
