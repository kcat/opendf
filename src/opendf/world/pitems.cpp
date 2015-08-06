
#include "pitems.hpp"

#include <sstream>
#include <iomanip>
#include <array>

#include "components/vfs/manager.hpp"

#include "log.hpp"


namespace
{

static const std::array<std::array<char,5>,45> gBuildingLabel{{
    {"TVRN"},
    {"GENR"},
    {"RESI"},
    {"WEAP"},
    {"ARMR"},
    {"ALCH"},
    {"BANK"},
    {"BOOK"},
    {"CLOT"},
    {"FURN"},
    {"GEMS"},
    {"LIBR"},
    {"PAWN"},
    {"TEMP"},
    {"TEMP"},
    {"PALA"},
    {"FARM"},
    {"DUNG"},
    {"CAST"},
    {"MANR"},
    {"SHRI"},
    {"RUIN"},
    {"SHCK"},
    {"GRVE"},
    {"FILL"},
    {"KRAV"},
    {"KDRA"},
    {"KOWL"},
    {"KMOO"},
    {"KCAN"},
    {"KFLA"},
    {"KHOR"},
    {"KROS"},
    {"KWHE"},
    {"KSCA"},
    {"KHAW"},
    {"MAGE"},
    {"THIE"},
    {"DARK"},
    {"FIGH"},
    {"CUST"},
    {"WALL"},
    {"MARK"},
    {"SHIP"},
    {"WITC"}
}};

}

namespace DF
{

void ExteriorLocation::load(std::istream& stream)
{
    LocationHeader::load(stream);

    uint16_t mBuildingCount = VFS::read_le16(stream);
    stream.read(reinterpret_cast<char*>(mUnknown1), sizeof(mUnknown1));

    mBuildings.resize(mBuildingCount);
    for(ExteriorBuilding &building : mBuildings)
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
    stream.read(mName2, sizeof(mName2));
    mUnknown4 = stream.get();
    mUnknownCount = stream.get();
    mNullValue1 = VFS::read_le32(stream);
    mNullValue2 = VFS::read_le32(stream);
    mNullValue3 = stream.get();
    for(uint32_t &val : mUnknown5)
        val = VFS::read_le32(stream);
    stream.read(reinterpret_cast<char*>(mNullValue4), sizeof(mNullValue4));
    mUnknown6 = VFS::read_le32(stream);
}

std::string ExteriorLocation::getMapBlockName(size_t idx, size_t regnum) const
{
    std::stringstream name;

    name<< gBuildingLabel.at(mBlockIndex[idx]).data();
    if(mBlockIndex[idx] == 13 || mBlockIndex[idx] == 14)
    {
        if(mBlockCharacter[idx] > 0x07)
            name<< "GA";
        else
            name<< "AA";
        static const char numbers[8][3] = {
            "A0", "B0", "C0", "D0", "E0", "F0", "G0", "H0"
        };
        name<< numbers[mBlockCharacter[idx]&0x07];
    }
    else
    {
        size_t q = mBlockCharacter[idx] >> 4;
        if(regnum == 23 /* Wayrest special-case */)
        {
            if(mBlockIndex[idx] == 40)
                q = 0;
            else if(q > 0)
                --q;
        }
        else if(regnum == 20 /* Sentinel special-case */)
        {
            if(mBlockIndex[idx] == 40)
                q = 8;
        }
        else
        {
            if(mBlockIndex[idx] == 40)
                q = 0;
        }
        const std::array<std::array<char,3>,12> letters{{
            {"AA"}, {"BA"}, {"AL"}, {"BL"}, {"AM"}, {"BM"},
            {"AS"}, {"BS"}, {"GA"}, {"GL"}, {"GM"}, {"GS"}
        }};
        name<< letters.at(q).data();
        name<< std::setfill('0')<<std::setw(2)<<(int)mBlockNumber[idx];
    }
    name<< ".RMB";

    return name.str();
}

LogStream& operator<<(LogStream &stream, const ExteriorLocation &ext)
{
    stream<< static_cast<const LocationHeader&>(ext) <<"\n";
    stream<<std::setfill('0');

    stream<< "  BuildingCount: "<<ext.mBuildingCount<<"\n";
    stream<< "  Unknown:";//<<std::hex<<std::setw(2);
    for(uint32_t unk : ext.mUnknown1)
        stream<< " 0x"<<std::hex<<std::setw(2)<<unk;
    stream<<std::setw(0)<<std::dec<<"\n";

    for(size_t i = 0;i < ext.mBuildingCount;++i)
    {
        const ExteriorBuilding &building = ext.mBuildings[i];
        stream<< "  Building "<<i<<"\n";
        stream<< "    NameSeed: 0x"<<std::hex<<std::setw(4)<<building.mNameSeed<<std::setw(0)<<std::dec<<"\n";
        stream<< "    FactionId: 0x"<<std::hex<<std::setw(4)<<building.mFactionId<<std::setw(0)<<std::dec<<"\n";
        stream<< "    Sector: 0x"<<std::hex<<std::setw(4)<<building.mSector<<std::setw(0)<<std::dec<<"\n";
        stream<< "    LocationId: 0x"<<std::hex<<std::setw(4)<<building.mLocationId<<std::setw(0)<<std::dec<<"\n";
        stream<< "    BuildingType: 0x"<<std::hex<<std::setw(2)<<(int)building.mBuildingType<<std::setw(0)<<std::dec<<"\n";
        stream<< "    Quality: 0x"<<std::hex<<std::setw(2)<<(int)building.mQuality<<std::setw(0)<<std::dec<<"\n";
    }

    stream<< "  Name: \""<<ext.mName<<"\"\n";
    stream<< "  MapId: 0x"<<std::hex<<std::setw(8)<<ext.mMapId<<std::setw(0)<<std::dec<<"\n";
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<ext.mUnknown2<<std::setw(0)<<std::dec<<"\n";
    stream<< "  Width: "<<(int)ext.mWidth<<"\n";
    stream<< "  Height: "<<(int)ext.mHeight<<"\n";
    stream<< "  Unknown:";
    for(uint32_t unk : ext.mUnknown3)
        stream<< " 0x"<<std::hex<<std::setw(2)<<unk;
    stream<<std::setw(0)<<std::dec<<"\n";
    /*uint8_t  mBlockIndex[64];
    uint8_t  mBlockNumber[64];
    uint8_t  mBlockCharacter[64];*/
    stream<< "  Name2: \""<<ext.mName2<<"\"\n";
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(2)<<(int)ext.mUnknown4<<std::setw(0)<<std::dec<<"\n";
    stream<< "  UnknownCount: "<<(int)ext.mUnknownCount<<"\n";
    /*uint32_t mNullValue1;
    uint32_t mNullValue2;
    uint8_t  mNullValue3;*/
    stream<< "  Unknown:";
    for(uint32_t unk : ext.mUnknown5)
        stream<< " 0x"<<std::hex<<std::setw(8)<<unk;
    stream<<std::setw(0)<<std::dec<<"\n";
    /*uint8_t  mNullValue4[40];*/
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<ext.mUnknown6<<std::setw(0)<<std::dec<<"\n";

    stream<<std::setfill(' ');
    return stream;
}

} // namespace DF
