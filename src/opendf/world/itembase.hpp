#ifndef WORLD_ITEMBASE_HPP
#define WORLD_ITEMBASE_HPP

#include <vector>
#include <iostream>
#include <stdint.h>


namespace DF
{

class LogStream;

struct LocationDoor {
    uint16_t mBuildingDataIndex;
    uint8_t mNullValue;
    uint8_t mUnknownMask;
    uint8_t mUnknown1;
    uint8_t mUnknown2;
};

struct LocationHeader {

    uint32_t mDoorCount;
    std::vector<LocationDoor> mDoors;

    uint32_t mAlwaysOne1;
    uint16_t mNullValue1;
    uint8_t mNullValue2;
    int32_t mY;
    uint32_t mNullValue3;
    int32_t mX;
    uint16_t mIsExterior;
    uint16_t mNullValue4;
    uint32_t mUnknown1;
    uint32_t mUnknown2;
    uint16_t mAlwaysOne2;
    uint16_t mLocationId;
    uint32_t mNullValue5;
    uint16_t mIsInterior;
    uint32_t mExteriorLocationId;
    uint8_t mNullValue6[26];
    char mLocationName[32];
    uint8_t mUnknown3[9];

    void load(std::istream &stream);
};
LogStream& operator<<(LogStream &stream, const LocationHeader &loc);

} // namespace DF

#endif /* WORLD_ITEMBASE_HPP */
