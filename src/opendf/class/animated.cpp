
#include "animated.hpp"

#include <cmath>

#include "render/renderer.hpp"


namespace DF
{

Animated Animated::sAnimated;


void Animated::allocate(size_t idx, uint32_t numframes, float frametime)
{
    mAnimations[idx] = AnimInfo{ idx, frametime, 0.0f, numframes, 0 };
    Renderer::get().setAnimated(idx, 0);
}

void Animated::deallocate(size_t idx)
{
    mAnimations.erase(idx);
}


void Animated::update(float timediff)
{
    auto iter = mAnimations.begin();
    while(iter != mAnimations.end())
    {
        iter->mCurTime += timediff;
        iter->mCurFrame += (uint32_t)floorf(iter->mCurTime / iter->mFrameTime);
        iter->mCurFrame %= iter->mNumFrames;
        iter->mCurTime = fmodf(iter->mCurTime, iter->mFrameTime);
        Renderer::get().setFrameNum(iter->mIdx, iter->mCurFrame);
        ++iter;
    }
}

} // namespace DF
