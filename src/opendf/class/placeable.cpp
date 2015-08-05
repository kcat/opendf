
#include "placeable.hpp"

#include "render/renderer.hpp"


namespace DF
{

Placeable Placeable::sPlaceables;


void Placeable::deallocate(const size_t *ids, size_t count)
{
    while(count > 0)
        mPositions.erase(ids[--count]);
}


void Placeable::setPos(size_t idx, const osg::Vec3f &pt, const osg::Quat &ori)
{
    Position &pos = mPositions[idx];
    pos.mOrientation = ori;
    pos.mPoint = pt;
    Renderer::get().markDirty(idx, pos);
}

void Placeable::setRotate(size_t idx, const osg::Quat &ori)
{
    Position &pos = mPositions[idx];
    pos.mOrientation = ori;
    Renderer::get().markDirty(idx, pos);
}

void Placeable::setPoint(size_t idx, const osg::Vec3f &pt)
{
    Position &pos = mPositions[idx];
    pos.mPoint = pt;
    Renderer::get().markDirty(idx, pos);
}

} // namespace DF
