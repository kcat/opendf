#ifndef CLASS_DOOR_HPP
#define CLASS_DOOR_HPP

#include <osg/Vec3>

#include "misc/sparsearray.hpp"


namespace DF
{

typedef std::pair<size_t,osg::Vec3f> DoorData;
typedef std::pair<DoorData,float> DoorActiveData;

/* This duplicates a fair amount of the Mover class's rotate activator type,
 * but special handling is currently needed for the activate and deactivate
 * steps (physics and sound handling is different).
 */
class Door {
    static Door sDoors;

    Misc::SparseArray<DoorData> mClosed;
    Misc::SparseArray<DoorActiveData> mOpening;
    Misc::SparseArray<DoorData> mOpened;
    Misc::SparseArray<DoorActiveData> mClosing;

public:
    void allocate(size_t idx, const osg::Vec3f &orig);
    void deallocate(size_t idx);

    void activate(size_t idx);

    void update(float timediff);

    static void activateFunc(size_t idx) { sDoors.activate(idx); }
    static void deallocateFunc(size_t idx) { sDoors.deallocate(idx); }
    static Door &get() { return sDoors; }
};

} // namespace DF

#endif /* CLASS_DOOR_HPP */
