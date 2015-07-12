
#include "cvars.hpp"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>

#include "gui/iface.hpp"
#include "delegates.hpp"
#include "log.hpp"


namespace
{

class CVarRegistry {
    std::map<std::string,DF::CVar*> mCVarRegistry;
    std::map<std::string,DF::CCmd*> mCCmdRegistry;

    CVarRegistry(const CVarRegistry&) = delete;
    CVarRegistry& operator=(const CVarRegistry&) = delete;

    CVarRegistry() { }

public:
    void add(std::string&& name, DF::CVar *cvar)
    {
        mCVarRegistry.insert(std::make_pair(std::move(name), cvar));
    }
    void add(std::string&& name, DF::CCmd *ccmd)
    {
        mCCmdRegistry.insert(std::make_pair(std::move(name), ccmd));
    }

    const std::map<std::string,DF::CVar*>& getAll() const
    {
        return mCVarRegistry;
    }

    void callCCmd(const std::string &name, const std::string &value)
    {
        auto ccmd = mCCmdRegistry.find(name);
        if(ccmd != mCCmdRegistry.end())
            (*ccmd->second)(value);
    }

    void setCVarValue(const std::string &name, const std::string &value)
    {
        auto iter = mCVarRegistry.find(name);
        if(iter != mCVarRegistry.end())
        {
            DF::CVar *cvar = iter->second;
            if(!value.empty())
            {
                if(!cvar->set(value))
                {
                    DF::Log::get().stream(DF::Log::Level_Error)<< "Invalid "<<name<<" value: "<<value<<cvar->show_range();
                    return;
                }
            }

            DF::Log::get().stream()<< name<<" = \""<<cvar->get()<<"\"";
        }
    }

    void loadCVarValue(const std::string &name, const std::string &value)
    {
        auto iter = mCVarRegistry.find(name);
        if(iter == mCVarRegistry.end())
            DF::Log::get().stream(DF::Log::Level_Error)<< "CVar "<<name<<" does not exist.";
        else
        {
            DF::CVar *cvar = iter->second;
            if(!cvar->set(value))
                DF::Log::get().stream(DF::Log::Level_Error)<< "Invalid "<<name<<" value: "<<value<<cvar->show_range();
        }
    }

    void registerAll()
    {
        auto deleg = DF::makeDelegate(this, &CVarRegistry::setCVarValue);
        auto &gui = DF::GuiIface::get();
        for(auto &cvar : mCVarRegistry)
            gui.addConsoleCallback(cvar.first.c_str(), deleg);
        deleg = DF::makeDelegate(this, &CVarRegistry::callCCmd);
        for(auto &ccmd : mCCmdRegistry)
            gui.addConsoleCallback(ccmd.first.c_str(), deleg);
    }

    static CVarRegistry& get()
    {
        static CVarRegistry instance;
        return instance;
    }
};

} // namespace


namespace DF
{

CVar::CVar(std::string&& name)
{
    CVarRegistry::get().add(std::move(name), this);
}

void CVar::setByName(const std::string &name, const std::string &value)
{
    CVarRegistry::get().loadCVarValue(name, value);
}

void CVar::registerAll()
{
    CVarRegistry::get().registerAll();
}

void CVar::writeAll(std::ostream &stream)
{
    const auto &cvars = CVarRegistry::get().getAll();
    for(const auto &cvar : cvars)
        stream<< cvar.first<<" = "<<cvar.second->get() <<std::endl;
}


CCmd::CCmd(std::string&& name, std::initializer_list<const char*>&& aliases)
{
    CVarRegistry::get().add(std::move(name), this);
    for(auto alias : aliases)
        CVarRegistry::get().add(alias, this);
}


CVarString::CVarString(std::string&& name, std::string&& value)
  : CVar(std::move(name)), mValue(value)
{
}

bool CVarString::set(const std::string &value)
{
    if(value.length() >= 2 && value.front() == '"' && value.back() == '"')
        mValue = value.substr(1, value.length()-2);
    else
        mValue = value;
    return true;
}

std::string CVarString::get() const
{
    return mValue;
}


CVarBool::CVarBool(std::string&& name, bool value)
  : CVar(std::move(name)), mValue(value)
{
}

bool CVarBool::set(const std::string &value)
{
    if(strcasecmp(value.c_str(), "true") == 0 || strcasecmp(value.c_str(), "yes") == 0 || strcasecmp(value.c_str(), "on") == 0 || value == "1")
    {
        mValue = true;
        return true;
    }
    if(strcasecmp(value.c_str(), "false") == 0 || strcasecmp(value.c_str(), "no") == 0 || strcasecmp(value.c_str(), "off") == 0 || value == "0")
    {
        mValue = false;
        return true;
    }
    return false;
}

std::string CVarBool::get() const
{
    return mValue ? "true" : "false";
}


CVarInt::CVarInt(std::string&& name, int value, int minval, int maxval)
  : CVar(std::move(name)), mMinValue(minval), mMaxValue(maxval), mValue(value)
{
}

bool CVarInt::set(const std::string &value)
{
    try {
        size_t end;
        int val = std::stoi(value, &end, 0);
        if(end < value.length()) return false;
        if(val >= mMinValue && val <= mMaxValue)
        {
            mValue = val;
            return true;
        }
    }
    catch(std::invalid_argument&) {
    }
    catch(std::out_of_range&) {
    }
    return false;
}

std::string CVarInt::get() const
{
    return std::to_string(mValue);
}

std::string CVarInt::show_range() const
{
    return " (min: "+std::to_string(mMinValue)+", max: "+std::to_string(mMaxValue)+")";
}

} // namespace DF
