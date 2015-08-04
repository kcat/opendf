
#include "exitdoor.hpp"

#include "world/iface.hpp"


namespace DF
{

ExitDoor ExitDoor::sExitDoors;


void ExitDoor::allocate(size_t idx, size_t regnum, size_t locnum)
{
    mExits[idx] = std::make_pair(regnum, locnum);
}

void ExitDoor::deallocate(size_t idx)
{
    mExits.erase(idx);
    mActivatedExit.erase(idx);
}


void ExitDoor::activate(size_t idx)
{
    auto iter = mExits.find(idx);
    if(iter != mExits.end())
        mActivatedExit[idx] = *iter;
}


void ExitDoor::update()
{
    auto iter = mActivatedExit.begin();
    if(iter != mActivatedExit.end())
    {
        auto pos = *iter;
        mActivatedExit.clear();
        WorldIface::get().loadExterior(pos.first, pos.second);
    }
}

} // namespace DF
