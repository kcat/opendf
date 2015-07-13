#ifndef WORLD_DITEMS_HPP
#define WORLD_DITEMS_HPP

#include "itembase.hpp"


namespace DF
{

struct DungeonBlock {
    int8_t mX;
    int8_t mZ;
    union {
        uint16_t mBlockNumberStartIndex;
        struct {
            uint16_t mBlockIdx : 10;
            uint16_t mStartBlock : 1;
            uint16_t mBlockPreIndex : 5;
        };
    };
};

struct DungeonInterior : public LocationHeader {
    uint16_t mNullValue;
    uint32_t mUnknown1;
    uint32_t mUnknown2;
    uint16_t mBlockCount;
    uint8_t mUnknown3[5];

    std::vector<DungeonBlock> mBlocks;

    void load(std::istream &stream);
};

} // namespace DF

#endif /* WORLD_DITEMS_HPP */
