
#include "world.hpp"

#include <sstream>
#include <iomanip>
#include <array>

#include <osgViewer/Viewer>
#include <osg/Light>

#include "components/vfs/manager.hpp"

#include "gui/iface.hpp"
#include "dblocks.hpp"
#include "cvars.hpp"
#include "log.hpp"


namespace
{

static const std::array<char,6> gBlockIndexLabel{{ 'N', 'W', 'L', 'S', 'B', 'M' }};

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

LogStream& operator<<(LogStream &stream, const LocationHeader &loc)
{
    stream<<std::setfill('0');
    stream<< "  Door count: "<<loc.mDoorCount<<"\n";
    for(const LocationDoor &door : loc.mDoors)
    {
        stream<< "  Door "<<std::distance(loc.mDoors.data(), &door)<<":\n";
        stream<< "    BuildingDataIndex: "<<door.mBuildingDataIndex<<"\n";
        stream<< "    Null: "<<(int)door.mNullValue<<"\n";
        stream<< "    UnknownMask: 0x"<<std::hex<<std::setw(2)<<(int)door.mUnknownMask<<std::setw(0)<<std::dec<<"\n";
        stream<< "    Unknown: 0x"<<std::hex<<std::setw(2)<<(int)door.mUnknown1<<std::setw(0)<<std::dec<<"\n";
        stream<< "    Unknown: 0x"<<std::hex<<std::setw(2)<<(int)door.mUnknown2<<std::setw(0)<<std::dec<<"\n";
    }

    stream<< "  Always one: "<<loc.mAlwaysOne1<<"\n";
    stream<< "  Null: "<<loc.mNullValue1<<"\n";
    stream<< "  Null: "<<(int)loc.mNullValue2<<"\n";
    stream<< "  Y: "<<loc.mY<<"\n";
    stream<< "  Null: "<<loc.mNullValue3<<"\n";
    stream<< "  X: "<<loc.mX<<"\n";
    stream<< "  IsExterior: "<<loc.mIsExterior<<"\n";
    stream<< "  Null: "<<loc.mNullValue4<<"\n";
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<loc.mUnknown1<<std::setw(0)<<std::dec<<"\n";
    stream<< "  Unknown: 0x"<<std::hex<<std::setw(8)<<loc.mUnknown2<<std::setw(0)<<std::dec<<"\n";
    stream<< "  Always one: "<<loc.mAlwaysOne2<<"\n";
    stream<< "  LocationID: "<<loc.mLocationId<<"\n";
    stream<< "  Null: "<<loc.mNullValue5<<"\n";
    stream<< "  IsInterior: "<<loc.mIsInterior<<"\n";
    stream<< "  ExteriorLocationID: "<<loc.mExteriorLocationId<<"\n";
    //uint8_t mNullValue6[26];
    stream<< "  LocationName: \""<<loc.mLocationName<<"\"\n";
    stream<< "  Unknown:";//<<std::hex<<std::setw(2);
    for(uint32_t unk : loc.mUnknown3)
        stream<< " 0x"<<std::hex<<std::setw(2)<<unk;
    stream<<std::setw(0)<<std::dec<<std::setfill(' ');

    return stream;
}


CCMD(dumparea)
{
    WorldIface::get().dumpArea();
}

CCMD(dumpblocks)
{
    WorldIface::get().dumpBlocks();
}


CCMD(dwarp)
{
    if(params.empty())
    {
        Log::get().stream(Log::Level_Error)<< "Usage: dwarp <region index> <location index>";
        return;
    }

    try {
        char *next = nullptr;
        size_t regnum = strtoul(params.c_str(), &next, 10);
        if(!next || *next != ' ')
        {
            Log::get().stream(Log::Level_Error)<< "Invalid region parameter: "<<params;
            return;
        }
        size_t mapnum = strtoul(next, &next, 10);
        if(next && *next != '\0')
        {
            Log::get().stream(Log::Level_Error)<< "Invalid location parameter: "<<params;
            return;
        }

        WorldIface::get().loadDungeonByExterior(regnum, mapnum);
    }
    catch(std::exception &e) {
        Log::get().stream(Log::Level_Error)<< "Exception: "<<e.what();
        return;
    }
}


CVAR(CVarBool, g_introspect, false);


World World::sWorld;
WorldIface &WorldIface::sInstance = World::sWorld;

World::World()
  : mCurrentRegion(nullptr)
  , mCurrentExterior(nullptr)
  , mCurrentDungeon(nullptr)
  , mFirstStart(true)
{
    mCameraRot.makeRotate(
            0.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
        3.14159f, osg::Vec3f(0.0f, 1.0f, 0.0f),
            0.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
    );
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
    mDungeon.clear();
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

        mDungeon.clear();
        mCurrentRegion = &region;
        mCurrentExterior = &extloc;
        mCurrentDungeon = &dinfo;

        Log::get().stream()<< "Entering "<<dinfo.mLocationName;
        for(const DungeonBlock &block : dinfo.mBlocks)
        {
            std::stringstream sstr;
            sstr<< std::setfill('0')<<std::setw(8)<< block.mBlockIdx<<".RDB";
            std::string name = sstr.str();
            name.front() = gBlockIndexLabel.at(block.mBlockPreIndex);

            VFS::IStreamPtr stream = VFS::Manager::get().open(name.c_str());
            if(!stream) throw std::runtime_error("Failed to open "+name);

            mDungeon.push_back(DBlockHeader());
            mDungeon.back().load(*stream);
        }

        osg::Group *root = mViewer->getSceneData()->asGroup();
        for(size_t i = 0;i < mDungeon.size();++i)
        {
            mDungeon[i].buildNodes(root, i, dinfo.mBlocks[i].mX, dinfo.mBlocks[i].mZ);

            if(dinfo.mBlocks[i].mStartBlock)
            {
                FlatObject *flat = nullptr;
                if(mFirstStart)
                {
                    mFirstStart = false;
                    try {
                        flat = mDungeon[i].getFlatByTexture(Marker_EnterID);
                    }
                    catch(std::exception &e) {
                        Log::get().stream(Log::Level_Error)<< "Exception looking for Enter marker: "<<e.what();
                    }
                }
                if(!flat)
                    flat = mDungeon[i].getFlatByTexture(Marker_StartID);
                osg::Vec3f pos(flat->mXPos, flat->mYPos, flat->mZPos);
                mCameraPos = osg::componentMultiply(
                    -pos + osg::Vec3f(dinfo.mBlocks[i].mX*2048.0f, 0.0f, dinfo.mBlocks[i].mZ*2048.0f),
                    osg::Vec3f(1.0f, -1.0f, -1.0f)
                );
            }
        }
        break;
    }
}


void World::move(float xrel, float yrel, float zrel)
{
    mCameraPos += mCameraRot*osg::Vec3f(xrel, yrel, zrel);
}

void World::rotate(float xrel, float yrel)
{
    /* HACK: rotate the camera around */
    static float x=0.0f, y=180.0f;

    x = std::min(std::max(x+xrel, -89.0f), 89.0f);
    y += yrel;

    mCameraRot.makeRotate(
        x*3.14159f/180.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
       -y*3.14159f/180.0f, osg::Vec3f(0.0f, 1.0f, 0.0f),
                     0.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
    );
}


void World::update(float timediff)
{
    osg::Matrixf matf(mCameraRot.inverse());
    matf.preMultTranslate(mCameraPos);
    mViewer->getCamera()->setViewMatrix(matf);

    size_t result;
    if(GuiIface::get().getMode() == GuiIface::Mode_Game)
        result = castCameraToViewportRay(0.5f, 0.5f, 1024.0f, false);
    else
    {
        float x, y;
        GuiIface::get().getMousePosition(x, y);
        result = castCameraToViewportRay(x, y, 1024.0f, false);
    }
    if(result == ~static_cast<size_t>(0) || !*g_introspect)
        GuiIface::get().updateStatus(std::string());
    else
    {
        std::stringstream sstr;
        DBlockHeader &block = mDungeon.at(result>>24);
        const ObjectBase *obj = block.getObject(result&0x00ffffff);
        if(!obj)
            sstr<< "Failed to lookup object 0x"<<std::hex<<std::setfill('0')<<std::setw(8)<<result;
        else
        {
            sstr<<std::setfill('0');
            obj->print(sstr);
        }
        GuiIface::get().updateStatus(sstr.str());
    }
}


void World::dumpArea() const
{
    LogStream stream(Log::get().stream());
    if(mCurrentDungeon)
    {
        int regnum = std::distance(mRegions.data(), mCurrentRegion);
        stream<< "Current region index: "<<regnum<<"\n";
        int mapnum = std::distance(mCurrentRegion->mExteriors.data(), mCurrentExterior);
        stream<< "Current exterior index: "<<mapnum<<"\n";
        stream<< "Current Dungeon:\n";
        stream<< *mCurrentDungeon;
    }
    else if(mCurrentExterior)
    {
        int regnum = std::distance(mRegions.data(), mCurrentRegion);
        stream<< "Current region index: "<<regnum<<"\n";
        stream<< "Current Exterior:\n";
        stream<< static_cast<const LocationHeader&>(*mCurrentExterior);
    }
    else if(mCurrentRegion)
    {
        int regnum = std::distance(mRegions.data(), mCurrentRegion);
        stream<< "Current region index "<<regnum;
    }
    else
        stream<< "Not in a region";
}

void World::dumpBlocks() const
{
    LogStream stream(Log::get().stream());
    int i = 0;
    for(const DBlockHeader &block : mDungeon)
    {
        stream<< "Block "<<i<<":\n";
        block.print(stream);
        ++i;
    }
}


size_t World::castCameraToViewportRay(const float vpX, const float vpY, float maxDistance, bool ignoreFlats)
{
    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector(new osgUtil::LineSegmentIntersector(
        osgUtil::LineSegmentIntersector::PROJECTION, vpX*2.f - 1.f, vpY*-2.f + 1.f
    ));

    osg::Vec3d dist = osg::Vec3d(0.0f,0.0f,-maxDistance) *
                      mViewer->getCamera()->getProjectionMatrix();

    osg::Vec3d end = intersector->getEnd();
    end.z() = dist.z();
    intersector->setEnd(end);
    intersector->setIntersectionLimit(osgUtil::LineSegmentIntersector::LIMIT_NEAREST);

    osgUtil::IntersectionVisitor intersectionVisitor(intersector);
    int mask = intersectionVisitor.getTraversalMask();
    mask &= ~(Mask_UI | Mask_Light);
    if(ignoreFlats)
        mask &= ~(Mask_Flat);

    intersectionVisitor.setTraversalMask(mask);

    mViewer->getCamera()->accept(intersectionVisitor);

    size_t result = ~static_cast<size_t>(0);
    if(intersector->containsIntersections())
    {
        osgUtil::LineSegmentIntersector::Intersection intersection = intersector->getFirstIntersection();

        ObjectRef *ref = nullptr;
        for(auto it = intersection.nodePath.cbegin();it != intersection.nodePath.cend();++it)
        {
            osg::Referenced *userData = (*it)->getUserData();
            if(!userData) continue;

            ref = dynamic_cast<ObjectRef*>(userData);
            if(ref) break;
        }

        if(ref)
            result = ref->getId();
    }

    return result;
}

} // namespace DF
