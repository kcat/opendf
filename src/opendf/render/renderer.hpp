#ifndef RENDER_RENDERER_HPP
#define RENDER_RENDERER_HPP

#include <queue>

#include <osg/ref_ptr>

#include "misc/sparsearray.hpp"

#include "class/placeable.hpp"


namespace osg
{
    class MatrixTransform;
}

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

    Misc::SparseArray<osg::ref_ptr<osg::MatrixTransform>> mBaseNodes;
    std::priority_queue<NodePosPair> mDirtyNodes;

public:
    enum {
        Mask_RTT    = 1<<0,
        Mask_UI     = 1<<1,
        Mask_Static = 1<<2,
        Mask_Light  = 1<<3,
        Mask_Flat   = 1<<4,
    };

    void setNode(size_t idx, osg::MatrixTransform *node);

    void remove(const size_t *ids, size_t count);

    void markDirty(size_t idx, const Position &pos);

    void update();

    static Renderer &get() { return sRenderer; }
};

} // namespace DF

#endif /* RENDER_RENDERER_HPP */
