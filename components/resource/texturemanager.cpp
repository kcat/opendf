
#include "texturemanager.hpp"

#include <sstream>
#include <iomanip>

#include <osg/Vec3ub>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/Texture2DArray>

#include "components/vfs/manager.hpp"
#include "components/dfosg/texloader.hpp"


namespace
{

Resource::Palette gCurrentPalette;
static_assert(sizeof(gCurrentPalette)==768, "Palette is not 768 bytes");

} // namespace


namespace Resource
{

TextureManager TextureManager::sManager;


TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
}


void TextureManager::initialize()
{
    VFS::IStreamPtr stream = VFS::Manager::get().open("PAL.PAL");

    std::streamsize len = 0;
    if(stream && stream->seekg(0, std::ios_base::end))
    {
        len = stream->tellg();
        stream->seekg(0);
    }

    if(len == 776)
    {
        len -= 8;
        stream->ignore(8);
    }

    if(len != sizeof(gCurrentPalette))
        throw std::runtime_error("Invalid palette size (expected 768 or 776 bytes)");

    stream->read(reinterpret_cast<char*>(gCurrentPalette.data()), sizeof(gCurrentPalette));
}


osg::ref_ptr<osg::Texture> TextureManager::get(size_t idx)
{
    auto iter = mTexCache.find(idx);
    if(iter != mTexCache.end())
    {
        osg::ref_ptr<osg::Texture> tex;
        if(iter->second.lock(tex))
            return tex;
    }

    std::vector<osg::ref_ptr<osg::Image>> images = DFOSG::TexLoader::get().load(idx, gCurrentPalette);
    if(images.empty()) return osg::ref_ptr<osg::Texture>();

    osg::ref_ptr<osg::Texture> tex;
    if(images.size() == 1)
    {
        osg::ref_ptr<osg::Texture2D> tex2d(new osg::Texture2D(images[0]));
        tex2d->setTextureSize(images[0]->s(), images[0]->t());
        tex2d->setResizeNonPowerOfTwoHint(false);
        tex = tex2d;
    }
    else
    {
        /* Multiframe textures would ideally be loaded as a Texture2DArray and
         * animated by offseting the R texture coord. However, they don't work
         * in the fixed-function pipeline.
         */
#if 0
        osg::ref_ptr<osg::Texture2DArray> tex2darr(new osg::Texture2DArray());
        tex2darr->setTextureSize(images[0]->s(), images[0]->t(), images.size());
        tex2darr->setResizeNonPowerOfTwoHint(false);
        for(size_t i = 0;i < images.size();++i)
            tex2darr->setImage(i, images[i]);
        tex = tex2darr;
#else
        osg::ref_ptr<osg::Texture2D> tex2d(new osg::Texture2D(images[0]));
        tex2d->setTextureSize(images[0]->s(), images[0]->t());
        tex2d->setResizeNonPowerOfTwoHint(false);
        tex = tex2d;
#endif
    }

    tex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    tex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    tex->setUnRefImageDataAfterApply(true);
    // Filter should be configurable. Defaults to nearest to retain DF's pixely
    // look (with linear mipmapping to reduce aliasing).
    tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
    tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);

    mTexCache[idx] = tex;
    return tex;
}


} // namespace Resource
