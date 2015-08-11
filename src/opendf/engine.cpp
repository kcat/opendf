
#include "engine.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

#include <sys/stat.h>
#include <sys/types.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Depth>
#include <osg/CullFace>

#include "components/sdlutil/graphicswindow.hpp"
#include "components/vfs/manager.hpp"
#include "components/settings/configfile.hpp"
#include "components/resource/texturemanager.hpp"
#include "components/resource/meshmanager.hpp"
#include "components/dfosg/meshloader.hpp"

#include "render/pipeline.hpp"
#include "gui/iface.hpp"
#include "input/input.hpp"
#include "world/iface.hpp"
#include "cvars.hpp"
#include "log.hpp"

#ifdef _WIN32
#include <direct.h>
#define mkdir(x,y) _mkdir(x)
#define S_IRWXU 0
#endif


namespace
{

std::vector<std::string> getGlobalConfigDirs()
{
    std::vector<std::string> paths;
#ifdef _WIN32
    // ???
#else
    const char *str = getenv("XDG_CONFIG_DIRS");
    if(!str || str[0] == 0)
        str = "/etc/xdg";

    /* Go through the list in reverse, since "the order of base directories
     * denotes their importance; the first directory listed is the most
     * important". Ergo, we need to load the settings from the later dirs
     * first so that the settings in the earlier dirs override them.
     */
    std::string xdg_paths = str;
    while(1)
    {
        size_t pos = xdg_paths.find_last_of(':');
        if(pos == std::string::npos)
        {
            paths.push_back(xdg_paths);
            break;
        }

        paths.push_back(xdg_paths.substr(pos+1));
        xdg_paths.resize(pos);
    }
#endif
    return paths;
}

std::string getUserConfigDir()
{
    std::string path;
#ifdef _WIN32
    const char *base = getenv("AppData");
    if(base) path = base;
#else
    const char *base = getenv("XDG_CONFIG_HOME");
    if(base && base[0] != 0)
        path = base;
    else
    {
        base = getenv("HOME");
        if(base) path = base;
        path += "/.config";
    }
#endif
    return path;
}

void makeDirRecurse(std::string path)
{
    int err = mkdir(path.c_str(), S_IRWXU);
    if(err != 0 && errno == ENOENT)
    {
        size_t pos = path.find_last_of('/');
        if(pos != std::string::npos)
        {
            makeDirRecurse(path.substr(0, pos));
            err = mkdir(path.c_str(), S_IRWXU);
        }
    }
    if(err != 0)
    {
        std::stringstream sstr;
        sstr<< "Failed to create "<<path<<": "<<strerror(errno)<<" ("<<errno<<")";
        throw std::runtime_error(sstr.str());
    }
}

}

namespace DF
{

CVAR(CVarInt, vid_width, 1280, 0);
CVAR(CVarInt, vid_height, 720, 0);
CVAR(CVarBool, vid_fullscreen, false);

CCMD(qqq)
{
    SDL_Event evt{};
    evt.quit.type = SDL_QUIT;
    SDL_PushEvent(&evt);
}

CCMD(savecfg)
{
    static const std::string default_cfg("opendf.cfg");
    std::string cfg_name = (params.empty() ? getUserConfigDir()+"/opendf/"+default_cfg : params);

    Log::get().stream()<< "Saving config "<<cfg_name<<"...";
    std::ofstream ocfg(cfg_name, std::ios_base::binary);
    if(!ocfg.is_open())
    {
        size_t pos = cfg_name.find_last_of('/');
        if(pos != std::string::npos)
        {
            makeDirRecurse(cfg_name.substr(0, pos));
            ocfg.open(cfg_name, std::ios_base::binary);
        }
        if(!ocfg.is_open())
            throw std::runtime_error("Failed to open "+cfg_name+" for writing");
    }

    std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    ocfg<< "# File saved on "<<std::ctime(&end_time);

    ocfg<< std::endl<<"[CVars]" <<std::endl;
    CVar::writeAll(ocfg);
}


Engine::Engine(void)
  : mSDLWindow(nullptr)
{
}

Engine::~Engine(void)
{
    RenderPipeline::get().deinitialize();

    Resource::MeshManager::get().deinitialize();

    WorldIface::get().deinitialize();

    Log::get().setGuiIface(nullptr);
    GuiIface::get().deinitialize();

    mSceneRoot = nullptr;
    mCamera = nullptr;

    Input::get().deinitialize();

    if(mSDLWindow)
    {
        // If we don't do this, the desktop resolution is not restored on exit
        SDL_SetWindowFullscreen(mSDLWindow, 0);
        SDL_DestroyWindow(mSDLWindow);
        mSDLWindow = nullptr;
    }
    SDL_Quit();
}

bool Engine::parseOptions(int argc, char *argv[])
{
    for(int i = 1;i < argc;i++)
    {
        if(strcasecmp(argv[i], "-data") == 0)
        {
            if(i < argc-1)
                mRootPaths.push_back(argv[++i]);
        }
        else if(strcasecmp(argv[i], "-log") == 0)
        {
            if(i < argc-1)
                Log::get().setLog(argv[++i]);
        }
        else if(strcasecmp(argv[i], "-devparm") == 0)
            Log::get().setLevel(Log::Level_Debug);
        else
        {
            std::stringstream str;
            str<< "Unrecognized option: "<<argv[i];
            throw std::runtime_error(str.str());
        }
    }

    return true;
}


void Engine::handleWindowEvent(const SDL_WindowEvent &evt)
{
    switch(evt.event)
    {
        case SDL_WINDOWEVENT_MOVED:
        {
            int width, height;
            SDL_GetWindowSize(mSDLWindow, &width, &height);
            mCamera->getGraphicsContext()->resized(evt.data1, evt.data2, width, height);
            break;
        }
        case SDL_WINDOWEVENT_RESIZED:
        {
            int x, y;
            SDL_GetWindowPosition(mSDLWindow, &x, &y);
            mCamera->getGraphicsContext()->resized(x, y, evt.data1, evt.data2);
            break;
        }

        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_HIDDEN:
            break;

        case SDL_WINDOWEVENT_EXPOSED:
            // Needs redraw
            break;

        case SDL_WINDOWEVENT_ENTER:
        case SDL_WINDOWEVENT_LEAVE:
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        case SDL_WINDOWEVENT_FOCUS_LOST:
            break;

        case SDL_WINDOWEVENT_CLOSE:
            // FIXME: Inject an SDL_QUIT event? Seems to happen anyway...
            break;

        default:
            std::cerr<< "Unhandled window event: "<<(int)evt.event <<std::endl;
    }
}

bool Engine::pumpEvents()
{
    SDL_PumpEvents();

    SDL_Event evt;
    while(SDL_PollEvent(&evt))
    {
        switch(evt.type)
        {
        case SDL_WINDOWEVENT:
            handleWindowEvent(evt.window);
            break;

        case SDL_MOUSEMOTION:
            Input::get().handleMouseMotionEvent(evt.motion);
            break;
        case SDL_MOUSEWHEEL:
            Input::get().handleMouseWheelEvent(evt.wheel);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            Input::get().handleMouseButtonEvent(evt.button);
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            Input::get().handleKeyboardEvent(evt.key);
            break;
        case SDL_TEXTINPUT:
            Input::get().handleTextInputEvent(evt.text);
            break;

        case SDL_QUIT:
            return false;
        }
    }

    return true;
}


bool Engine::go(void)
{
    Log::get().initialize();

    // Init everything except audio (we will use OpenAL for that)
    Log::get().message("Initializing SDL...");
    if(SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_AUDIO) != 0)
    {
        std::stringstream sstr;
        sstr<< "SDL_Init Error: "<<SDL_GetError();
        throw std::runtime_error(sstr.str());
    }

    {
        std::string cfgname = getUserConfigDir() + "/opendf/opendf.cfg";
        Log::get().stream()<< "Loading "<<cfgname<<"...";

        Settings::ConfigFile cf;
        cf.load(cfgname);

        Log::get().message("Loading cvar values...");
        const Settings::ConfigSection &cvars = cf.getSection("CVars");
        for(const Settings::ConfigEntry &cvar : cvars)
            CVar::setByName(cvar.first, cvar.second);
    }

    Log::get().message("Initializing VFS...");
    {
        std::vector<std::string> cfg_paths = getGlobalConfigDirs();
        cfg_paths.push_back(getUserConfigDir());
        cfg_paths.push_back(".");

        Settings::ConfigFile cf;
        for(std::string path : cfg_paths)
        {
            if(!path.empty())
            {
                if(path == ".")
                    path += "/settings.cfg";
                else
                    path += "/opendf/settings.cfg";
                Log::get().stream()<< "Loading "<<path<<"...";
                cf.load(path);
            }
        }

        std::string root_path = cf.getOption("data-root", std::string());
        if(root_path.empty())
        {
            std::string user_path = getUserConfigDir();
            if(user_path.empty()) user_path = "settings.cfg";
            else user_path += "/opendf/settings.cfg";
            std::stringstream sstr;
            sstr<< "No root path found. Please create or edit\n"<<
            user_path<<"\n"<<
            "and add:\n"<<
            "data-root = C:\\DAGGER\\ARENA2\n"<<
            "where C:\\DAGGER is your Daggerfall install folder";
            Log::get().message(sstr.str(), Log::Level_Error);
            throw std::runtime_error(sstr.str());
        }

        Log::get().stream()<< "  Setting root path "<<root_path<<"...";
        VFS::Manager::get().initialize(root_path.c_str());

        Settings::ConfigMultiEntryRange paths = cf.getMultiOptionRange("data");
        Settings::ConfigSection::const_iterator path = paths.first;
        for(;path != paths.second;++path)
        {
            Log::get().stream()<< "  Adding data path "<<path->second<<"...";
            VFS::Manager::get().addDataPath(path->second.c_str());
        }
    }

    {
        auto path = mRootPaths.begin();
        for(;path != mRootPaths.end();++path)
        {
            Log::get().stream()<< "  Adding data path "<<*path<<"...";
            VFS::Manager::get().addDataPath(*path);
        }
    }

    // Configure
    osg::ref_ptr<osgViewer::Viewer> viewer;
    {
        int width = *vid_width;
        int height = *vid_height;
        int xpos = SDL_WINDOWPOS_CENTERED;
        int ypos = SDL_WINDOWPOS_CENTERED;
        Uint32 flags = SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN;
        if(*vid_fullscreen)
            flags |= SDL_WINDOW_FULLSCREEN;

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, SDL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        Log::get().stream()<< "Creating window "<<width<<"x"<<height<<", flags 0x"<<std::hex<<flags;
        mSDLWindow = SDL_CreateWindow("OpenDF", xpos, ypos, width, height, flags);
        if(mSDLWindow == nullptr)
        {
            std::stringstream sstr;
            sstr<< "SDL_CreateWindow Error: "<<SDL_GetError();
            throw std::runtime_error(sstr.str());
        }

        SDLUtil::graphicswindow_SDL2();
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        SDL_GetWindowPosition(mSDLWindow, &traits->x, &traits->y);
        SDL_GetWindowSize(mSDLWindow, &traits->width, &traits->height);
        traits->windowName = SDL_GetWindowTitle(mSDLWindow);
        traits->windowDecoration = !(SDL_GetWindowFlags(mSDLWindow)&SDL_WINDOW_BORDERLESS);
        traits->screenNum = SDL_GetWindowDisplayIndex(mSDLWindow);
        // FIXME: Some way to get these settings back from the SDL window?
        traits->red = 8;
        traits->green = 8;
        traits->blue = 8;
        traits->alpha = 0;
        traits->depth = 24;
        traits->stencil = 8;
        traits->doubleBuffer = true;
        traits->inheritedWindowData = new SDLUtil::GraphicsWindowSDL2::WindowData(mSDLWindow);

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        if(!gc.valid()) throw std::runtime_error("Failed to create GraphicsContext");
        gc->getState()->setUseModelViewAndProjectionUniforms(true);
        gc->getState()->setUseVertexAttributeAliasing(true);

        mCamera = new osg::Camera();
        mCamera->setGraphicsContext(gc.get());
        mCamera->setViewport(0, 0, width, height);
        mCamera->setProjectionResizePolicy(osg::Camera::FIXED);
        mCamera->setProjectionMatrix(osg::Matrix::identity());

        viewer = new osgViewer::Viewer();
        viewer->setCamera(mCamera.get());
    }
    SDL_ShowCursor(0);

    Log::get().message("Initializing Texture Manager...");
    Resource::TextureManager::get().initialize();

    Log::get().message("Initializing Mesh Manager...");
    Resource::MeshManager::get().initialize();

    Log::get().message("Initializing Input...");
    Input::get().initialize(viewer);

    {
        mSceneRoot = new osg::MatrixTransform(osg::Matrix::rotate(
            3.14159f, osg::Vec3(1.0f, 0.0f, 0.0f))
        );
        osg::StateSet *ss = mSceneRoot->getOrCreateStateSet();
        ss->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, true));
        ss->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));
        ss->setMode(GL_BLEND, osg::StateAttribute::OFF);
    }

    {
        int screen_width = mCamera->getViewport()->width();
        int screen_height = mCamera->getViewport()->height();
        RenderPipeline &pipeline = RenderPipeline::get();
        pipeline.initialize(mSceneRoot.get(), screen_width, screen_height);
        pipeline.setProjectionMatrix(osg::Matrix::perspective(
            *r_fov, pipeline.getAspectRatio(), 10.0, 10000.0
        ));

        // Add a light so we can see
        osg::Vec3f lightDir(70.f, -100.f, 10.f);
        lightDir.normalize();
        osg::ref_ptr<osg::Node> light = pipeline.createDirectionalLight();
        osg::StateSet *ss = light->getOrCreateStateSet();
        ss->addUniform(new osg::Uniform("light_direction", lightDir));
        ss->addUniform(new osg::Uniform("diffuse_color", osg::Vec4f(1.0f, 0.988f, 0.933f, 1.0f)));
        ss->addUniform(new osg::Uniform("specular_color", osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f)));
        pipeline.getLightingStateSet()->getUniform("ambient_color")->set(
            osg::Vec4f(0.537f, 0.549f, 0.627f, 1.0f)
        );
    }

    {
        osg::ref_ptr<osgViewer::StatsHandler> statshandler(new osgViewer::StatsHandler());
        statshandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F3);
        viewer->addEventHandler(statshandler);
    }

    viewer->setSceneData(RenderPipeline::get().getGraphRoot());
    viewer->requestContinuousUpdate();
    viewer->setLightingMode(osg::View::NO_LIGHT);
    viewer->realize();

    Log::get().message("Initializing GUI...");
    GuiIface::get().initialize(viewer, viewer->getSceneData()->asGroup());
    Log::get().setGuiIface(&GuiIface::get());

    CVar::registerAll();

    WorldIface::get().initialize(viewer, mSceneRoot);

    // Region: Daggerfall, Location: Privateer's Hold
    WorldIface::get().loadDungeonByExterior(17, 179);

    // And away we go!
    Uint32 last_tick = SDL_GetTicks();
    while(!viewer->done() && pumpEvents())
    {
        Uint32 current_tick = SDL_GetTicks();
        Uint32 tick_count = current_tick - last_tick;
        last_tick = current_tick;
        float timediff = tick_count / 1000.0;

        Input::get().update(timediff);

        WorldIface::get().update(timediff);

        viewer->frame(timediff);
    }
    Log::get().message("Main loop shutting down...");
    mSceneRoot->removeChildren(0, mSceneRoot->getNumChildren());

    savecfg(std::string());

    return true;
}

} // namespace DF
