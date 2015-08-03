
#include "configfile.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <locale>
#include <cctype>


namespace
{

// Trims whitespace characters from the front and back of the given string,
// within the range [front,back).
std::string trim_whitespace(const std::string &str, size_t front=0, size_t back=std::string::npos)
{
    back = std::min(back, str.length());
    size_t start = std::min(str.find_first_not_of(" \t\r", front, 3), back);
    size_t end = std::min(str.find_last_not_of(" \t\r", back, 3), back);
    if(end < back) ++end;

    return str.substr(start, end-start);
}

// Expands environment variables in the form of $VAR or ${VAR}.
std::string expand_env(std::string&& str)
{
    size_t pos = 0;
    while(pos < str.length())
    {
        size_t next = str.find('$', pos);
        if(next >= str.length()-1)
            break;
        pos = next;
        ++next;

        if(str[next] == '$')
        {
            str.erase(str.begin()+pos);
            ++pos;
        }
        else
        {
            if(str[next] == '{')
            {
                size_t end = str.find('}', next+1);
                if(end == std::string::npos)
                {
                    std::cerr<< "Unclosed expansion brace in string \""<<str<<"\"" <<std::endl;
                    ++pos;
                }
                else
                {
                    std::string env = str.substr(next+1, end-(next+1));
                    const char *res = getenv(env.c_str());
                    if(!res) env.clear();
                    else env = res;

                    str.replace(pos, end+1-pos, env);
                }
            }
            else
            {
                while(next < str.length() && std::isalnum(str[next]))
                    ++next;

                std::string env = str.substr(pos+1, next-(pos+1));
                const char *res = getenv(env.c_str());
                if(!res) env.clear();
                else env = res;

                str.replace(pos, next-pos, env);
            }
        }
    }

    return str;
}

}


namespace Settings
{

void ConfigFile::load(const std::string &fname)
{
    std::ifstream stream(fname.c_str(), std::ios_base::binary);
    if(!stream.is_open()) return;

    ConfigSection *section = nullptr;
    std::string cur_section;
    std::string line;
    size_t linenum = 0;
    while(stream)
    {
        line.clear();
        if(!std::getline(stream, line))
            break;
        ++linenum;

        line = trim_whitespace(line, 0, line.find('#'));
        if(line.empty()) continue;

        if(line.front() == '[' && line.back() == ']')
        {
            cur_section = line.substr(1, line.length()-2);
            section = &mSections[cur_section];
            continue;
        }

        size_t split = line.find('=');
        if(split == std::string::npos)
        {
            std::cerr<< "Option missing value on line "<<linenum<<" in "<<fname <<std::endl;
            continue;
        }
        if(split == 0)
        {
            std::cerr<< "Option name missing on line "<<linenum<<" in "<<fname <<std::endl;
            continue;
        }

        if(!section)
            section = &mSections[cur_section];
        section->insert(std::make_pair(trim_whitespace(line, 0, split-1),
                                       expand_env(trim_whitespace(line, split+1))));
    }
}

const ConfigSection &ConfigFile::getSection(const std::string &section)
{
    return mSections[section];
}

std::string ConfigFile::getOption(const std::string &section, const std::string &option, const std::string &def)
{
    ConfigSection &sec = mSections[section];
    ConfigMultiEntryRange range = sec.equal_range(option);
    if(range.first == range.second) return def;
    return (--range.second)->second;
}

ConfigMultiEntryRange ConfigFile::getMultiOptionRange(const std::string &section, const std::string &option)
{
    return mSections[section].equal_range(option);
}

} // namespace Settings
