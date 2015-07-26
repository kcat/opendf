#ifndef WORLD_MBLOCKS_HPP
#define WORLD_MBLOCKS_HPP

#include <iostream>
#include <vector>
#include <array>

#include <osg/ref_ptr>

#include "pitems.hpp"


namespace osg
{
    class Group;
}

namespace DF
{

struct MObjectBase {
    int32_t mXPos, mYPos, mZPos;

    osg::ref_ptr<osg::Group> mBaseNode;

    void load(std::istream &stream);

    virtual void print(std::ostream &stream) const;
};

struct MSection3 : public MObjectBase {
    uint16_t mUnknown1;
    uint16_t mUnknown2;

    void load(std::istream &stream);
};

struct MDoor : public MObjectBase {
    uint16_t mUnknown1;
    int16_t mRotation;
    uint16_t mUnknown2;
    uint8_t mNullValue;

    void load(std::istream &stream);
};

struct MFlat : public MObjectBase {
    uint16_t mTexture;
    uint16_t mUnknown;
    uint8_t mFlags;

    void load(std::istream &stream);

    void buildNodes(osg::Group *root, size_t objid);

    virtual void print(std::ostream &stream) const;
};

struct MPerson : public MObjectBase {
    uint16_t mTexture;
    uint16_t mFactionId;

    void load(std::istream &stream);
};

struct MModel : public MObjectBase {
    uint32_t mModelIdx; /* le16*100 + byte */
    uint8_t  mUnknown1;
    uint32_t mUnknown2;
    uint32_t mUnknown3;
    uint32_t mUnknown4;
    uint32_t mNullValue1;
    uint32_t mNullValue2;
    int32_t  mUnknownX, mUnknownY, mUnknownZ;
    //int32_t  mXPos, mYPos, mZPos;
    uint32_t mNullValue3;
    int16_t  mYRotation;
    uint16_t mUnknown5;
    uint32_t mUnknown6;
    uint32_t mUnknown8;
    uint16_t mNullValue4;

    void load(std::istream &stream);

    void buildNodes(osg::Group *root, size_t objid);

    virtual void print(std::ostream &stream) const;
};

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

    std::vector<MModel>    mModels;
    std::vector<MFlat>     mFlats;
    std::vector<MSection3> mSection3s;
    std::vector<MPerson>   mNpcs;
    std::vector<MDoor>     mDoors;

    osg::ref_ptr<osg::Group> mBaseNode;

    void load(std::istream &stream);

    void buildNodes(osg::Group *root, size_t objid, int x, int z, int yrot);

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
    // Ground texture byte format: IRTTTTTT
    // I: Invert flag (flip texture on X/Y)
    // R: Rotate flag (rotate 90 degrees)
    // T: Texture index, [0-64). File depends on location and weather.
    std::array<uint8_t,256> mGroundTexture;
    std::array<uint8_t,256> mUnknown3;

    std::array<uint8_t,4096> mAutomap;

    std::vector<MBlock> mExteriorBlocks;
    std::vector<MBlock> mInteriorBlocks;

    std::vector<MModel> mModels;
    std::vector<MFlat> mFlats;

    osg::ref_ptr<osg::Group> mBaseNode;

    ~MBlockHeader();

    void load(std::istream &stream);

    void buildNodes(osg::Group *root, size_t objid, int x, int z);
    void detachNode();

    MObjectBase *getObject(size_t id);

    /* Object types are (apparently) identified by what Texture ID they use.
     * For instance, 0x638A (the 10th entry of TEXTURE.199) is the "Start"
     * marker, and is used to identify where to spawn.
     */
    const MFlat *getFlatByTexture(size_t texid) const;
};

} // namespace DF

#endif /* WORLD_MBLOCKS_HPP */
