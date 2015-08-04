#ifndef INPUT_INPUT_HPP
#define INPUT_INPUT_HPP

#include <osg/ref_ptr>


struct SDL_MouseMotionEvent;
struct SDL_MouseWheelEvent;
struct SDL_MouseButtonEvent;
struct SDL_KeyboardEvent;
struct SDL_TextInputEvent;

namespace osgViewer
{
    class Viewer;
}

namespace DF
{

class Input {
    static Input sInput;

    osg::ref_ptr<osgViewer::Viewer> mViewer;

    int mMouseX;
    int mMouseY;
    int mMouseZ;

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    Input();
    ~Input();

public:
    void initialize(osgViewer::Viewer *viewer);
    void deinitialize();

    void update(float timediff);

    void handleMouseMotionEvent(const SDL_MouseMotionEvent &evt);
    void handleMouseWheelEvent(const SDL_MouseWheelEvent &evt);
    void handleMouseButtonEvent(const SDL_MouseButtonEvent &evt);
    void handleKeyboardEvent(const SDL_KeyboardEvent &evt);
    void handleTextInputEvent(const SDL_TextInputEvent &evt);

    static Input &get() { return sInput; }
};

} // namespace DF

#endif /* INPUT_INPUT_HPP */
