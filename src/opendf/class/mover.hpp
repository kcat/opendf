#ifndef CLASS_MOVER_HPP
#define CLASS_MOVER_HPP

#include "misc/sparsearray.hpp"
#include <osg/Vec3>


namespace DF
{

struct MoverData {
    size_t mId;
    osg::Vec3f mOrig;
    osg::Vec3f mAmount;
    float mDuration;
};

class Mover {
    static Mover sMovers;

    Misc::SparseArray<size_t> mSoundIds;

    Misc::SparseArray<MoverData> mTranslateStart;
    Misc::SparseArray<std::pair<MoverData,float>> mActiveTrans;
    Misc::SparseArray<MoverData> mTranslateEnd;
    Misc::SparseArray<std::pair<MoverData,float>> mActiveTransRev;

    Misc::SparseArray<MoverData> mRotateStart;
    Misc::SparseArray<std::pair<MoverData,float>> mActiveRot;
    Misc::SparseArray<MoverData> mRotateEnd;
    Misc::SparseArray<std::pair<MoverData,float>> mActiveRotRev;

public:
    void allocateTranslate(size_t idx, size_t soundid, const osg::Vec3f &orig, const osg::Vec3f &amount, float duration);
    void allocateRotate(size_t idx, size_t soundid, const osg::Vec3f &orig, const osg::Vec3f &amount, float duration);
    void deallocateTranslate(size_t idx);
    void deallocateRotate(size_t idx);

    void activateTranslate(size_t idx);
    void activateRotate(size_t idx);

    void update(float timediff);

    static void activateTranslateFunc(size_t idx) { sMovers.activateTranslate(idx); }
    static void activateRotateFunc(size_t idx) { sMovers.activateRotate(idx); }
    static void deallocateTranslateFunc(size_t idx) { sMovers.deallocateTranslate(idx); }
    static void deallocateRotateFunc(size_t idx) { sMovers.deallocateRotate(idx); }
    static Mover &get() { return sMovers; }
};

} // namespace DF

#endif /* CLASS_MOVER_HPP */
