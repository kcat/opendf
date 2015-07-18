#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include "iface.hpp"

#include <vector>
#include <string>

#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Quat>

#include "itembase.hpp"
#include "pitems.hpp"
#include "ditems.hpp"


namespace DF
{

struct DBlockHeader;

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

class World : public WorldIface {
    osg::ref_ptr<osgViewer::Viewer> mViewer;

    std::vector<MapRegion> mRegions;

    const MapRegion *mCurrentRegion;
    const ExteriorLocation *mCurrentExterior;
    const DungeonInterior *mCurrentDungeon;
    std::vector<DBlockHeader> mDungeon;

    osg::Vec3f mCameraPos;
    osg::Quat mCameraRot;

    bool mFirstStart;

    World();
    ~World();

public:
    static World sWorld;

    virtual void initialize(osgViewer::Viewer *viewer) final;
    virtual void deinitialize() final;

    virtual void loadDungeonByExterior(int regnum, int extid) final;

    virtual void move(/*int objid,*/ float xrel, float yrel, float zrel) final;
    virtual void rotate(/*int objid,*/ float xrel, float yrel) final;

    virtual void update(float timediff) final;

    virtual void dumpArea() const final;
    virtual void dumpBlocks() const final;

    size_t castCameraToViewportRay(const float vpX, const float vpY, float maxDistance, bool ignoreFlats);
};

} // namespace DF

#endif /* WORLD_WORLD_HPP */
