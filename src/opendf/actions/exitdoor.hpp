#ifndef ACTIONS_EXITDOOR_HPP
#define ACTIONS_EXITDOOR_HPP

#include "misc/sparsearray.hpp"


namespace DF
{

class ExitDoor {
    static ExitDoor sExitDoors;

    Misc::SparseArray<std::pair<size_t,size_t>> mExits;
    Misc::SparseArray<std::pair<size_t,size_t>> mActivatedExit;

public:
    void allocate(size_t idx, size_t regnum, size_t locnum);
    void deallocate(size_t idx);

    void activate(size_t idx);

    void update();

    static void activateFunc(size_t idx) { sExitDoors.activate(idx); }
    static void deallocateFunc(size_t idx) { sExitDoors.deallocate(idx); }
    static ExitDoor &get() { return sExitDoors; }
};

} // namespace DF

#endif /* ACTIONS_EXITDOOR_HPP */
