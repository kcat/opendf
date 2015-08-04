#ifndef COMPONENTS_RESOURCE_MESHMANAGER_HPP
#define COMPONENTS_RESOURCE_MESHMANAGER_HPP

#include <map>

#include <osg/ref_ptr>


namespace osg
{
    class Matrixf;
    class Node;
    class StateSet;
    class Program;
}

namespace Resource
{

class MeshManager {
    static MeshManager sManager;

    std::map<size_t,osg::observer_ptr<osg::Node>> mModelCache;
    std::map<size_t,osg::observer_ptr<osg::StateSet>> mStateSetCache;
    std::map<std::pair<size_t,bool>,osg::observer_ptr<osg::Node>> mFlatCache;

    osg::ref_ptr<osg::Program> mModelProgram;
    osg::ref_ptr<osg::Program> mFlatProgram;

    MeshManager();
    ~MeshManager();

public:
    void initialize();
    void deinitialize();

    osg::ref_ptr<osg::Node> get(size_t idx);

    /* Loads a billboard flat for the given texture (see TextureManager::get),
     * with either a centered billboard or one rooted on its bottom. Optionally
     * returns the number of frames in the loaded texture.
     */
    osg::ref_ptr<osg::Node> loadFlat(size_t texid, bool centered, size_t *num_frames=nullptr);

    static MeshManager &get() { return sManager; }
};

} // namespace Resource

#endif /* COMPONENTS_RESOURCE_MESHMANAGER_HPP */
