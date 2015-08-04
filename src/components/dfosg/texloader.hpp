#ifndef COMPONENTS_DFOSG_TEXLOADER_HPP
#define COMPONENTS_DFOSG_TEXLOADER_HPP

#include <vector>

#include <osg/ref_ptr>

#include "components/resource/texturemanager.hpp"


namespace osg
{
    class Image;
}

namespace DFOSG
{

typedef std::vector<osg::ref_ptr<osg::Image>> ImagePtrArray;

class TexLoader {
    static TexLoader sLoader;

    TexLoader(const TexLoader&) = delete;
    TexLoader& operator=(const TexLoader&) = delete;

    TexLoader();
    ~TexLoader();

    osg::Image *createDummyImage();

    osg::Image *loadUncompressedSingle(size_t width, size_t height,
                                       const Resource::Palette &palette,
                                       std::istream &stream);
    void loadUncompressedMulti(osg::Image *image, const Resource::Palette &palette,
                               std::istream &stream);

public:
    ImagePtrArray load(size_t idx, int16_t *xoffset, int16_t *yoffset, int16_t *xscale, int16_t *yscale, const Resource::Palette &palette);

    ImagePtrArray load(size_t idx, const Resource::Palette &palette)
    {
        int16_t xoffset, yoffset, xscale, yscale;
        return load(idx, &xoffset, &yoffset, &xscale, &yscale, palette);
    }

    static TexLoader &get() { return sLoader; }
};

} // namespace DFOSG

#endif /* COMPONENTS_DFOSG_TEXLOADER_HPP */
