
#include "input.hpp"

#include <SDL.h>

#include <osg/Vec3>

#include "gui/iface.hpp"
#include "world/iface.hpp"
#include "cvars.hpp"
#include "log.hpp"


namespace DF
{

CVAR(CVarBool, i_inverty, true);


Input Input::sInput;


Input::Input()
  : mMouseX(0)
  , mMouseY(0)
  , mMouseZ(0)
{
}

Input::~Input()
{
}


void Input::initialize()
{
    int ret = SDL_SetRelativeMouseMode(SDL_TRUE);
    if(ret != 0)
        Log::get().stream()<< "SDL_SetRelativeMouseMode returned "<<ret<<", "<<SDL_GetError();
}

void Input::deinitialize()
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
}


void Input::update(float timediff)
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if(keystate[SDL_SCANCODE_ESCAPE])
    {
        SDL_Event evt{};
        evt.quit.type = SDL_QUIT;
        SDL_PushEvent(&evt);
    }

    if(GuiIface::get().getMode() == GuiIface::Mode_Game)
    {
        float speed = 64.0f * timediff;
        if(keystate[SDL_SCANCODE_LSHIFT])
            speed *= 2.0f;

        osg::Vec3f movedir;
        if(keystate[SDL_SCANCODE_W])
            movedir.z() += +1.0f;
        if(keystate[SDL_SCANCODE_A])
            movedir.x() += +1.0f;
        if(keystate[SDL_SCANCODE_S])
            movedir.z() += -1.0f;
        if(keystate[SDL_SCANCODE_D])
            movedir.x() += -1.0f;
        if(keystate[SDL_SCANCODE_PAGEUP])
            movedir.y() += -1.0f;
        if(keystate[SDL_SCANCODE_PAGEDOWN])
            movedir.y() += +1.0f;
        movedir.normalize();
        movedir *= speed;

        WorldIface::get().move(movedir.x(), movedir.y(), movedir.z());
    }
}


void Input::handleMouseMotionEvent(const SDL_MouseMotionEvent &evt)
{
    if(GuiIface::get().getMode() == GuiIface::Mode_Game)
    {
        if(*i_inverty)
            WorldIface::get().rotate(evt.yrel * 0.1f, evt.xrel * 0.1f);
        else
            WorldIface::get().rotate(evt.yrel * -0.1f, evt.xrel * 0.1f);
    }

    mMouseX = evt.x;
    mMouseY = evt.y;
    GuiIface::get().mouseMoved(mMouseX, mMouseY, mMouseZ);
}

void Input::handleMouseWheelEvent(const SDL_MouseWheelEvent &evt)
{
    mMouseZ += evt.y;
    GuiIface::get().mouseMoved(mMouseX, mMouseY, mMouseZ);
}

void Input::handleMouseButtonEvent(const SDL_MouseButtonEvent &evt)
{
    if(evt.state == SDL_PRESSED)
        GuiIface::get().mousePressed(evt.x, evt.y, evt.button);
    else if(evt.state == SDL_RELEASED)
        GuiIface::get().mouseReleased(evt.x, evt.y, evt.button);
}

void Input::handleKeyboardEvent(const SDL_KeyboardEvent &evt)
{
    if(evt.state == SDL_PRESSED)
    {
        if(evt.repeat)
            return;

        GuiIface::get().injectKeyPress(evt.keysym.sym);
        if(evt.keysym.sym == SDLK_BACKQUOTE)
        {
            if(!GuiIface::get().testMode(GuiIface::Mode_Console))
            {
                GuiIface::get().pushMode(GuiIface::Mode_Console);
                if(SDL_GetRelativeMouseMode())
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                if(!SDL_IsTextInputActive())
                    SDL_StartTextInput();
            }
            else
            {
                GuiIface::get().popMode(GuiIface::Mode_Console);
                if(GuiIface::get().getMode() == GuiIface::Mode_Game)
                {
                    SDL_StopTextInput();
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
            }
        }
    }
    else if(evt.state == SDL_RELEASED)
        GuiIface::get().injectKeyRelease(evt.keysym.sym);
}

void Input::handleTextInputEvent(const SDL_TextInputEvent &evt)
{
    GuiIface::get().injectTextInput(evt.text);
}

} // namespace DF
