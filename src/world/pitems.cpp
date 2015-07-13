
#include "pitems.hpp"

#include "components/vfs/manager.hpp"


namespace DF
{

void ExteriorLocation::load(std::istream& stream)
{
    LocationHeader::load(stream);

    uint16_t mBuildingCount = VFS::read_le16(stream);
    stream.read(reinterpret_cast<char*>(mUnknown1), sizeof(mUnknown1));

    mBuildings.resize(mBuildingCount);
    for(Building &building : mBuildings)
    {
        building.mNameSeed = VFS::read_le16(stream);
        building.mNullValue1 = VFS::read_le32(stream);
        building.mNullValue2 = VFS::read_le32(stream);
        building.mNullValue3 = VFS::read_le32(stream);
        building.mNullValue4 = VFS::read_le32(stream);
        building.mFactionId = VFS::read_le16(stream);
        building.mSector = VFS::read_le16(stream);
        building.mLocationId = VFS::read_le16(stream);
        building.mBuildingType = stream.get();
        building.mQuality = stream.get();
    }

    stream.read(mName, sizeof(mName));
    mMapId = VFS::read_le32(stream);
    mUnknown2 = VFS::read_le32(stream);
    mWidth = stream.get();
    mHeight = stream.get();
    stream.read(reinterpret_cast<char*>(mUnknown3), sizeof(mUnknown3));
    stream.read(reinterpret_cast<char*>(mBlockIndex), sizeof(mBlockIndex));
    stream.read(reinterpret_cast<char*>(mBlockNumber), sizeof(mBlockNumber));
    stream.read(reinterpret_cast<char*>(mBlockCharacter), sizeof(mBlockCharacter));
    stream.read(reinterpret_cast<char*>(mUnknown4), sizeof(mUnknown4));
    mNullValue1 = VFS::read_le32(stream);
    mNullValue2 = VFS::read_le32(stream);
    mNullValue3 = stream.get();
    for(uint32_t &val : mUnknown5)
        val = VFS::read_le32(stream);
    stream.read(reinterpret_cast<char*>(mNullValue4), sizeof(mNullValue4));
    mUnknown6 = VFS::read_le32(stream);
}

} // namespace DF
