#ifndef COMPONENTS_SETTINGS_CONFIGFILE_HPP
#define COMPONENTS_SETTINGS_CONFIGFILE_HPP

#include <string>
#include <map>


namespace Settings
{

typedef std::pair<std::string,std::string> ConfigEntry;

typedef std::multimap<std::string,std::string> ConfigSection;

typedef std::pair<ConfigSection::const_iterator,ConfigSection::const_iterator> ConfigMultiEntryRange;

class ConfigFile {
    std::map<std::string,ConfigSection> mSections;

public:
    void load(const std::string &fname);

    const ConfigSection &getSection(const std::string &section);

    std::string getOption(const std::string &section, const std::string &option, const std::string &def);
    std::string getOption(const std::string &option, const std::string &def) { return getOption(std::string(), option, def); }

    ConfigMultiEntryRange getMultiOptionRange(const std::string &section, const std::string &option);
    ConfigMultiEntryRange getMultiOptionRange(const std::string &option) { return getMultiOptionRange(std::string(), option); }
};

} // namespace Settings

#endif /* COMPONENTS_SETTINGS_CONFIGFILE_HPP */
