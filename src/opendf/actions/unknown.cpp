
#include "unknown.hpp"

#include <iomanip>

#include "class/activator.hpp"
#include "log.hpp"


namespace DF
{

UnknownAction UnknownAction::sActions;


void UnknownAction::allocate(size_t idx, uint8_t type, const std::array<uint8_t,5> &data)
{
    mUnknowns[idx] = TypeDataPair{type, data};
}

void UnknownAction::deallocate(size_t idx)
{
    mUnknowns.erase(idx);
    mActive.erase(idx);
}


void UnknownAction::activate(size_t idx)
{
    auto iter = mUnknowns.find(idx);
    if(iter != mUnknowns.end())
        mActive[idx] = *iter;
}


void UnknownAction::update()
{
    auto iter = mActive.begin();
    if(iter != mActive.end())
    {
        do {
            Log::get().stream(Log::Level_Error)<< "Unhandled action:"<<std::setfill('0')
                <<" Type 0x"<<std::hex<<std::setw(2)<<iter->first<<", data "
                <<" 0x"<<std::hex<<std::setw(2)<<iter->second[0]
                <<" 0x"<<std::hex<<std::setw(2)<<iter->second[1]
                <<" 0x"<<std::hex<<std::setw(2)<<iter->second[2]
                <<" 0x"<<std::hex<<std::setw(2)<<iter->second[3]
                <<" 0x"<<std::hex<<std::setw(2)<<iter->second[4];
            Activator::get().deactivate(mActive.getKey(iter++));
        } while(iter != mActive.end());
        mActive.clear();
    }
}

} // namespace DF
