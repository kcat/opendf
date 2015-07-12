#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include "iface.hpp"

#include <osg/ref_ptr>


namespace DF
{

class World : public WorldIface {
    osg::ref_ptr<osgViewer::Viewer> mViewer;

    World();
    ~World();

public:
    static World sWorld;

    virtual void initialize(osgViewer::Viewer *viewer) final;
    virtual void deinitialize() final;
};

} // namespace DF

#endif /* WORLD_WORLD_HPP */
