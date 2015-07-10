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

public:
    std::vector<osg::ref_ptr<osg::Image>> load(size_t idx, const Resource::Palette &palette);

    static TexLoader &get() { return sLoader; }
};

} // namespace DFOSG

#endif /* COMPONENTS_DFOSG_TEXLOADER_HPP */
