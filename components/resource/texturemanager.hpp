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

class TextureManager {
    static TextureManager sManager;

    std::map<size_t,osg::observer_ptr<osg::Texture>> mTexCache;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    TextureManager();
    ~TextureManager();

public:
    void initialize();

    // The index has the TEXTURE.??? file number in the upper nine bits, and
    // the image index in the lower 7 bits.
    osg::ref_ptr<osg::Texture> get(size_t idx);

    static TextureManager &get() { return sManager; }
};

} // namespace Resource

#endif /* COMPONENTS_RESOURCE_TEXTUREMANAGER_HPP */
