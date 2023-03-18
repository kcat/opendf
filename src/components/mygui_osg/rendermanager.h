#ifndef COMPONENTS_MYGUI_OSG_RENDERMANAGER_H
#define COMPONENTS_MYGUI_OSG_RENDERMANAGER_H

#include <MYGUI/MyGUI_RenderManager.h>

#include <osg/ref_ptr>

namespace osg
{
    class Group;
    class Camera;
    class RenderInfo;
}

namespace osgViewer
{
    class Viewer;
}


namespace MyGUI_OSG
{

class RenderManager : public MyGUI::RenderManager, public MyGUI::IRenderTarget
{
    osg::ref_ptr<osgViewer::Viewer> mViewer;
    osg::ref_ptr<osg::Group> mSceneRoot;

    MyGUI::IntSize mViewSize;
    bool mUpdate;
    MyGUI::VertexColourType mVertexFormat;
    MyGUI::RenderTargetInfo mInfo;

    typedef std::map<std::string, MyGUI::ITexture*> MapTexture;
    MapTexture mTextures;

    bool mIsInitialise;

    osg::ref_ptr<osg::Camera> mGuiRoot;

    // Only valid during drawFrame()!
    osg::RenderInfo *mRenderInfo;

    void destroyAllResources();

public:
    RenderManager(osgViewer::Viewer *viewer, osg::Group *sceneroot);
    virtual ~RenderManager();

    void initialise();

    static RenderManager& getInstance() { return *getInstancePtr(); };
    static RenderManager* getInstancePtr()
    { return static_cast<RenderManager*>(MyGUI::RenderManager::getInstancePtr()); }

    /** @see RenderManager::getViewSize */
    virtual const MyGUI::IntSize& getViewSize() const { return mViewSize; }

    /** @see RenderManager::getVertexFormat */
    virtual MyGUI::VertexColourType getVertexFormat() const { return mVertexFormat; }

    /** @see RenderManager::isFormatSupported */
    virtual bool isFormatSupported(MyGUI::PixelFormat format, MyGUI::TextureUsage usage);

    /** @see RenderManager::createVertexBuffer */
    virtual MyGUI::IVertexBuffer* createVertexBuffer();
    /** @see RenderManager::destroyVertexBuffer */
    virtual void destroyVertexBuffer(MyGUI::IVertexBuffer *buffer);

    /** @see RenderManager::createTexture */
    virtual MyGUI::ITexture* createTexture(const std::string &name);
    /** @see RenderManager::destroyTexture */
    virtual void destroyTexture(MyGUI::ITexture* _texture);
    /** @see RenderManager::getTexture */
    virtual MyGUI::ITexture* getTexture(const std::string &name);


    /** @see IRenderTarget::begin */
    virtual void begin();
    /** @see IRenderTarget::end */
    virtual void end();
    /** @see IRenderTarget::doRender */
    virtual void doRender(MyGUI::IVertexBuffer *buffer, MyGUI::ITexture *texture, size_t count);
    /** @see IRenderTarget::getInfo */
    virtual const MyGUI::RenderTargetInfo& getInfo() const { return mInfo; }

/*internal:*/
    void drawFrame(osg::RenderInfo &renderInfo);
    virtual void setViewSize(int width, int height);


    // Fake
    virtual void registerShader(
        const std::string& _shaderName,
        const std::string& _vertexProgramFile,
        const std::string& _fragmentProgramFile) {};
};

} // namespace MyGUI_OSG

#endif /* COMPONENTS_MYGUI_OSG_RENDERMANAGER_H */
