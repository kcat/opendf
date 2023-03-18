#ifndef COMPONENTS_MYGUI_OSG_TEXTURE_H
#define COMPONENTS_MYGUI_OSG_TEXTURE_H

#include <cstddef>
#include <MYGUI/MyGUI_ITexture.h>

#include <osg/ref_ptr>

namespace osg
{
    class Image;
    class Texture2D;
}

namespace MyGUI_OSG
{

class Texture : public MyGUI::ITexture {
    std::string mName;

    osg::ref_ptr<osg::Image> mLockedImage;
    osg::ref_ptr<osg::Texture2D> mTexture;
    MyGUI::PixelFormat mFormat;
    MyGUI::TextureUsage mUsage;
    size_t mNumElemBytes;

public:
    Texture(const std::string &name);
    virtual ~Texture();

    virtual const std::string& getName() const { return mName; }

    virtual void createManual(int width, int height, MyGUI::TextureUsage usage, MyGUI::PixelFormat format);
    virtual void loadFromFile(const std::string &fname);
    virtual void saveToFile(const std::string &fname);

    virtual void destroy();

    virtual void* lock(MyGUI::TextureUsage access);
    virtual void unlock();
    virtual bool isLocked() const;

    virtual int getWidth() const;
    virtual int getHeight() const;

    virtual MyGUI::PixelFormat getFormat() const { return mFormat; }
    virtual MyGUI::TextureUsage getUsage() const { return mUsage; }
    virtual size_t getNumElemBytes() const { return mNumElemBytes; }

    virtual MyGUI::IRenderTarget *getRenderTarget();

/*internal:*/
    osg::Texture2D *getTexture() const { return mTexture.get(); }

    //fake
    virtual void setShader(const std::string& _shaderName) {};
};

} // namespace MyGUI_OSG

#endif /* COMPONENTS_MYGUI_OSG_TEXTURE_H */
