
#include "world.hpp"

#include <sstream>
#include <iomanip>

#include <osgViewer/Viewer>
#include <osg/Light>

#include "components/vfs/manager.hpp"

#include "log.hpp"


namespace
{

static const char gBlockIndexLabel[6] = { 'N', 'W', 'L', 'S', 'B', 'M' };

/* This is only stored temporarily */
struct DungeonHeader {
    struct Offset {
        uint32_t mOffset;
        uint16_t mIsDungeon;
        uint16_t mExteriorLocationId;
    };

    uint32_t mDungeonCount;
    std::vector<Offset> mOffsets;

    void load(std::istream &stream)
    {
        mDungeonCount = VFS::read_le32(stream);

        mOffsets.resize(mDungeonCount);
        for(Offset &offset : mOffsets)
        {
            offset.mOffset = VFS::read_le32(stream);
            offset.mIsDungeon = VFS::read_le16(stream);
            offset.mExteriorLocationId = VFS::read_le16(stream);
        }
    }
};

}

namespace DF
{

void LocationHeader::load(std::istream &stream)
{
    mDoorCount = VFS::read_le32(stream);

    mDoors.resize(mDoorCount);
    for(auto &door : mDoors)
    {
        door.mBuildingDataIndex = VFS::read_le16(stream);
        door.mNullValue = stream.get();
        door.mUnknownMask = stream.get();
        door.mUnknown1 = stream.get();
        door.mUnknown2 = stream.get();
    }

    mAlwaysOne1 = VFS::read_le32(stream);
    mNullValue1 = VFS::read_le16(stream);
    mNullValue2 = stream.get();
    mY = VFS::read_le32(stream);
    mNullValue3 = VFS::read_le32(stream);
    mX = VFS::read_le32(stream);
    mIsExterior = VFS::read_le16(stream);
    mNullValue4 = VFS::read_le16(stream);
    mUnknown1 = VFS::read_le32(stream);
    mUnknown2 = VFS::read_le32(stream);
    mAlwaysOne2 = VFS::read_le16(stream);
    mLocationId = VFS::read_le16(stream);
    mNullValue5 = VFS::read_le32(stream);
    mIsInterior = VFS::read_le16(stream);
    mExteriorLocationId = VFS::read_le32(stream);
    stream.read(reinterpret_cast<char*>(mNullValue6), sizeof(mNullValue6));
    stream.read(mLocationName, sizeof(mLocationName));
    stream.read(reinterpret_cast<char*>(mUnknown3), sizeof(mUnknown3));
}


World World::sWorld;
WorldIface &WorldIface::sInstance = World::sWorld;

World::World()
{
}

World::~World()
{
}


void World::initialize(osgViewer::Viewer *viewer)
{
    std::set<std::string> names = VFS::Manager::get().list("MAPNAMES.[0-9]*");
    if(names.empty()) throw std::runtime_error("Failed to find any regions");

    for(const std::string &name : names)
    {
        size_t pos = name.rfind('.');
        if(pos == std::string::npos)
            continue;
        std::string regstr = name.substr(pos+1);
        unsigned long regnum = std::stoul(regstr, nullptr, 10);

        /* Get names */
        VFS::IStreamPtr stream = VFS::Manager::get().open(name.c_str());
        if(!stream) throw std::runtime_error("Failed to open "+name);

        uint32_t mapcount = VFS::read_le32(*stream);
        if(mapcount == 0) continue;

        MapRegion region;
        region.mNames.resize(mapcount);
        for(std::string &mapname : region.mNames)
        {
            char mname[32];
            if(!stream->read(mname, sizeof(mname)) || stream->gcount() != sizeof(mname))
                throw std::runtime_error("Failed to read map names from "+name);
            mapname.assign(mname, sizeof(mname));
            size_t end = mapname.find('\0');
            if(end != std::string::npos)
                mapname.resize(end);
        }
        stream = nullptr;

        /* Get table data */
        std::string fname = "MAPTABLE."+regstr;
        stream = VFS::Manager::get().open(fname.c_str());
        if(!stream) throw std::runtime_error("Failed to open "+fname);

        region.mTable.resize(region.mNames.size());
        for(MapTable &maptable : region.mTable)
        {
            maptable.mMapId = VFS::read_le32(*stream);
            maptable.mUnknown1 = stream->get();
            maptable.mLongitudeType = VFS::read_le32(*stream);
            maptable.mLatitude = VFS::read_le16(*stream);
            maptable.mUnknown2 = VFS::read_le16(*stream);
            maptable.mUnknown3 = VFS::read_le32(*stream);
        }
        stream = nullptr;

        /* Get exterior data */
        fname = "MAPPITEM."+regstr;
        stream = VFS::Manager::get().open(fname.c_str());
        if(!stream) throw std::runtime_error("Failed to open "+fname);

        std::vector<uint32_t> extoffsets(region.mNames.size());
        for(uint32_t &offset : extoffsets)
            offset = VFS::read_le32(*stream);
        std::streamoff extbase_offset = stream->tellg();

        uint32_t *extoffset = extoffsets.data();
        region.mExteriors.resize(extoffsets.size());
        for(ExteriorLocation &extinfo : region.mExteriors)
        {
            stream->seekg(extbase_offset + *extoffset);
            extinfo.load(*stream);
            ++extoffset;
        }
        stream = nullptr;

        /* Get dungeon data */
        fname = "MAPDITEM."+regstr;
        stream = VFS::Manager::get().open(fname.c_str());
        if(!stream) throw std::runtime_error("Failed to open "+fname);

        DungeonHeader dheader;
        dheader.load(*stream);
        std::streamoff dbase_offset = stream->tellg();

        DungeonHeader::Offset *doffset = dheader.mOffsets.data();
        region.mDungeons.resize(dheader.mDungeonCount);
        for(DungeonInterior &dinfo : region.mDungeons)
        {
            stream->seekg(dbase_offset + doffset->mOffset);
            dinfo.load(*stream);
            if(dinfo.mExteriorLocationId != doffset->mExteriorLocationId)
                throw std::runtime_error("Dungeon exterior location id mismatch for "+std::string(dinfo.mLocationName)+": "+
                    std::to_string(dinfo.mExteriorLocationId)+" / "+std::to_string(doffset->mExteriorLocationId));
            ++doffset;
        }
        stream = nullptr;

        if(regnum >= mRegions.size()) mRegions.resize(regnum+1);
        mRegions[regnum] = std::move(region);
    }

    mViewer = viewer;

    mViewer->setLightingMode(osg::View::HEADLIGHT);
    osg::ref_ptr<osg::Light> light(new osg::Light());
    light->setAmbient(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    mViewer->setLight(light);
}

void World::deinitialize()
{
    mViewer = nullptr;
}


void World::loadDungeonByExterior(int regnum, int extid)
{
    const MapRegion &region = mRegions.at(regnum);
    const ExteriorLocation &extloc = region.mExteriors.at(extid);
    for(const DungeonInterior &dinfo : region.mDungeons)
    {
        if(extloc.mLocationId != dinfo.mExteriorLocationId)
            continue;

        Log::get().stream()<< "Entering "<<dinfo.mLocationName;
        for(const DungeonInterior::Block &block : dinfo.mBlocks)
        {
            std::stringstream sstr;
            sstr<< std::setfill('0')<<std::setw(8)<< block.mBlockIdx<<".RDB";
            std::string name = sstr.str();
            name.front() = gBlockIndexLabel[block.mBlockPreIndex];

            Log::get().stream()<< "  would load "<<name<<" @ "<<(int)block.mX<<"x"<<(int)block.mZ<<", start="<<block.mStartBlock;
        }
        break;
    }
}


} // namespace DF
