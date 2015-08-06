
#include "activator.hpp"


namespace DF
{

Activator Activator::sActivators;


void Activator::allocate(size_t idx, uint32_t flags, size_t link, ActivatorCallback callback, ActivatorCallback dealloc)
{
    mDeallocators[idx] = dealloc;
    mFlags[idx] = flags;
    mInactive[idx] = ActivatorData{ callback, link };
}

void Activator::deallocate(size_t idx)
{
    auto iter = mDeallocators.find(idx);
    if(iter != mDeallocators.end())
    {
        (*iter)(idx);
        mDeallocators.erase(iter);
    }
    mFlags.erase(idx);
    mInactive.erase(idx);
    mActive.erase(idx);
}


void Activator::activate(size_t idx)
{
    // Make sure the object is inactive, and is activatable
    auto iter = mInactive.find(idx);
    if(iter == mInactive.end() || !(mFlags[idx]&ActionFlag_Activatable))
        return;

    // Make sure all the object links are currently inactive too
    do {
        if(iter->mLink == ~static_cast<size_t>(0))
            break;
        iter = mInactive.find(iter->mLink);
    } while(iter != mInactive.end());
    if(iter == mInactive.end())
        return;

    // Now activate them all
    do {
        iter = mInactive.find(idx);
        iter->mCallback(idx);

        size_t next = iter->mLink;
        mActive[idx] = *iter;
        mInactive.erase(iter);
        idx = next;
    } while(idx != ~static_cast<size_t>(0));
}

void Activator::deactivate(size_t idx)
{
    auto iter = mActive.find(idx);
    if(iter != mActive.end())
    {
        mInactive[idx] = *iter;
        mActive.erase(iter);
    }
}

} // namespace DF
