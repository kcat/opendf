#ifndef GUI_GUI_HPP
#define GUI_GUI_HPP

#include "iface.hpp"

#include <string>


namespace osg
{
    class Group;
}

namespace osgViewer
{
    class Viewer;
}

namespace MyGUI
{
    class Gui;
    class TextBox;
}

namespace DF
{

class Console;

class Gui : public GuiIface {
    MyGUI::Gui *mGui;

    MyGUI::TextBox *mStatusMessages;

    Console *mConsole;

    int mActiveModes;

    Gui();
    ~Gui();

public:
    static Gui sGui;

    virtual void initialize(osgViewer::Viewer *viewer, osg::Group *sceneroot) final;
    virtual void deinitialize() final;

    virtual void printToConsole(const std::string &str) final;

    virtual void addConsoleCallback(const char *command, CommandDelegateT *delegate) final;

    virtual void pushMode(Mode mode) final;
    virtual void popMode(Mode mode) final;
    virtual bool testMode(Mode mode) const final { return !!(mActiveModes&mode); }
    virtual Mode getMode() const final;

    virtual void mouseMoved(int x, int y, int z) final;
    virtual void mousePressed(int x, int y, int button) final;
    virtual void mouseReleased(int x, int y, int button) final;
    virtual void injectKeyPress(SDL_Keycode code) final;
    virtual void injectKeyRelease(SDL_Keycode code) final;
    virtual void injectTextInput(const char *text) final;

    void updateStatus(const std::string &str);
};

} // namespace DF

#endif /* GUI_GUI_HPP */
