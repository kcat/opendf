
#include "world.hpp"

#include <osgViewer/Viewer>
#include <osg/Light>


namespace DF
{

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

} // namespace DF
