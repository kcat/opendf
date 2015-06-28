
#include "engine.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <SDL.h>
#include <SDL_syswm.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>

#include "components/sdlutil/graphicswindow.hpp"


namespace DF
{

Engine::Engine(void)
  : mSDLWindow(nullptr)
{
}

Engine::~Engine(void)
{
    mSceneRoot = nullptr;
    mCamera = nullptr;

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
            break;
        case SDL_MOUSEWHEEL:
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            break;
        case SDL_TEXTINPUT:
            break;

        case SDL_QUIT:
            return false;
        }
    }

    return true;
}


bool Engine::go(void)
{
    // Init everything except audio (we will use OpenAL for that)
    if(SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_AUDIO) != 0)
    {
        std::stringstream sstr;
        sstr<< "SDL_Init Error: "<<SDL_GetError();
        throw std::runtime_error(sstr.str());
    }

    // Configure
    osg::ref_ptr<osgViewer::Viewer> viewer;
    {
        int width = 1024;
        int height = 768;
        int xpos = SDL_WINDOWPOS_CENTERED;
        int ypos = SDL_WINDOWPOS_CENTERED;
        Uint32 flags = SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN;

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, SDL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        std::cout<< "Creating window "<<width<<"x"<<height<<", flags 0x"<<std::hex<<flags <<std::endl;
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
        traits->alpha = 8;
        traits->depth = 24;
        traits->stencil = 8;
        traits->doubleBuffer = true;
        traits->inheritedWindowData = new SDLUtil::GraphicsWindowSDL2::WindowData(mSDLWindow);

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
        if(!gc.valid()) throw std::runtime_error("Failed to create GraphicsContext");
        //gc->getState()->setUseModelViewAndProjectionUniforms(true);
        //gc->getState()->setUseVertexAttributeAliasing(true);

        mCamera = new osg::Camera();
        mCamera->setGraphicsContext(gc.get());
        mCamera->setViewport(0, 0, width, height);
        mCamera->setProjectionResizePolicy(osg::Camera::FIXED);
        mCamera->setProjectionMatrix(osg::Matrix::identity());

        viewer = new osgViewer::Viewer();
        viewer->setCamera(mCamera.get());
    }
    SDL_ShowCursor(0);

    mSceneRoot = new osg::Group();

    viewer->setSceneData(mSceneRoot);
    viewer->requestContinuousUpdate();
    viewer->setLightingMode(osg::View::NO_LIGHT);
    viewer->addEventHandler(new osgViewer::StatsHandler);
    viewer->realize();

    // And away we go!
    Uint32 last_tick = SDL_GetTicks();
    while(!viewer->done() && pumpEvents())
    {
        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        if(keystate[SDL_SCANCODE_ESCAPE])
            break;

        Uint32 current_tick = SDL_GetTicks();
        Uint32 tick_count = current_tick - last_tick;
        last_tick = current_tick;

        viewer->frame(tick_count / 1000.0);
    }

    return true;
}

} // namespace DF
