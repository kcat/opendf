#ifndef CLASS_PLACEABLE_HPP
#define CLASS_PLACEABLE_HPP

#include <osg/Vec3>

#include "misc/sparsearray.hpp"


namespace DF
{

// Should probably be somewhere else...
struct Position {
    osg::Vec3f mRotation;
    osg::Vec3f mLocalRotation;
    osg::Vec3f mPoint;
};

class Placeable {
    static Placeable sPlaceables;

    Misc::SparseArray<Position> mPositions;

public:
    void deallocate(const size_t *ids, size_t count);
    void deallocate(size_t idx) { return deallocate(&idx, 1); }

    void setPos(size_t idx, const osg::Vec3f &pt, const osg::Vec3f &rot);
    void setRotate(size_t idx, const osg::Vec3f &rot);
    void setLocalRot(size_t idx, const osg::Vec3f &rot);
    void setPoint(size_t idx, const osg::Vec3f &pt);

    const Position &getPos(size_t idx)
    {
        return mPositions[idx];
    };

    static Placeable &get() { return sPlaceables; }
};

} // namespace DF

#endif /* CLASS_PLACEABLE_HPP */
