#ifndef COMPONENTS_DFOSG_MESHLOADER_HPP
#define COMPONENTS_DFOSG_MESHLOADER_HPP

#include <map>

#include <osg/ref_ptr>
#include <osg/observer_ptr>


namespace osg
{
    class Matrixf;
    class Node;
}

namespace DFOSG
{

class MeshLoader {
    static MeshLoader sLoader;

    std::map<size_t,osg::observer_ptr<osg::Node>> mModelCache;

    MeshLoader(const MeshLoader&) = delete;
    MeshLoader& operator=(const MeshLoader&) = delete;

    MeshLoader();

public:
    /* Loads a mesh by the given index (for ARCH3D.BSA), and returns a root
     * node for the object. */
    osg::ref_ptr<osg::Node> load(size_t id);

    /* Loads a billboard flat for the given texture (see TextureManager::get).
     * Optionally returns the number of frames in the loaded texture, and a
     * scale+translate matrix with its X/Y scale and offset.
     */
    osg::ref_ptr<osg::Node> loadFlat(size_t texid, size_t *num_frames=nullptr, osg::Matrixf *mtx=nullptr);

    static MeshLoader &get()
    {
        return sLoader;
    }
};

} // namespace DFOSG

#endif /* COMPONENTS_DFOSG_MESHLOADER_HPP */
