
#include "exitdoor.hpp"

#include "world/iface.hpp"
#include "class/activator.hpp"


namespace DF
{

ExitDoor ExitDoor::sExitDoors;


void ExitDoor::allocate(size_t idx, uint32_t flags, size_t link, size_t regnum, size_t locnum)
{
    mExits[idx] = std::make_pair(regnum, locnum);
    Activator::get().allocate(idx, flags, link, ExitDoor::activateFunc, ExitDoor::deallocateFunc);
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
        while(iter != mActivatedExit.end())
            Activator::get().deactivate(mActivatedExit.getKey(iter++));
        mActivatedExit.clear();
        WorldIface::get().loadExterior(pos.first, pos.second);
    }
}

} // namespace DF
