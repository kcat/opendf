
#include "ditems.hpp"


#include "components/vfs/manager.hpp"


namespace DF
{

void DungeonInterior::load(std::istream &stream)
{
    LocationHeader::load(stream);

    mNullValue = VFS::read_le16(stream);
    mUnknown1 = VFS::read_le32(stream);
    mUnknown2 = VFS::read_le32(stream);
    mBlockCount = VFS::read_le16(stream);
    stream.read(reinterpret_cast<char*>(mUnknown3), sizeof(mUnknown3));

    mBlocks.resize(mBlockCount);
    for(DungeonBlock &block : mBlocks)
    {
        block.mX = stream.get();
        block.mZ = stream.get();
        block.mBlockNumberStartIndex = VFS::read_le16(stream);
    }
}

} // namespace DF
