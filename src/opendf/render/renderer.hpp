#ifndef RENDER_RENDERER_HPP
#define RENDER_RENDERER_HPP

#include <queue>

#include <osg/ref_ptr>
#include <osg/MatrixTransform>

#include "misc/sparsearray.hpp"

#include "class/placeable.hpp"


namespace DF
{

struct NodePosPair {
    osg::ref_ptr<osg::MatrixTransform> mNode;
    Position mPosition;

    bool operator<(const NodePosPair &rhs) const
    { return mNode < rhs.mNode; }
};

class Renderer {
    static Renderer sRenderer;

    osg::ref_ptr<osg::Group> mObjectRoot;
    Misc::SparseArray<osg::ref_ptr<osg::MatrixTransform>> mBaseNodes;
    std::priority_queue<NodePosPair> mDirtyNodes;
    Misc::SparseArray<osg::ref_ptr<osg::Uniform>> mAnimUniform;

public:
    enum {
        Mask_RTT    = 1<<0,
        Mask_UI     = 1<<1,
        Mask_Static = 1<<2,
        Mask_Light  = 1<<3,
        Mask_Flat   = 1<<4,
    };

    void setObjectRoot(osg::Group *root) { mObjectRoot = root; }
    osg::Group *getObjectRoot() const { return mObjectRoot; }

    void setNode(size_t idx, osg::MatrixTransform *node);
    void setAnimated(size_t idx, uint32_t startframe);

    void remove(const size_t *ids, size_t count);

    void markDirty(size_t idx, const Position &pos);
    void setFrameNum(size_t idx, uint32_t frame);

    void update();

    static Renderer &get() { return sRenderer; }
};

} // namespace DF

#endif /* RENDER_RENDERER_HPP */
