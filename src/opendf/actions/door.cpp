
#include "door.hpp"

#include <algorithm>

#include "class/placeable.hpp"
#include "class/activator.hpp"


namespace DF
{

Door Door::sDoors;


void Door::allocate(size_t idx, uint32_t flags, size_t link, const osg::Vec3f &orig)
{
    mClosed[idx] = std::make_pair(idx, orig);
    Activator::get().allocate(idx, flags, link, Door::activateFunc, Door::deallocateFunc);
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
        osg::Vec3f rot = iter->first.second;
        rot.y() += 512.0f * iter->second;
        Placeable::get().setRotate(iter->first.first, rot);

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
        osg::Vec3f rot = iter->first.second;
        rot.y() += 512.0f * (1.0f-iter->second);
        Placeable::get().setRotate(iter->first.first, rot);

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
