#ifndef CLASS_ANIMATED_HPP
#define CLASS_ANIMATED_HPP

#include "misc/sparsearray.hpp"


namespace DF
{

struct AnimInfo {
    size_t mIdx;
    float mFrameTime;
    float mCurTime;
    uint32_t mNumFrames;
    uint32_t mCurFrame;
};

class Animated {
    static Animated sAnimated;

    Misc::SparseArray<AnimInfo> mAnimations;

public:
    void allocate(size_t idx, uint32_t numframes, float frametime);
    void deallocate(size_t idx);

    void update(float timediff);

    static Animated &get() { return sAnimated; }
};

} // namespace DF

#endif /* CLASS_ANIMATED_HPP */
