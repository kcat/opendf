#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include "iface.hpp"

#include <memory>
#include <vector>
#include <string>

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Quat>

#include "itembase.hpp"
#include "pitems.hpp"
#include "ditems.hpp"


namespace DF
{

struct MBlockHeader;
struct DBlockHeader;

class ObjectRef : public osg::Referenced {
    size_t mId;

public:
    ObjectRef(size_t id) : mId(id) { }

    size_t getId() const { return mId; }
};

struct MapTable {
    uint32_t mMapId;
    uint8_t mUnknown1;
    union {
        uint32_t mLongitudeType;
        struct {
            uint32_t mLongitude : 17;
            uint32_t mType : 15;
        };
    };
    uint16_t mLatitude;
    uint16_t mUnknown2;
    uint32_t mUnknown3;
};

struct MapRegion {
    std::vector<std::string> mNames;
    std::vector<MapTable> mTable;
    std::vector<ExteriorLocation> mExteriors;
    std::vector<DungeonInterior> mDungeons;
};

typedef std::vector<std::pair<uint16_t,uint8_t>> PakArray;

class World : public WorldIface {
    osg::ref_ptr<osgViewer::Viewer> mViewer;

    std::vector<MapRegion> mRegions;
    std::vector<PakArray> mClimates;
    std::vector<PakArray> mPolitics;

    const MapRegion *mCurrentRegion;
    const ExteriorLocation *mCurrentExterior;
    const DungeonInterior *mCurrentDungeon;

    size_t mCurrentSelection;

    std::vector<std::unique_ptr<MBlockHeader>> mExterior;
    std::vector<std::unique_ptr<DBlockHeader>> mDungeon;

    osg::Vec3f mCameraPos;
    osg::Quat mCameraRot;

    bool mFirstStart;

    World();
    ~World();

    static void loadPakList(std::string&& fname, std::vector<PakArray> &paklist);

    static uint8_t getPakListValue(const std::vector<PakArray> &paklist, size_t x, size_t y);

    uint8_t getClimateValue(size_t x, size_t y) const { return getPakListValue(mClimates, x, y); }
    uint8_t getPoliticValue(size_t x, size_t y) const { return getPakListValue(mPolitics, x, y); }

public:
    static World sWorld;

    virtual void initialize(osgViewer::Viewer *viewer) final;
    virtual void deinitialize() final;

    virtual bool getExteriorByName(const std::string &name, size_t &regnum, size_t &mapnum) const final;
    virtual void loadExterior(int regnum, int extid) final;

    virtual void loadDungeonByExterior(int regnum, int extid) final;

    virtual void move(/*int objid,*/ float xrel, float yrel, float zrel) final;
    virtual void rotate(/*int objid,*/ float xrel, float yrel) final;

    virtual void update(float timediff) final;

    virtual void activate() final;

    virtual void dumpArea() const final;
    virtual void dumpBlocks() const final;

    size_t castCameraToViewportRay(const float vpX, const float vpY, float maxDistance, bool ignoreFlats);
};

} // namespace DF

#endif /* WORLD_WORLD_HPP */
