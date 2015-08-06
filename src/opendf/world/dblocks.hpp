#ifndef WORLD_DBLOCKS_HPP
#define WORLD_DBLOCKS_HPP

#include <iostream>
#include <vector>
#include <array>
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

    void loadAction(std::istream &stream);

    virtual void print(std::ostream &stream) const;
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

    uint32_t mUnknownOffset;
    uint32_t mUnknown4;
    uint32_t mUnknown5;
    uint32_t mUnknown6;

    std::vector<uint32_t> mUnknownList;

    /* Objects are stored in the files as an array of (width*height) root
     * offsets, which contain a linked list of objects. We merely use an array
     * of pointers to polymorphic objects, using its offset as a lookup. The
     * offset is used as an ID lookup for Action records that specify targets
     * as a byte offset.
     */
    Misc::SparseArray<ref_ptr<ObjectBase>> mObjects;

    osg::ref_ptr<osg::Group> mBaseNode;

    ~DBlockHeader();

    void load(std::istream &stream, size_t blockid, float x, float z, size_t regnum, size_t locnum, osg::Group *root);
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
