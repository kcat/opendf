#ifndef CLASS_DOOR_HPP
#define CLASS_DOOR_HPP

#include "misc/sparsearray.hpp"


namespace DF
{

typedef std::pair<size_t,float> DoorData;
typedef std::pair<DoorData,float> DoorActiveData;

class Door {
    static Door sDoors;

    Misc::SparseArray<DoorData> mClosed;
    Misc::SparseArray<DoorActiveData> mOpening;
    Misc::SparseArray<DoorData> mOpened;
    Misc::SparseArray<DoorActiveData> mClosing;

public:
    void allocate(size_t idx, float yrot);
    void deallocate(size_t idx);

    void activate(size_t idx);

    void update(float timediff);

    static Door &get() { return sDoors; }
};

} // namespace DF

#endif /* CLASS_DOOR_HPP */
