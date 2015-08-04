
#include "pipeline.hpp"

#include <SDL_opengl.h>

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/TextureRectangle>
#include <osg/PolygonMode>
#include <osg/Depth>
#include <osg/Stencil>
#include <osg/BlendFunc>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <osgDB/ReadFile>

#include "renderer.hpp"

#include "cvars.hpp"
#include "log.hpp"


namespace DF
{

CVAR(CVarInt, r_fov, 65, 40, 120);

CCMD(setfov)
{
    if(!params.empty() && !r_fov.set(params))
    {
        Log::get().stream(Log::Level_Error)<< "Failed to set FOV to \""<<params<<"\"";
        return;
    }
    RenderPipeline::get().setProjectionMatrix(osg::Matrix::perspective(
        *r_fov, RenderPipeline::get().getAspectRatio(), 1.0, 10000.0
    ));
}


CCMD(togglemaps, "tm")
{
    RenderPipeline::get().toggleDebugMapDisplay();
}


RenderPipeline RenderPipeline::sPipeline;


RenderPipeline::RenderPipeline()
  : mScreenWidth(0), mScreenHeight(0)
  , mTextureWidth(0), mTextureHeight(0)
{
}
RenderPipeline::~RenderPipeline()
{
}


osg::ref_ptr<osg::Geometry> RenderPipeline::createScreenGeometry(const osg::Vec2f &corner, float width, float height, int tex_width, int tex_height)
{
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);

    osg::ref_ptr<osg::Vec2Array> vertices = new osg::Vec2Array(4);
    (*vertices)[0] = corner;
    (*vertices)[1] = corner + osg::Vec2(width,   0.0f);
    (*vertices)[2] = corner + osg::Vec2(width, height);
    (*vertices)[3] = corner + osg::Vec2( 0.0f, height);
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(4);
    (*texcoords)[0] = osg::Vec2(     0.0f,       0.0f);
    (*texcoords)[1] = osg::Vec2(tex_width,       0.0f);
    (*texcoords)[2] = osg::Vec2(tex_width, tex_height);
    (*texcoords)[3] = osg::Vec2(     0.0f, tex_height);

    geom->setVertexArray(vertices.get());
    geom->setTexCoordArray(0, texcoords.get(), osg::Array::BIND_PER_VERTEX);
    geom->addPrimitiveSet(new osg::DrawArrays(
        osg::PrimitiveSet::QUADS, 0, vertices->size()
    ));

    return geom;
}

osg::Geode *RenderPipeline::createScreenQuad(const osg::Vec2f &corner, float width, float height, int tex_width, int tex_height)
{
    osg::ref_ptr<osg::Geometry> geom = createScreenGeometry(corner, width, height, tex_width, tex_height);

    osg::ref_ptr<osg::Geode> quad = new osg::Geode();
    osg::StateSet *ss = quad->getOrCreateStateSet();
    ss->setAttribute(
        new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL),
        osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED
    );
    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    quad->addDrawable(geom.get());

    return quad.release();
}

osg::ref_ptr<osg::Texture> RenderPipeline::createTextureRect(int width, int height, GLenum internalFormat, GLenum format, GLenum type)
{
    osg::ref_ptr<osg::TextureRectangle> tex = new osg::TextureRectangle();
    tex->setTextureSize(width, height);
    tex->setInternalFormat(internalFormat);
    tex->setSourceFormat(format);
    tex->setSourceType(type);
    return osg::ref_ptr<osg::Texture>(tex);
}

osg::ref_ptr<osg::Camera> RenderPipeline::createRTTCamera(osg::Camera::BufferComponent buffer, osg::Texture *tex)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera();
    camera->setClearColor(osg::Vec4());
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    if(tex)
    {
        tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        camera->setViewport(0, 0, tex->getTextureWidth(), tex->getTextureHeight());
        camera->attach(buffer, tex);
    }
    return camera;
}

osg::StateSet *RenderPipeline::setShaderProgram(osg::Node *node, std::string vert, std::string frag)
{
    osg::ref_ptr<osg::Program> program = new osg::Program();
    program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, vert));
    program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, frag));

    osg::StateSet *ss = node->getOrCreateStateSet();
    ss->setAttributeAndModes(program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE
    );
    return ss;
}


void RenderPipeline::initialize(osg::Group *scene, int width, int height)
{
    if(mGraph.valid())
        deinitialize();

    mScreenWidth = mTextureWidth = width;
    mScreenHeight = mTextureHeight = height;

    mGBufferColors    = createTextureRect(mTextureWidth, mTextureHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    mGBufferNormals   = createTextureRect(mTextureWidth, mTextureHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    mGBufferPositions = createTextureRect(mTextureWidth, mTextureHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    mDepthStencil     = createTextureRect(mTextureWidth, mTextureHeight, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV);
    mDiffuseLight  = createTextureRect(mTextureWidth, mTextureHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    mSpecularLight = createTextureRect(mTextureWidth, mTextureHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    mFinalBuffer = createTextureRect(mTextureWidth, mTextureHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    int pre_render_pass = 0;

    // Clear pass (clears specular and depth buffers)
    mClearPass = createRTTCamera(osg::Camera::COLOR_BUFFER, mSpecularLight.get());
    mClearPass->setNodeMask(Renderer::Mask_RTT);
    mClearPass->attach(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, mDepthStencil.get());
    mClearPass->setRenderOrder(osg::Camera::PRE_RENDER, pre_render_pass++);

    // Main pass (generates colors, normals, positions, and emissive diffuse lighting).
    mMainPass = createRTTCamera(osg::Camera::COLOR_BUFFER0, mGBufferColors.get());
    mMainPass->attach(osg::Camera::COLOR_BUFFER1, mGBufferNormals.get());
    mMainPass->attach(osg::Camera::COLOR_BUFFER2, mGBufferPositions.get());
    mMainPass->attach(osg::Camera::COLOR_BUFFER3, mDiffuseLight.get());
    mMainPass->attach(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, mDepthStencil.get());
    // FIXME: Once sky rendering is implemented, don't clear buffers here
    //mMainPass->setClearMask(GL_NONE);
    mMainPass->setRenderOrder(osg::Camera::PRE_RENDER, pre_render_pass++);
    osg::StateSet *ss = mMainPass->getOrCreateStateSet();
    ss->addUniform(new osg::Uniform("illumination_color", osg::Vec4()));
    {
        // Make sure to clear stencil bit 0x1 by default (geometry that doesn't
        // want external lighting should set bit 0x1 on z-pass).
        osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil();
        stencil->setWriteMask(~0);
        stencil->setFunction(osg::Stencil::ALWAYS, 0x00, 0x01);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);
        ss->setAttributeAndModes(stencil.get());
    }
    mMainPass->addChild(scene);

    // Lighting pass (generates diffuse and specular).
    mLightPass = createRTTCamera(osg::Camera::COLOR_BUFFER0, mDiffuseLight.get());
    mLightPass->setNodeMask(Renderer::Mask_RTT);
    mLightPass->attach(osg::Camera::COLOR_BUFFER1, mSpecularLight.get());
    mLightPass->attach(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, mDepthStencil.get());
    mLightPass->setClearMask(GL_NONE);
    mLightPass->setRenderOrder(osg::Camera::PRE_RENDER, pre_render_pass++);
    mLightPass->setCullingMode(osg::CullSettings::NO_CULLING);
    mLightPass->setProjectionResizePolicy(osg::Camera::FIXED);
    mLightPass->setProjectionMatrixAsOrtho2D(0.0, 1.0, 0.0, 1.0);
    ss = mLightPass->getOrCreateStateSet();
    ss->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ONE));
    ss->setAttributeAndModes(new osg::Depth(osg::Depth::GEQUAL, 0.0, 1.0, false));
    ss->setTextureAttribute(0, mGBufferColors.get());
    ss->setTextureAttribute(1, mGBufferNormals.get());
    ss->setTextureAttribute(2, mGBufferPositions.get());
    ss->addUniform(new osg::Uniform("ColorTex",  0));
    ss->addUniform(new osg::Uniform("NormalTex", 1));
    ss->addUniform(new osg::Uniform("PosTex",    2));
    // Default light values
    ss->addUniform(new osg::Uniform("ambient_color", osg::Vec4f(0.2f, 0.2f, 0.2f, 1.0f)));
    ss->addUniform(new osg::Uniform("diffuse_color", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f)));
    ss->addUniform(new osg::Uniform("specular_color", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f)));
    {
        // Skip lighting for pixels that have stencil bit 0x1 set
        osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil();
        stencil->setWriteMask(0);
        stencil->setFunction(osg::Stencil::EQUAL, 0x0, 0x1);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
        ss->setAttributeAndModes(stencil.get());
    }

    // Combiner pass (combines colors, diffuse, and specular).
    mCombinerPass = createRTTCamera(osg::Camera::COLOR_BUFFER, mFinalBuffer.get());
    mCombinerPass->setNodeMask(Renderer::Mask_RTT);
    mCombinerPass->attach(osg::Camera::PACKED_DEPTH_STENCIL_BUFFER, mDepthStencil.get());
    mCombinerPass->setClearMask(GL_NONE);
    mCombinerPass->setRenderOrder(osg::Camera::PRE_RENDER, pre_render_pass++);
    mCombinerPass->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    mCombinerPass->setProjectionResizePolicy(osg::Camera::FIXED);
    mCombinerPass->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
    ss = setShaderProgram(mCombinerPass.get(), "shaders/combiner.vert", "shaders/combiner.frag");
    ss->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false),
                             osg::StateAttribute::OFF);
    ss->setTextureAttribute(0, mGBufferColors.get());
    ss->setTextureAttribute(1, mDiffuseLight.get());
    ss->setTextureAttribute(2, mSpecularLight.get());
    ss->addUniform(new osg::Uniform("ColorTex",    0));
    ss->addUniform(new osg::Uniform("DiffuseTex",  1));
    ss->addUniform(new osg::Uniform("SpecularTex", 2));
    mCombinerPass->addChild(createScreenQuad(osg::Vec2f(), 1.0f, 1.0f, mTextureWidth, mTextureHeight));

    // Final output to back buffer
    mOutputPass = new osg::Camera();
    mOutputPass->setNodeMask(Renderer::Mask_RTT);
    mOutputPass->setClearMask(GL_NONE);
    mOutputPass->setRenderOrder(osg::Camera::POST_RENDER, -1);
    mOutputPass->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    mOutputPass->setProjectionResizePolicy(osg::Camera::FIXED);
    mOutputPass->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
    mOutputPass->setViewport(0, 0, mScreenWidth, mScreenHeight);
    mOutputPass->setAllowEventFocus(false);
    ss = setShaderProgram(mOutputPass.get(), "shaders/quad_rect.vert", "shaders/quad_rect.frag");
    ss->setTextureAttribute(0, mFinalBuffer.get());
    ss->addUniform(new osg::Uniform("ImageTex", 0));
    ss->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false),
                             osg::StateAttribute::OFF);
    mOutputPass->addChild(createScreenQuad(osg::Vec2f(), 1.0f, 1.0f, mTextureWidth, mTextureHeight));

    // Graph.
    mGraph = new osg::Group();

    mGraph->addChild(mClearPass.get());
    mGraph->addChild(mMainPass.get());
    mGraph->addChild(mLightPass.get());
    mGraph->addChild(mCombinerPass.get());
    mGraph->addChild(mOutputPass.get());
}

void RenderPipeline::deinitialize()
{
    mGraph = nullptr;
    mClearPass = nullptr;
    mMainPass = nullptr;
    mLightPass = nullptr;
    mCombinerPass = nullptr;
    mOutputPass = nullptr;

    mGBufferColors    = nullptr;
    mGBufferNormals   = nullptr;
    mGBufferPositions = nullptr;
    mDepthStencil     = nullptr;

    mDiffuseLight  = nullptr;
    mSpecularLight = nullptr;

    mFinalBuffer = nullptr;

    mDebugMapDisplay = nullptr;
}


osg::Node* RenderPipeline::createDirectionalLight()
{
    osg::ref_ptr<osg::Geode> light = createScreenQuad(osg::Vec2f(0.0f, 0.0f), 1.0f, 1.0f,
                                                      mTextureWidth, mTextureHeight);
    osg::StateSet *ss = setShaderProgram(light, "shaders/dir_light.vert", "shaders/dir_light.frag");
    ss->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false),
                             osg::StateAttribute::OFF);

    mLightPass->addChild(light.get());
    return light.release();
}

void RenderPipeline::removeDirectionalLight(osg::Node *node)
{
    mLightPass->removeChild(node);
}


void RenderPipeline::toggleDebugMapDisplay()
{
    if(mDebugMapDisplay.valid())
    {
        mGraph->removeChild(mDebugMapDisplay.get());
        mDebugMapDisplay = nullptr;
        return;
    }

    mDebugMapDisplay = new osg::Camera;
    mDebugMapDisplay->setNodeMask(Renderer::Mask_RTT);
    mDebugMapDisplay->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    mDebugMapDisplay->setProjectionResizePolicy(osg::Camera::FIXED);
    mDebugMapDisplay->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
    mDebugMapDisplay->setViewport(0, 0, mScreenWidth, mScreenHeight);
    mDebugMapDisplay->setClearMask(GL_NONE);
    mDebugMapDisplay->setRenderOrder(osg::Camera::POST_RENDER, -1);
    mDebugMapDisplay->setAllowEventFocus(false);
    osg::StateSet *ss = setShaderProgram(mDebugMapDisplay.get(), "shaders/quad_rect.vert", "shaders/quad_rect.frag");
    ss->setAttribute(
        new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL),
        osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED
    );
    ss->setAttribute(new osg::Depth(osg::Depth::ALWAYS, 0.0, 1.0, false));
    ss->addUniform(new osg::Uniform("TexImage", 0));

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::ref_ptr<osg::Geometry> geom;
    geom = createScreenGeometry(osg::Vec2f(0.375f, 0.74f), 0.25f, 0.25f, mScreenWidth, mScreenHeight);
    geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, mGBufferPositions.get());
    geode->addDrawable(geom.get());

    geom = createScreenGeometry(osg::Vec2f(0.74f, 0.74f), 0.25f, 0.25f, mScreenWidth, mScreenHeight);
    geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, mGBufferNormals.get());
    geode->addDrawable(geom.get());

    geom = createScreenGeometry(osg::Vec2f(0.01f, 0.74f), 0.25f, 0.25f, mScreenWidth, mScreenHeight);
    geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, mGBufferColors.get());
    geode->addDrawable(geom.get());

    geom = createScreenGeometry(osg::Vec2f(0.01f, 0.375f), 0.25f, 0.25f, mScreenWidth, mScreenHeight);
    geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, mDiffuseLight.get());
    geode->addDrawable(geom.get());

    geom = createScreenGeometry(osg::Vec2f(0.74f, 0.375f), 0.25f, 0.25f, mScreenWidth, mScreenHeight);
    geom->getOrCreateStateSet()->setTextureAttributeAndModes(0, mSpecularLight.get());
    geode->addDrawable(geom.get());

    mDebugMapDisplay->addChild(geode.get());
    mGraph->addChild(mDebugMapDisplay.get());
}

} // namespace DF
