
#include "texturemanager.hpp"

#include <sstream>
#include <iomanip>

#include <osg/Vec3ub>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/Texture2DArray>

#include "components/vfs/manager.hpp"
#include "components/dfosg/texloader.hpp"


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

    if(len != sizeof(mCurrentPalette))
        throw std::runtime_error("Invalid palette size (expected 768 or 776 bytes)");

    stream->read(reinterpret_cast<char*>(mCurrentPalette.data()), sizeof(mCurrentPalette));
}


osg::ref_ptr<osg::Texture> TextureManager::get(size_t idx, int16_t *xoffset, int16_t *yoffset, float *xscale, float *yscale)
{
    auto iter = mTexCache.find(idx);
    if(iter != mTexCache.end())
    {
        osg::ref_ptr<osg::Texture> tex;
        if(iter->second.mTexture.lock(tex))
        {
            *xoffset = iter->second.mXOffset;
            *yoffset = iter->second.mYOffset;
            *xscale = iter->second.mXScale;
            *yscale = iter->second.mYScale;
            return tex;
        }
    }

    int16_t x_offset, y_offset, x_scale, y_scale;
    std::vector<osg::ref_ptr<osg::Image>> images = DFOSG::TexLoader::get().load(
        idx, &x_offset, &y_offset, &x_scale, &y_scale, mCurrentPalette
    );
    *xoffset = x_offset;
    *yoffset = y_offset;
    *xscale = 1.0f + x_scale/256.0f;
    *yscale = 1.0f + y_scale/256.0f;
    if(images.empty())
        return osg::ref_ptr<osg::Texture>();

    osg::ref_ptr<osg::Texture> tex;
    if(images.size() == 1)
    {
        osg::ref_ptr<osg::Texture2D> tex2d(new osg::Texture2D(images[0]));
        tex2d->setTextureSize(images[0]->s(), images[0]->t());
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
        tex = tex2d;
#endif
    }

    tex->setResizeNonPowerOfTwoHint(false);
    tex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    tex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    tex->setUnRefImageDataAfterApply(true);
    // Filter should be configurable. Defaults to nearest to retain DF's pixely
    // look (with linear mipmapping to reduce aliasing).
    tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
    tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);

    mTexCache[idx] = TextureInfo{tex, x_offset, y_offset, 1.0f + x_scale/256.0f, 1.0f + y_scale/256.0f};
    return tex;
}

osg::ref_ptr<osg::Texture> TextureManager::get(size_t idx)
{
    int16_t xoffset, yoffset;
    float xscale, yscale;
    return get(idx, &xoffset, &yoffset, &xscale, &yscale);
}


} // namespace Resource
