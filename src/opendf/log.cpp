
#include "log.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <ctime>

#include "gui/iface.hpp"


namespace DF
{

Log Log::sLog;


Log::Log() : mLevel(Level_Normal), mFilename("opendf.log"), mGui(nullptr)
{
}

Log::~Log()
{
}


std::string Log::getTimestamp()
{
    std::time_t cur_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm *tm = std::localtime(&cur_time);

    std::stringstream sstr;
    sstr<< std::setw(2)<<std::setfill('0')<<tm->tm_hour<<
           ":"<<std::setw(2)<<std::setfill('0')<<tm->tm_min<<
           ":"<<std::setw(2)<<std::setfill('0')<<tm->tm_sec<<
           ": ";
    return sstr.str();
}


void Log::initialize()
{
    if(!mFilename.empty())
    {
        mOutfile.open(mFilename.c_str());
        if(mOutfile.is_open())
            mOutfile<< getTimestamp()<<"--- Starting log ---" <<std::endl;
        if(!mOutfile.is_open())
            std::cerr<< "Failed to open log file "<<mFilename <<std::endl;
    }
}


void Log::setGuiIface(GuiIface *iface)
{
    mGui = iface;
    if(mGui)
    {
        for(const auto &str : mBuffer)
            mGui->printToConsole(str);
        std::vector<std::string>().swap(mBuffer);
    }
}

void Log::message(const std::string &msg, Level level)
{
    if(level < mLevel)
        return;

    if(mOutfile.is_open())
    {
        mOutfile<< getTimestamp()<<msg <<std::endl;
        mOutfile.flush();
    }
    std::ostream &out = (level==Level_Error) ? std::cerr : std::cout;
    out<< msg <<std::endl;

    if(mGui)
        mGui->printToConsole(msg);
    else
        mBuffer.push_back(msg);
}

LogStream Log::stream(Level level)
{
    return LogStream(this, level);
}

} // namespace DF
