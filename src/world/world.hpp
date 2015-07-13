#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include "iface.hpp"

#include <vector>
#include <string>

#include <osg/ref_ptr>

#include "itembase.hpp"
#include "ditems.hpp"


namespace DF
{

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

    std::vector<DungeonInterior> mDungeons;
};

class World : public WorldIface {
    osg::ref_ptr<osgViewer::Viewer> mViewer;

    std::vector<MapRegion> mRegions;

    World();
    ~World();

public:
    static World sWorld;

    virtual void initialize(osgViewer::Viewer *viewer) final;
    virtual void deinitialize() final;
};

} // namespace DF

#endif /* WORLD_WORLD_HPP */
