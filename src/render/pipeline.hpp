#ifndef RENDER_PIPELINE_HPP
#define RENDER_PIPELINE_HPP

#include <string>

#include <osg/ref_ptr>
#include <osg/Camera>

#include "cvars.hpp"


namespace osg
{
    class Geode;

    class Texture;
    class StateSet;

    class Vec2f;
}

namespace DF
{

EXTERN_CVAR(CVarInt, r_fov);


class RenderPipeline {
    static RenderPipeline sPipeline;

    int mScreenWidth;
    int mScreenHeight;

    int mTextureWidth;
    int mTextureHeight;

    osg::ref_ptr<osg::Group> mGraph;
    osg::ref_ptr<osg::Camera> mClearPass;
    osg::ref_ptr<osg::Camera> mMainPass;
    osg::ref_ptr<osg::Camera> mLightPass;
    osg::ref_ptr<osg::Camera> mCombinerPass;
    osg::ref_ptr<osg::Camera> mOutputPass;

    osg::ref_ptr<osg::Texture> mGBufferColors;
    osg::ref_ptr<osg::Texture> mGBufferNormals;
    osg::ref_ptr<osg::Texture> mGBufferPositions;
    osg::ref_ptr<osg::Texture> mDepthStencil;

    osg::ref_ptr<osg::Texture> mDiffuseLight;
    osg::ref_ptr<osg::Texture> mSpecularLight;

    osg::ref_ptr<osg::Texture> mFinalBuffer;

    osg::ref_ptr<osg::Camera> mDebugMapDisplay;

    static osg::ref_ptr<osg::Geometry> createScreenGeometry(const osg::Vec2f &corner, float width, float height, int tex_width, int tex_height);
    static osg::Geode *createScreenQuad(const osg::Vec2f &corner, float width, float height, int tex_width, int tex_height);

    static osg::ref_ptr<osg::Texture> createTextureRect(int width, int height, GLenum internalFormat, GLenum format, GLenum type);
    static osg::ref_ptr<osg::Camera> createRTTCamera(osg::Camera::BufferComponent buffer, osg::Texture *tex);

    static osg::StateSet *setShaderProgram(osg::Node *node, std::string vert, std::string frag);

public:
    RenderPipeline();
    ~RenderPipeline();

    void initialize(osg::Group *scene, int width, int height);
    void deinitialize();

    double getAspectRatio() const
    {
        return double(mScreenWidth) / double(mScreenHeight);
    }

    void setProjectionMatrix(const osg::Matrix &matrix) { mMainPass->setProjectionMatrix(matrix); }
    const osg::Matrix &getProjectionMatrix() const { return mMainPass->getProjectionMatrix(); }

    osg::Node *createDirectionalLight();
    void removeDirectionalLight(osg::Node *node);

    void toggleDebugMapDisplay();

    osg::StateSet *getLightingStateSet() { return mLightPass->getStateSet(); }

    osg::Group *getGraphRoot() const { return mGraph.get(); }

    static RenderPipeline &get() { return sPipeline; }
};

} // namespace DF

#endif /* RENDER_PIPELINE_HPP */
