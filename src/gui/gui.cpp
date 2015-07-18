
#include "gui.hpp"

#include <vector>
#include <map>

#include <MyGUI.h>

#include <SDL_scancode.h>
#include <SDL_mouse.h>

#include <osg/Group>

#include "components/mygui_osg/rendermanager.h"
#include "components/mygui_osg/datamanager.h"

#include "world/iface.hpp"
#include "delegates.hpp"
#include "log.hpp"


namespace
{
    const std::map<SDL_Keycode,MyGUI::KeyCode> SDLtoMyGUIKeycode{
        {SDLK_UNKNOWN, MyGUI::KeyCode::None},

        {SDLK_a, MyGUI::KeyCode::A},
        {SDLK_b, MyGUI::KeyCode::B},
        {SDLK_c, MyGUI::KeyCode::C},
        {SDLK_d, MyGUI::KeyCode::D},
        {SDLK_e, MyGUI::KeyCode::E},
        {SDLK_f, MyGUI::KeyCode::F},
        {SDLK_g, MyGUI::KeyCode::G},
        {SDLK_h, MyGUI::KeyCode::H},
        {SDLK_i, MyGUI::KeyCode::I},
        {SDLK_j, MyGUI::KeyCode::J},
        {SDLK_k, MyGUI::KeyCode::K},
        {SDLK_l, MyGUI::KeyCode::L},
        {SDLK_m, MyGUI::KeyCode::M},
        {SDLK_n, MyGUI::KeyCode::N},
        {SDLK_o, MyGUI::KeyCode::O},
        {SDLK_p, MyGUI::KeyCode::P},
        {SDLK_q, MyGUI::KeyCode::Q},
        {SDLK_r, MyGUI::KeyCode::R},
        {SDLK_s, MyGUI::KeyCode::S},
        {SDLK_t, MyGUI::KeyCode::T},
        {SDLK_u, MyGUI::KeyCode::U},
        {SDLK_v, MyGUI::KeyCode::V},
        {SDLK_w, MyGUI::KeyCode::W},
        {SDLK_x, MyGUI::KeyCode::X},
        {SDLK_y, MyGUI::KeyCode::Y},
        {SDLK_z, MyGUI::KeyCode::Z},

        {SDLK_1, MyGUI::KeyCode::One},
        {SDLK_2, MyGUI::KeyCode::Two},
        {SDLK_3, MyGUI::KeyCode::Three},
        {SDLK_4, MyGUI::KeyCode::Four},
        {SDLK_5, MyGUI::KeyCode::Five},
        {SDLK_6, MyGUI::KeyCode::Six},
        {SDLK_7, MyGUI::KeyCode::Seven},
        {SDLK_8, MyGUI::KeyCode::Eight},
        {SDLK_9, MyGUI::KeyCode::Nine},
        {SDLK_0, MyGUI::KeyCode::Zero},

        {SDLK_RETURN,    MyGUI::KeyCode::Return},
        {SDLK_ESCAPE,    MyGUI::KeyCode::Escape},
        {SDLK_BACKSPACE, MyGUI::KeyCode::Backspace},
        {SDLK_TAB,   MyGUI::KeyCode::Tab},
        {SDLK_SPACE, MyGUI::KeyCode::Space},

        {SDLK_MINUS,  MyGUI::KeyCode::Minus},
        {SDLK_EQUALS, MyGUI::KeyCode::Equals},
        {SDLK_LEFTBRACKET,  MyGUI::KeyCode::LeftBracket},
        {SDLK_RIGHTBRACKET, MyGUI::KeyCode::RightBracket},
        {SDLK_BACKSLASH, MyGUI::KeyCode::Backslash},

        {SDLK_SEMICOLON, MyGUI::KeyCode::Semicolon},
        {SDLK_QUOTE, MyGUI::KeyCode::Apostrophe},
        {SDLK_BACKQUOTE, MyGUI::KeyCode::Grave},

        {SDLK_COMMA, MyGUI::KeyCode::Comma},
        {SDLK_PERIOD, MyGUI::KeyCode::Period},
        {SDLK_SLASH, MyGUI::KeyCode::Slash},

        {SDLK_CAPSLOCK, MyGUI::KeyCode::Capital},

        {SDLK_F1, MyGUI::KeyCode::F1},
        {SDLK_F2, MyGUI::KeyCode::F2},
        {SDLK_F3, MyGUI::KeyCode::F3},
        {SDLK_F4, MyGUI::KeyCode::F4},
        {SDLK_F5, MyGUI::KeyCode::F5},
        {SDLK_F6, MyGUI::KeyCode::F6},
        {SDLK_F7, MyGUI::KeyCode::F7},
        {SDLK_F8, MyGUI::KeyCode::F8},
        {SDLK_F9, MyGUI::KeyCode::F9},
        {SDLK_F10, MyGUI::KeyCode::F10},
        {SDLK_F11, MyGUI::KeyCode::F11},
        {SDLK_F12, MyGUI::KeyCode::F12},

        {SDLK_PRINTSCREEN, MyGUI::KeyCode::SysRq},
        {SDLK_SCROLLLOCK, MyGUI::KeyCode::ScrollLock},
        {SDLK_PAUSE, MyGUI::KeyCode::Pause},

        {SDLK_INSERT, MyGUI::KeyCode::Insert},
        {SDLK_HOME, MyGUI::KeyCode::Home},
        {SDLK_PAGEUP, MyGUI::KeyCode::PageUp},
        {SDLK_DELETE, MyGUI::KeyCode::Delete},
        {SDLK_END, MyGUI::KeyCode::End},
        {SDLK_PAGEDOWN, MyGUI::KeyCode::PageDown},

        {SDLK_RIGHT, MyGUI::KeyCode::ArrowRight},
        {SDLK_LEFT, MyGUI::KeyCode::ArrowLeft},
        {SDLK_DOWN, MyGUI::KeyCode::ArrowDown},
        {SDLK_UP, MyGUI::KeyCode::ArrowUp},

        {SDLK_NUMLOCKCLEAR, MyGUI::KeyCode::NumLock},
        {SDLK_KP_DIVIDE, MyGUI::KeyCode::Divide},
        {SDLK_KP_MULTIPLY, MyGUI::KeyCode::Multiply},
        {SDLK_KP_MINUS, MyGUI::KeyCode::Subtract},
        {SDLK_KP_PLUS, MyGUI::KeyCode::Add},
        {SDLK_KP_ENTER, MyGUI::KeyCode::NumpadEnter},
        {SDLK_KP_1, MyGUI::KeyCode::Numpad1},
        {SDLK_KP_2, MyGUI::KeyCode::Numpad2},
        {SDLK_KP_3, MyGUI::KeyCode::Numpad3},
        {SDLK_KP_4, MyGUI::KeyCode::Numpad4},
        {SDLK_KP_5, MyGUI::KeyCode::Numpad5},
        {SDLK_KP_6, MyGUI::KeyCode::Numpad6},
        {SDLK_KP_7, MyGUI::KeyCode::Numpad7},
        {SDLK_KP_8, MyGUI::KeyCode::Numpad8},
        {SDLK_KP_9, MyGUI::KeyCode::Numpad9},
        {SDLK_KP_0, MyGUI::KeyCode::Numpad0},
        {SDLK_KP_PERIOD, MyGUI::KeyCode::Decimal},

        {SDLK_LCTRL, MyGUI::KeyCode::LeftControl},
        {SDLK_LSHIFT, MyGUI::KeyCode::LeftShift},
        {SDLK_LALT, MyGUI::KeyCode::LeftAlt},
        {SDLK_LGUI, MyGUI::KeyCode::LeftWindows},
        {SDLK_RCTRL, MyGUI::KeyCode::RightControl},
        {SDLK_RSHIFT, MyGUI::KeyCode::RightShift},
        {SDLK_RALT, MyGUI::KeyCode::RightAlt},
        {SDLK_RGUI, MyGUI::KeyCode::RightWindows},
    };

    std::vector<unsigned int> utf8ToUnicode(const char *utf8)
    {
        std::vector<unsigned int> unicode;
        for(size_t i = 0;utf8[i];)
        {
            unsigned long uni;
            size_t todo;

            unsigned char ch = utf8[i++];
            if(ch <= 0x7F)
            {
                uni = ch;
                todo = 0;
            }
            else if(ch <= 0xBF)
                throw std::logic_error("not a UTF-8 string");
            else if(ch <= 0xDF)
            {
                uni = ch&0x1F;
                todo = 1;
            }
            else if(ch <= 0xEF)
            {
                uni = ch&0x0F;
                todo = 2;
            }
            else if(ch <= 0xF7)
            {
                uni = ch&0x07;
                todo = 3;
            }
            else
                throw std::logic_error("not a UTF-8 string");

            for(size_t j = 0;j < todo;++j)
            {
                if(utf8[i])
                    throw std::logic_error("not a UTF-8 string");
                unsigned char ch = utf8[i++];
                if (ch < 0x80 || ch > 0xBF)
                    throw std::logic_error("not a UTF-8 string");
                uni <<= 6;
                uni += ch & 0x3F;
            }
            if(uni >= 0xD800 && uni <= 0xDFFF)
                throw std::logic_error("not a UTF-8 string");
            if(uni > 0x10FFFF)
                throw std::logic_error("not a UTF-8 string");
            unicode.push_back(uni);
        }
        return unicode;
    }

} // namespace

namespace DF
{

class Console {
    typedef CDelegate<const std::string&,const std::string&> CommandDelegate;
    typedef std::map<MyGUI::UString, CommandDelegate> MapDelegate;

    MyGUI::VectorWidgetPtr mWidgets;
    MyGUI::Widget *mMainWidget;

    MyGUI::EditBox *mListHistory;
    MyGUI::ComboBox *mComboCommand;
    MyGUI::Button *mButtonSubmit;

    MapDelegate mDelegates;


    template<typename T=MyGUI::Widget>
    T *getWidget(const char *name)
    {
        for(MyGUI::Widget *widget : mWidgets)
        {
            MyGUI::Widget *w = widget->findWidget(name);
            if(w) return w->castType<T>();
        }
        throw std::runtime_error(std::string("Failed to find widget ")+name);
    }

    void notifyMouseButtonClick(MyGUI::Widget *_sender)
    {
        notifyComboAccept(mComboCommand, MyGUI::ITEM_NONE);
    }

    void notifyComboAccept(MyGUI::ComboBox *_sender, size_t _index)
    {
        const MyGUI::UString& command = _sender->getOnlyText();
        if(command.empty()) return;

        MyGUI::UString key = command;
        MyGUI::UString value;

        size_t pos = command.find(' ');
        if(pos != MyGUI::UString::npos)
        {
            key = command.substr(0, pos);
            value = command.substr(pos + 1);
        }

        MapDelegate::iterator iter = mDelegates.find(key);
        if(iter != mDelegates.end())
            iter->second(key, value);
        else
            addToConsole("#FF0000unknown command : #000000'"+key+"'");

        _sender->setCaption("");
    }

    void notifyButtonPressed(MyGUI::Widget *_sender, MyGUI::KeyCode _key, MyGUI::Char _char)
    {
        MyGUI::EditBox *edit = _sender->castType<MyGUI::EditBox>();
        MyGUI::UString command = edit->getCaption();
        if(command.empty()) return;

        if(_key == MyGUI::KeyCode::Tab)
        {
            // Build a space-separated list of all commands that start with
            // 'command'. Also, find the largest string portion that matches
            // those commands.
            std::stringstream sstr;
            MyGUI::UString matching;
            MapDelegate::iterator iter = mDelegates.begin();
            for(;iter != mDelegates.end();++iter)
            {
                if(iter->first.find(command) == 0)
                {
                    sstr<< iter->first;
                    matching = iter->first;
                    ++iter;
                    break;
                }
            }
            for(;iter != mDelegates.end();++iter)
            {
                if(iter->first.find(command) == 0)
                {
                    sstr<< " "<<iter->first;

                    size_t len = std::min(matching.length(), iter->first.length());
                    auto nonmatch = std::mismatch(matching.begin(), matching.begin()+len, iter->first.begin(),
                                                  std::equal_to<MyGUI::UString::value_type>()).first;
                    if(nonmatch != matching.end())
                        matching.erase(nonmatch, matching.end());
                }
            }

            if(sstr.tellp() == 0)
                Log::get().stream()<< "No matches for \""<<command<<"\"";
            else
            {
                std::string str = sstr.str();
                if(str == matching.asUTF8())
                    matching.push_back(' ');
                else
                    Log::get().stream()<< "Auto-complete list for \""<<command<<"\":\n"<<str;
                if(matching.length() > command.length())
                    edit->setCaption(matching);
            }
        }
    }

    void addToConsole(const MyGUI::UString &_line)
    {
        if(mListHistory->getCaption().empty())
            mListHistory->addText(_line);
        else
            mListHistory->addText("\n" + _line);
        mListHistory->setTextSelection(mListHistory->getTextLength(), mListHistory->getTextLength());
    }

    void clearConsole()
    {
        mListHistory->setCaption("");
    }

    void registerConsoleDelegate(const MyGUI::UString &_command, CommandDelegateT *_delegate)
    {
        MapDelegate::iterator iter = mDelegates.find(_command);
        if(iter != mDelegates.end())
            print("Command "+_command+" already registered");
        else
        {
            mDelegates.insert(std::make_pair(_command, _delegate));
            mComboCommand->addItem(_command);
        }
    }

    void unregisterConsoleDelegate(const MyGUI::UString &_command)
    {
        MapDelegate::iterator iter = mDelegates.find(_command);
        if(iter != mDelegates.end())
        {
            mDelegates.erase(iter);
            for(size_t i = 0; i < mComboCommand->getItemCount(); ++i)
            {
                if(mComboCommand->getItemNameAt(i) == _command)
                {
                    mComboCommand->removeItemAt(i);
                    break;
                }
            }
        }
    }

    void internalCommand(const std::string &key, const std::string &value)
    {
        if(key == "clear")
            clearConsole();
    }

    void addToConsole(const MyGUI::UString &_reason, const MyGUI::UString &_key, const MyGUI::UString &_value)
    {
        addToConsole(MyGUI::utility::toString(_reason, "'", _key, " ", _value, "'"));
    }

public:
    Console(const std::string &layout_name)
      : mWidgets(MyGUI::LayoutManager::getInstance().loadLayout(layout_name))
      , mMainWidget(nullptr)
      , mListHistory(nullptr)
      , mComboCommand(nullptr)
      , mButtonSubmit(nullptr)
    {
        mMainWidget = getWidget("_Main");
        mListHistory = getWidget<MyGUI::EditBox>("list_History");
        mComboCommand = getWidget<MyGUI::ComboBox>("combo_Command");
        mButtonSubmit = getWidget<MyGUI::Button>("button_Submit");

        mMainWidget->setRealCoord(MyGUI::FloatCoord(0.125f, 0.0f, 0.75f, 0.5f));
        mMainWidget->setVisible(false);
        mMainWidget->setEnabled(false);

        mComboCommand->eventComboAccept += newDelegate(this, &Console::notifyComboAccept);
        mComboCommand->eventKeyButtonPressed += newDelegate(this, &Console::notifyButtonPressed);
        mButtonSubmit->eventMouseButtonClick += newDelegate(this, &Console::notifyMouseButtonClick);

        auto deleg = makeDelegate(this, &Console::internalCommand);
        registerConsoleDelegate("clear", deleg);
    }

    bool getActive() const { return mMainWidget->getVisible(); }
    void setActive(bool active)
    {
        mMainWidget->setVisible(active);
        mMainWidget->setEnabled(active);
        MyGUI::PointerManager::getInstance().setVisible(active);
    }

    void print(const std::string &str)
    {
        addToConsole(str);
    }

    void addCommandCallback(const MyGUI::UString &command, CommandDelegateT *delegate)
    {
        registerConsoleDelegate(command, delegate);
    }
};


Gui Gui::sGui;

GuiIface &GuiIface::sInstance = Gui::sGui;

Gui::Gui()
  : mGui(nullptr)
  , mStatusMessages(nullptr)
  , mConsole(nullptr)
  , mActiveModes(0)
{
}

Gui::~Gui()
{
}


void Gui::initialize(osgViewer::Viewer *viewer, osg::Group *sceneroot)
{
    osg::ref_ptr<osg::Group> uiroot(new osg::Group());
    uiroot->setNodeMask(WorldIface::Mask_UI);
    sceneroot->addChild(uiroot);

    MyGUI::DataManager *dataMgr = new MyGUI_OSG::DataManager();
    MyGUI_OSG::RenderManager *renderMgr = new MyGUI_OSG::RenderManager(viewer, uiroot);
    MyGUI::LogManager *logMgr = new MyGUI::LogManager();
    try {
        switch(Log::get().getLevel())
        {
            case Log::Level_Debug:
            case Log::Level_Normal:
                logMgr->setLoggingLevel(MyGUI::LogLevel::Info);
                break;
            case Log::Level_Error:
                logMgr->setLoggingLevel(MyGUI::LogLevel::Warning);
                break;
        }
        renderMgr->initialise();

        mGui = new MyGUI::Gui();
        mGui->initialise("MyGUI_Core.xml");
    }
    catch(...) {
        delete mGui;
        mGui = nullptr;
        delete renderMgr;
        delete logMgr;
        delete dataMgr;
        throw;
    }

    MyGUI::PointerManager::getInstance().setVisible(false);
    mStatusMessages = mGui->createWidgetReal<MyGUI::TextBox>("TextBox",
        MyGUI::FloatCoord(0.f, 0.f, 1.f, .25f), MyGUI::Align::Default,
        "Overlapped"
    );
    mStatusMessages->setTextShadow(true);
    mStatusMessages->setTextColour(MyGUI::Colour::White);
    mStatusMessages->setVisible(false);

    mConsole = new Console("Console.layout");
}

void Gui::deinitialize()
{
    delete mConsole;
    mConsole = nullptr;

    if(mGui)
    {
        mGui->destroyWidget(mStatusMessages);
        mStatusMessages = nullptr;

        mGui->shutdown();
        delete mGui;
        mGui = nullptr;
    }

    delete MyGUI::RenderManager::getInstancePtr();
    delete MyGUI::LogManager::getInstancePtr();
    delete MyGUI::DataManager::getInstancePtr();
}


void Gui::printToConsole(const std::string &str)
{
    mConsole->print(str);
}

void Gui::addConsoleCallback(const char *command, CommandDelegateT *delegate)
{
    mConsole->addCommandCallback(command, delegate);
}


void Gui::pushMode(GuiIface::Mode mode)
{
    // If the mode is already set, early-out
    if((mActiveModes&mode))
        return;
    switch(mode)
    {
        case Mode_Console:
            mConsole->setActive(true);
            break;
        case Mode_Game:
            break;
    }
    mActiveModes |= mode;
}

void Gui::popMode(GuiIface::Mode mode)
{
    // If the mode is not set, early-out
    if(!(mActiveModes&mode))
        return;
    switch(mode)
    {
        case Mode_Console:
            mConsole->setActive(false);
            break;
        case Mode_Game:
            break;
    }
    mActiveModes &= ~mode;
}

GuiIface::Mode Gui::getMode() const
{
    unsigned int mode = Mode_Highest;
    while(mode && !(mActiveModes&mode))
        mode >>= 1;
    return (GuiIface::Mode)mode;
}


void Gui::updateStatus(const std::string &str)
{
    if(str.empty())
        mStatusMessages->setVisible(false);
    else
    {
        mStatusMessages->setCaption(MyGUI::UString(str));
        mStatusMessages->setVisible(true);
    }
}


void Gui::mouseMoved(int x, int y, int z)
{
    MyGUI::InputManager::getInstance().injectMouseMove(x, y, z);
}

void Gui::mousePressed(int x, int y, int button)
{
    MyGUI::MouseButton btn(MyGUI::MouseButton::None);
    if(button == SDL_BUTTON_LEFT)
        btn = MyGUI::MouseButton::Button0;
    else if(button == SDL_BUTTON_RIGHT)
        btn = MyGUI::MouseButton::Button1;
    else if(button == SDL_BUTTON_MIDDLE)
        btn = MyGUI::MouseButton::Button2;
    else if(button == SDL_BUTTON_X1)
        btn = MyGUI::MouseButton::Button3;
    else if(button == SDL_BUTTON_X2)
        btn = MyGUI::MouseButton::Button4;
    else
    {
        Log::get().stream(Log::Level_Error)<< "Unexpected SDL mouse button: "<<button;
        return;
    }
    MyGUI::InputManager::getInstance().injectMousePress(x, y, btn);
}

void Gui::mouseReleased(int x, int y, int button)
{
    MyGUI::MouseButton btn(MyGUI::MouseButton::None);
    if(button == SDL_BUTTON_LEFT)
        btn = MyGUI::MouseButton::Button0;
    else if(button == SDL_BUTTON_RIGHT)
        btn = MyGUI::MouseButton::Button1;
    else if(button == SDL_BUTTON_MIDDLE)
        btn = MyGUI::MouseButton::Button2;
    else if(button == SDL_BUTTON_X1)
        btn = MyGUI::MouseButton::Button3;
    else if(button == SDL_BUTTON_X2)
        btn = MyGUI::MouseButton::Button4;
    else
        return;
    MyGUI::InputManager::getInstance().injectMouseRelease(x, y, btn);
}


void Gui::injectKeyPress(SDL_Keycode code)
{
    auto key = SDLtoMyGUIKeycode.find(code);
    if(key != SDLtoMyGUIKeycode.end())
        MyGUI::InputManager::getInstance().injectKeyPress(key->second, 0);
    else
        Log::get().stream(Log::Level_Error)<< "Unexpected SDL keycode: "<<code;
}

void Gui::injectKeyRelease(SDL_Keycode code)
{
    auto key = SDLtoMyGUIKeycode.find(code);
    if(key != SDLtoMyGUIKeycode.end())
        MyGUI::InputManager::getInstance().injectKeyRelease(key->second);
}

void Gui::injectTextInput(const char *text)
{
    auto &inputMgr = MyGUI::InputManager::getInstance();
    std::vector<unsigned int> unicode = utf8ToUnicode(text);
    for(auto character : unicode)
        inputMgr.injectKeyPress(MyGUI::KeyCode::None, character);
}

} // namespace DF
