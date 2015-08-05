#ifndef CLASS_PLACEABLE_HPP
#define CLASS_PLACEABLE_HPP

#include <osg/Vec3>
#include <osg/Quat>

#include "misc/sparsearray.hpp"


namespace DF
{

// Should probably be somewhere else...
struct Position {
    osg::Quat mOrientation;
    osg::Vec3f mPoint;
};

class Placeable {
    static Placeable sPlaceables;

    Misc::SparseArray<Position> mPositions;

public:
    void deallocate(const size_t *ids, size_t count);
    void deallocate(size_t idx) { return deallocate(&idx, 1); }

    void setPos(size_t idx, const osg::Vec3f &pt, const osg::Quat &ori);
    void setRotate(size_t idx, const osg::Quat &ori);
    void setPoint(size_t idx, const osg::Vec3f &pt);

    void setPos(size_t idx, const osg::Vec3f &pt, const osg::Vec3f &rot)
    {
        setPos(idx, pt, osg::Quat(
             rot.x()*3.14159f/1024.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
            -rot.y()*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f),
             rot.z()*3.14159f/1024.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
        ));
    }
    void setRotate(size_t idx, const osg::Vec3f &rot)
    {
        setRotate(idx, osg::Quat(
             rot.x()*3.14159f/1024.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
            -rot.y()*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f),
             rot.z()*3.14159f/1024.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
        ));
    }

    const Position &getPos(size_t idx)
    {
        return mPositions[idx];
    };

    static Placeable &get() { return sPlaceables; }
};

} // namespace DF

#endif /* CLASS_PLACEABLE_HPP */
