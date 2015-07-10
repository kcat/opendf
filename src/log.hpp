#ifndef LOG_HPP
#define LOG_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <vector>


namespace DF
{

class GuiIface;

class LogStream;

class Log {
public:
    enum Level {
        Level_Debug,
        Level_Normal,
        Level_Error,
    };

private:
    static Log sLog;

    Level mLevel;
    std::string mFilename;
    GuiIface *mGui;
    std::vector<std::string> mBuffer;
    std::ofstream mOutfile;

    static std::string getTimestamp();

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    Log();
    ~Log();

public:
    void initialize();

    void setLog(std::string&& name) { mFilename = std::move(name); }
    void setLevel(Level level) { mLevel = level; }
    Level getLevel() const { return mLevel; }

    void setGuiIface(GuiIface *iface);

    LogStream stream(Level level=Level_Normal);
    void message(const std::string &msg, Level level=Level_Normal);

    static Log &get() { return sLog; }
};

class LogStream {
    Log *mLog;
    Log::Level mLevel;
    std::stringstream mStream;

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

public:
    LogStream(Log *log, Log::Level level) : mLog(log), mLevel(level) { }
    ~LogStream() { if(mLog) mLog->message(mStream.str(), mLevel); }

    LogStream(LogStream&& rhs) : mLog(std::move(rhs.mLog)), mLevel(std::move(rhs.mLevel)),
                                 mStream(rhs.mStream.str())
    {
        mStream.seekp(0, std::ios::end);
    }

    template<typename T>
    LogStream& operator<<(const T& val)
    {
        mStream << val;
        return *this;
    }
};

} // namespace DF

#endif /* LOG_HPP */
