#ifndef WORLD_MBLOCKS_HPP
#define WORLD_MBLOCKS_HPP

#include <iostream>
#include <vector>
#include <array>

#include <osg/ref_ptr>

#include "misc/sparsearray.hpp"

#include "pitems.hpp"


namespace osg
{
    class Group;
}

namespace DF
{

struct MObjectBase {
    size_t mId;

    int32_t mXPos, mYPos, mZPos;

    void load(std::istream &stream);

    virtual void print(std::ostream &stream) const;
};
struct MModel;
struct MFlat;
struct MSection3;
struct MPerson;
struct MDoor;

struct MBlock {
    uint8_t  mModelCount;
    uint8_t  mFlatCount;
    uint8_t  mSection3Count;
    uint8_t  mPersonCount;
    uint8_t  mDoorCount;
    uint16_t mUnknown1;
    uint16_t mUnknown2;
    uint16_t mUnknown3;
    uint16_t mUnknown4;
    uint16_t mUnknown5;
    uint16_t mUnknown6;

    Misc::SparseArray<MModel> mModels;
    Misc::SparseArray<MFlat>  mFlats;
    std::vector<MSection3> mSection3s;
    std::vector<MPerson>   mNpcs;
    std::vector<MDoor>     mDoors;

    osg::ref_ptr<osg::Group> mBaseNode;

    MBlock();
    ~MBlock();

    void load(std::istream &stream, size_t blockid, int x, int z, int yrot, osg::Group *root);

    void allocate();
    void deallocate();

    MObjectBase *getObject(size_t id);
};

struct MBlockPosition {
    uint32_t mUnknown1;
    uint32_t mUnknown2;
    int32_t mX;
    int32_t mZ;
    int32_t mYRot;

    void load(std::istream &stream);
};

struct MBlockHeader {
    uint8_t mBlockCount;
    uint8_t mModelCount;
    uint8_t mFlatCount;

    std::array<MBlockPosition,32> mBlockPositions;
    std::array<ExteriorBuilding,32> mBuildings;
    std::array<uint32_t,32> mUnknown1;
    std::array<uint32_t,32> mBlockSizes;

    std::array<uint8_t,8> mUnknown2;
    // Ground texture byte format: RRTTTTTT
    // R: Rotation (90 degree increments)
    // T: Texture index, [0-64). File depends on location and weather.
    std::array<uint8_t,256> mGroundTexture;
    std::array<uint8_t,256> mUnknown3;

    std::array<uint8_t,4096> mAutomap;

    std::vector<MBlock> mExteriorBlocks;
    std::vector<MBlock> mInteriorBlocks;

    Misc::SparseArray<MModel> mModels;
    Misc::SparseArray<MFlat> mFlats;

    osg::ref_ptr<osg::Group> mBaseNode;

    MBlockHeader();
    ~MBlockHeader();
    void deallocate();

    void load(std::istream &stream, size_t blockid, float x, float z, osg::Group *root);
    void detachNode();

    MObjectBase *getObject(size_t id);

    /* Object types are (apparently) identified by what Texture ID they use.
     * For instance, 0x638A (the 10th entry of TEXTURE.199) is the "Start"
     * marker, and is used to identify where to spawn.
     */
    size_t getObjectByTexture(size_t texid) const;
};

} // namespace DF

#endif /* WORLD_MBLOCKS_HPP */
