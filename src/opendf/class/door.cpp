
#include "door.hpp"

#include <algorithm>

#include "placeable.hpp"
#include "activator.hpp"


namespace DF
{

Door Door::sDoors;


void Door::allocate(size_t idx, float yrot)
{
    mClosed[idx] = std::make_pair(idx, yrot);
}

void Door::deallocate(size_t idx)
{
    mClosed.erase(idx);
    mOpening.erase(idx);
    mOpened.erase(idx);
    mClosing.erase(idx);
}


void Door::activate(size_t idx)
{
    auto iter = mClosed.find(idx);
    if(iter != mClosed.end())
    {
        mOpening[idx] = std::make_pair(*iter, 0.0f);
        mClosed.erase(iter);
        return;
    }
    iter = mOpened.find(idx);
    if(iter != mOpened.end())
    {
        mClosing[idx] = std::make_pair(*iter, 0.0f);
        mOpened.erase(iter);
        return;
    }
}


void Door::update(float timediff)
{
    auto iter = mOpening.begin();
    while(iter != mOpening.end())
    {
        iter->second = std::min(iter->second+(timediff/1.5f), 1.0f);
        float yrot = iter->first.second + (512.0f*iter->second);
        Placeable::get().setLocalRot(iter->first.first, osg::Vec3f(0.0f, yrot, 0.0f));
        if(iter->second < 1.0f)
            ++iter;
        else
        {
            mOpened[iter->first.first] = iter->first;
            Activator::get().deactivate(iter->first.first);
            iter = mOpening.erase(iter);
        }
    }
    iter = mClosing.begin();
    while(iter != mClosing.end())
    {
        iter->second = std::min(iter->second+(timediff/1.5f), 1.0f);
        float yrot = iter->first.second + (512.0f*(1.0f-iter->second));
        Placeable::get().setLocalRot(iter->first.first, osg::Vec3f(0.0f, yrot, 0.0f));
        if(iter->second < 1.0f)
            ++iter;
        else
        {
            mClosed[iter->first.first] = iter->first;
            Activator::get().deactivate(iter->first.first);
            iter = mClosing.erase(iter);
        }
    }
}


} // namespace DF
