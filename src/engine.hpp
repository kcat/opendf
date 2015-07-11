#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <vector>
#include <string>
#include <map>

#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Quat>


struct SDL_Window;
struct SDL_WindowEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseWheelEvent;
struct SDL_MouseButtonEvent;
struct SDL_KeyboardEvent;
struct SDL_TextInputEvent;

namespace osg
{
    class Group;
    class Camera;
}

namespace DF
{

class Engine {
    typedef void (Engine::*CmdFuncT)(const std::string&);
    typedef std::map<std::string,CmdFuncT> CommandFuncMap;

    SDL_Window *mSDLWindow;

    std::vector<const char*> mRootPaths;
    osg::Vec3f mCameraPos;
    osg::Quat mCameraRot;

    osg::ref_ptr<osg::Group> mSceneRoot;

    osg::ref_ptr<osg::Camera> mCamera;

    void handleWindowEvent(const SDL_WindowEvent &evt);
    bool pumpEvents();

public:
    Engine(void);
    virtual ~Engine(void);

    bool parseOptions(int argc, char *argv[]);

    bool go(void);
};

} // namespace DF

#endif /* ENGINE_HPP */
