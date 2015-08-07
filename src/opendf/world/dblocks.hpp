#ifndef WORLD_DBLOCKS_HPP
#define WORLD_DBLOCKS_HPP

#include <iostream>
#include <memory>
#include <vector>
#include <array>

#include "misc/sparsearray.hpp"
#include "referenceable.hpp"


namespace osg
{
    class Group;
    class Vec3f;
}

namespace DF
{

struct ObjectBase;
struct DBlockHeader;

struct ObjectBase {
    size_t mId;
    uint8_t mType;

    int32_t mXPos, mYPos, mZPos;

    ObjectBase(size_t id, uint8_t type, int x, int y, int z) : mId(id), mType(type), mXPos(x), mYPos(y), mZPos(z) { }
    virtual ~ObjectBase();

    void loadAction(std::istream &stream, int32_t actionoffset, uint32_t actionflags, uint8_t soundid, const osg::Vec3f &pos, const osg::Vec3f &rot);

    virtual void print(std::ostream &stream) const;
};
struct ModelObject;
struct FlatObject;

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

    Misc::SparseArray<std::unique_ptr<ModelObject>> mModels;
    Misc::SparseArray<std::unique_ptr<FlatObject>> mFlats;

    DBlockHeader();
    ~DBlockHeader();

    void load(std::istream &stream, size_t blockid, float x, float z, size_t regnum, size_t locnum, osg::Group *root);

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
