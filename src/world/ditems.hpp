#ifndef WORLD_DITEMS_HPP
#define WORLD_DITEMS_HPP

#include "itembase.hpp"


namespace DF
{

struct DungeonInterior : public LocationHeader {
    struct Block {
        uint8_t mX;
        uint8_t mZ;
        union {
            uint16_t mBlockNumberStartIndex;
            struct {
                uint16_t mBlockPreIndex : 5;
                uint16_t mStartBlock : 1;
                uint16_t mBlockIdx : 10;
            };
        };
    };

    uint16_t mNullValue;
    uint16_t mUnknown1;
    uint16_t mUnknown2;
    uint32_t mBlockCount;
    uint8_t mUnknown3[5];

    std::vector<Block> mBlocks;

    void load(std::istream &stream);
};

} // namespace DF

#endif /* WORLD_DITEMS_HPP */
