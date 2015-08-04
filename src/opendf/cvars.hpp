#ifndef CVARS_HPP
#define CVARS_HPP

#include <string>
#include <map>
#include <limits>

#ifdef _MSC_VER 
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

namespace DF
{

class CVar {
public:
    CVar(std::string&& name);
    virtual ~CVar() { }

    virtual bool set(const std::string &value) = 0;
    virtual std::string get() const = 0;
    virtual std::string show_range() const { return ""; }

    static void setByName(const std::string &name, const std::string &value);
    static void registerAll();
    static void writeAll(std::ostream &stream);
};

class CVarString : public CVar {
protected:
    std::string mValue;

public:
    CVarString(std::string&& name, std::string&& value);

    const std::string& operator*() const { return mValue; }
    const std::string* operator->() const { return &mValue; }

    virtual bool set(const std::string &value) final;
    virtual std::string get() const final;
};

class CVarBool : public CVar {
protected:
    bool mValue;

public:
    CVarBool(std::string&& name, bool value);

    bool operator*() const { return mValue; }

    virtual bool set(const std::string &value) final;
    virtual std::string get() const final;
};

class CVarInt : public CVar {
protected:
    const int mMinValue, mMaxValue;
    int mValue;

public:
    CVarInt(std::string&& name, int value, int minval=std::numeric_limits<int>::min(),
                                           int maxval=std::numeric_limits<int>::max());

    int operator*() const { return mValue; }

    virtual bool set(const std::string &value) final;
    virtual std::string get() const final;
    virtual std::string show_range() const final;
};


class CCmd {
public:
    CCmd(std::string&& name, std::initializer_list<const char*>&& aliases);
    virtual ~CCmd() { }

    virtual void operator()(const std::string &params) = 0;
};

} // namespace DF

#define CVAR(T, name, ...) ::DF::T name(#name, __VA_ARGS__)
#define CCMD(name, ...) namespace {                                           \
    class CCmd_##name : public ::DF::CCmd {                                   \
    public:                                                                   \
        CCmd_##name() : CCmd(#name, {__VA_ARGS__}) { }                        \
        virtual void operator()(const std::string &params) final;             \
    };                                                                        \
    CCmd_##name CCmd_##name##_cmd;                                            \
} /* namespace */                                                             \
::DF::CCmd &name = CCmd_##name##_cmd;                                         \
void CCmd_##name::operator()(const std::string &params)

#define EXTERN_CVAR(T, name) extern ::DF::T name
#define EXTERN_CCMD(name) extern ::DF::CCmd &name

#endif /* CVARS_HPP */
