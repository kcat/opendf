
#include "renderer.hpp"

#include <osg/MatrixTransform>

#include "class/placeable.hpp"


namespace DF
{

Renderer Renderer::sRenderer;


void Renderer::setNode(size_t idx, osg::MatrixTransform *node)
{
    mBaseNodes[idx] = node;
}

void Renderer::remove(const size_t *ids, size_t count)
{
    for(size_t i = 0;i < count;++i)
        mBaseNodes.erase(ids[i]);
}


void Renderer::markDirty(size_t idx, const Position &pos)
{
    if(mBaseNodes.exists(idx))
        mDirtyNodes.push({mBaseNodes[idx], pos});
}

void Renderer::update()
{
    while(!mDirtyNodes.empty())
    {
        const NodePosPair &nodepos = mDirtyNodes.top();

        const Position &pos = nodepos.mPosition;
        osg::Matrix mat;
        mat.makeRotate(
             pos.mRotation.x()*3.14159f/1024.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
            -pos.mRotation.y()*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f),
             pos.mRotation.z()*3.14159f/1024.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
        );
        mat.postMultTranslate(osg::Vec3(pos.mPoint.x(), pos.mPoint.y(), pos.mPoint.z()));
        mat.postMultRotate(osg::Quat(
             pos.mLocalRotation.x()*3.14159f/1024.0f, osg::Vec3f(1.0f, 0.0f, 0.0f),
            -pos.mLocalRotation.y()*3.14159f/1024.0f, osg::Vec3f(0.0f, 1.0f, 0.0f),
             pos.mLocalRotation.z()*3.14159f/1024.0f, osg::Vec3f(0.0f, 0.0f, 1.0f)
        ));

        osg::MatrixTransform *node = nodepos.mNode;
        //node->setDataVariance(osg::Node::DYNAMIC);
        node->setMatrix(mat);

        mDirtyNodes.pop();
    }
}


} // namespace DF
