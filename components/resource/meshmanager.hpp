#ifndef COMPONENTS_RESOURCE_MESHMANAGER_HPP
#define COMPONENTS_RESOURCE_MESHMANAGER_HPP

#include <map>

#include <osg/ref_ptr>


namespace osg
{
    class Matrixf;
    class Node;
}

namespace Resource
{

class MeshManager {
    static MeshManager sManager;

    std::map<size_t,osg::observer_ptr<osg::Node>> mModelCache;

    MeshManager();
    ~MeshManager();

public:
    void initialize();

    osg::ref_ptr<osg::Node> get(size_t idx);

    /* Loads a billboard flat for the given texture (see TextureManager::get).
     * Optionally returns the number of frames in the loaded texture, and a
     * scale+translate matrix with its X/Y scale and offset.
     */
    osg::ref_ptr<osg::Node> loadFlat(size_t texid, size_t *num_frames=nullptr, osg::Matrixf *mtx=nullptr);

    static MeshManager &get() { return sManager; }
};

} // namespace Resource

#endif /* COMPONENTS_RESOURCE_MESHMANAGER_HPP */
