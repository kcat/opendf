#ifndef WORLD_IFACE_HPP
#define WORLD_IFACE_HPP


namespace osgViewer
{
    class Viewer;
}

namespace DF
{

class WorldIface {
    static WorldIface &sInstance;

public:
    virtual void initialize(osgViewer::Viewer *viewer) = 0;
    virtual void deinitialize() = 0;

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
