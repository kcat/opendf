
#include "ditems.hpp"

#include <iomanip>

#include "components/vfs/manager.hpp"

#include "log.hpp"


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

LogStream& operator<<(LogStream &stream, const DungeonInterior &dgn)
{
    stream<< static_cast<const LocationHeader&>(dgn) <<"\n";
    stream<<std::setfill('0');

    stream<< "  Null: "<<dgn.mNullValue<<"\n";
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<dgn.mUnknown1<<std::setw(0)<<std::dec<<"\n";
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<dgn.mUnknown2<<std::setw(0)<<std::dec<<"\n";
    stream<< "  BlockCount: "<<dgn.mBlockCount<<"\n";
    stream<< "  Unknown:";//<<std::hex<<std::setw(2);
    for(uint32_t unk : dgn.mUnknown3)
        stream<< " 0x"<<std::hex<<std::setw(2)<<unk;
    stream<<std::setw(0)<<std::dec<<"\n";

    for(const DungeonBlock &block : dgn.mBlocks)
    {
        stream<< "  Block "<<std::distance(dgn.mBlocks.data(), &block)<<":\n";
        stream<< "    X: "<<(int)block.mX<<"\n";
        stream<< "    Z: "<<(int)block.mZ<<"\n";
        stream<< "    BlockIdx: "<<block.mBlockIdx<<"\n";
        stream<< "    StartBlock: "<<block.mStartBlock<<"\n";
        stream<< "    BlockPreIndex: "<<block.mBlockPreIndex<<"\n";
    }

    stream<<std::setfill(' ');
    return stream;
}

} // namespace DF
