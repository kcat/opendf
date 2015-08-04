#ifndef WORLD_PITEMS_HPP
#define WORLD_PITEMS_HPP

#include "itembase.hpp"


namespace DF
{

struct ExteriorBuilding {
    uint16_t mNameSeed;
    uint32_t mNullValue1;
    uint32_t mNullValue2;
    uint32_t mNullValue3;
    uint32_t mNullValue4;
    uint16_t mFactionId;
    int16_t  mSector;
    uint16_t mLocationId;
    uint8_t  mBuildingType;
    uint8_t  mQuality;
};

struct ExteriorLocation : public LocationHeader {
    uint16_t mBuildingCount;
    uint8_t  mUnknown1[5];

    std::vector<ExteriorBuilding> mBuildings;

    char mName[32]; // Unused?
    int32_t  mMapId; // 20 bits only
    uint32_t mUnknown2;
    uint8_t  mWidth;
    uint8_t  mHeight;
    uint8_t  mUnknown3[7];
    uint8_t  mBlockIndex[64];
    uint8_t  mBlockNumber[64];
    uint8_t  mBlockCharacter[64];
    uint8_t  mUnknown4[34];
    uint32_t mNullValue1;
    uint32_t mNullValue2;
    uint8_t  mNullValue3;
    uint32_t mUnknown5[32];
    uint8_t  mNullValue4[40];
    uint32_t mUnknown6;

    void load(std::istream &stream);

    std::string getMapBlockName(size_t idx, size_t regnum) const;
};

}

#endif /* WORLD_PITEMS_HPP */
