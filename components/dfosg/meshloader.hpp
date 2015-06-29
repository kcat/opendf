#ifndef COMPONENTS_DFOSG_MESHLOADER_HPP
#define COMPONENTS_DFOSG_MESHLOADER_HPP

#include <map>

#include <osg/ref_ptr>
#include <osg/observer_ptr>


namespace osg
{
    class Node;
}

namespace DFOSG
{

class MeshLoader {
    std::map<size_t,osg::observer_ptr<osg::Node>> mModelCache;

    MeshLoader(const MeshLoader&) = delete;
    MeshLoader& operator=(const MeshLoader&) = delete;

    MeshLoader();

public:
    osg::ref_ptr<osg::Node> load(size_t id);

    static MeshLoader &get()
    {
        static MeshLoader loader;
        return loader;
    }
};

} // namespace DFOSG

#endif /* COMPONENTS_DFOSG_MESHLOADER_HPP */
