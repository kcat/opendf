
#include "placeable.hpp"

#include "render/renderer.hpp"


namespace DF
{

Placeable Placeable::sPlaceables;


void Placeable::deallocate(const size_t *ids, size_t count)
{
    for(size_t i = 0;i < count;++i)
        mPositions.erase(ids[i]);
}


void Placeable::setPos(size_t idx, const osg::Vec3f &pt, const osg::Vec3f &rot)
{
    Position &pos = mPositions[idx];
    pos.mRotation = rot;
    pos.mPoint = pt;
    Renderer::get().markDirty(idx, pos);
}

void Placeable::setRotate(size_t idx, const osg::Vec3f &rot)
{
    Position &pos = mPositions[idx];
    pos.mRotation = rot;
    Renderer::get().markDirty(idx, pos);
}

void Placeable::setPoint(size_t idx, const osg::Vec3f &pt)
{
    Position &pos = mPositions[idx];
    pos.mPoint = pt;
    Renderer::get().markDirty(idx, pos);
}

void Placeable::setLocalRot(size_t idx, const osg::Vec3f &rot)
{
    Position &pos = mPositions[idx];
    pos.mLocalRotation = rot;
    Renderer::get().markDirty(idx, pos);
}

} // namespace DF
