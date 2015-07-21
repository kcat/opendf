#ifndef WORLD_IFACE_HPP
#define WORLD_IFACE_HPP

#include <string>


namespace osgViewer
{
    class Viewer;
}

namespace DF
{

class WorldIface {
    static WorldIface &sInstance;

public:
    enum {
        Mask_UI     = 1<<0,
        Mask_Static = 1<<1,
        Mask_Light  = 1<<2,
        Mask_Flat   = 1<<3,
    };

    virtual void initialize(osgViewer::Viewer *viewer) = 0;
    virtual void deinitialize() = 0;

    virtual bool getExteriorByName(const std::string &name, size_t &regnum, size_t &mapnum) const = 0;
    virtual void loadExterior(int regnum, int extid) = 0;

    virtual void loadDungeonByExterior(int regnum, int extid) = 0;

    virtual void move(/*int objid,*/ float xrel, float yrel, float zrel) = 0;
    virtual void rotate(/*int objid,*/ float xrel, float yrel) = 0;

    virtual void update(float timediff) = 0;

    virtual void dumpArea() const = 0;
    virtual void dumpBlocks() const = 0;

    static WorldIface &get() { return sInstance; }
};

} // namespace DF

#endif /* WORLD_IFACE_HPP */
