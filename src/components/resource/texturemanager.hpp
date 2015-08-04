#ifndef COMPONENTS_RESOURCE_TEXTUREMANAGER_HPP
#define COMPONENTS_RESOURCE_TEXTUREMANAGER_HPP

#include <string>
#include <array>
#include <map>

#include <osg/ref_ptr>
#include <osg/observer_ptr>


namespace osg
{
    class Texture;
}

namespace Resource
{

struct PaletteEntry {
    unsigned char r, g, b;
};
typedef std::array<PaletteEntry,256> Palette;

struct TextureInfo {
    osg::observer_ptr<osg::Texture> mTexture;

    int16_t mXOffset, mYOffset;
    float mXScale, mYScale;
};

class TextureManager {
    static TextureManager sManager;

    Palette mCurrentPalette;
    static_assert(sizeof(mCurrentPalette)==768, "Palette is not 768 bytes");

    std::map<size_t,TextureInfo> mTexCache;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    TextureManager();
    ~TextureManager();

public:
    void initialize();

    const Palette &getCurrentPalette() const { return mCurrentPalette; }

    // The index has the TEXTURE.??? file number in the upper nine bits, and
    // the image index in the lower 7 bits.
    osg::ref_ptr<osg::Texture> getTexture(size_t idx, int16_t *xoffset, int16_t *yoffset, float *xscale, float *yscale);
    osg::ref_ptr<osg::Texture> getTexture(size_t idx);

    static TextureManager &get() { return sManager; }
};

} // namespace Resource

#endif /* COMPONENTS_RESOURCE_TEXTUREMANAGER_HPP */
