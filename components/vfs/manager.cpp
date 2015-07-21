
#include "manager.hpp"

#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#include <sys/stat.h>
#endif
#include <fnmatch.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>

#include <osgDB/Registry>

#include "components/archives/archive.hpp"
#include "components/archives/bsaarchive.hpp"

#include "osg_callbacks.hpp"


namespace
{

// FIXME: These really should be Archives...
std::vector<std::string> gRootPaths;
std::vector<std::unique_ptr<Archives::Archive>> gArchives;
// Architecture and sound archive entries are addressed by ID, so need some
// special handling.
Archives::BsaArchive gArchitecture;
Archives::BsaArchive gSound;

}


namespace VFS
{

Manager::Manager()
{
}

void Manager::initialize(std::string&& root_path)
{
    if(root_path.empty())
        root_path += "./";
    else if(root_path.back() != '/' && root_path.back() != '\\')
        root_path += "/";

    static const char names[4][16] = {
        "MAPS.BSA", "BLOCKS.BSA", "MONSTER.BSA", "MIDI.BSA"
    };
    for(size_t i = 0;i < 4;++i)
    {
        std::unique_ptr<Archives::BsaArchive> archive(new Archives::BsaArchive());
        archive->load(root_path+names[i]);
        gArchives.push_back(std::move(archive));
    }
    gArchitecture.load(root_path+"ARCH3D.BSA");
    gSound.load(root_path+"DAGGER.SND");

    gRootPaths.push_back(std::move(root_path));

    osgDB::Registry::instance()->setReadFileCallback(new OSGReadCallback());
}

void Manager::addDataPath(std::string&& path)
{
    if(path.empty())
        path += "./";
    else if(path.back() != '/' && path.back() != '\\')
        path += "/";
    gRootPaths.push_back(std::move(path));
}


IStreamPtr Manager::open(const char *name)
{
    auto iter = gArchives.rbegin();
    while(iter != gArchives.rend())
    {
        IStreamPtr stream((*iter)->open(name));
        if(stream) return stream;
        ++iter;
    }

    std::unique_ptr<std::ifstream> stream(new std::ifstream());
    auto piter = gRootPaths.rbegin();
    while(piter != gRootPaths.rend())
    {
        stream->open((*piter+name).c_str(), std::ios_base::binary);
        if(stream->good()) return IStreamPtr(std::move(stream));
        ++piter;
    }

    return IStreamPtr();
}

IStreamPtr Manager::openSoundId(size_t id)
{
    return gSound.open(id);
}

IStreamPtr Manager::openArchId(size_t id)
{
    return gArchitecture.open(id);
}

bool Manager::exists(const char *name)
{
    auto iter = gArchives.rbegin();
    while(iter != gArchives.rend())
    {
        if((*iter)->exists(name))
            return true;
        ++iter;
    }

    std::ifstream file;
    for(const std::string &path : gRootPaths)
    {
        file.open((path+name).c_str(), std::ios_base::binary);
        if(file.is_open()) return true;
    }

    return false;
}


void Manager::add_dir(const std::string &path, const std::string &pre, const char *pattern, std::set<std::string> &names)
{
    DIR *dir = opendir(path.c_str());
    if(!dir) return;

    dirent *ent;
    while((ent=readdir(dir)) != nullptr)
    {
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        if(!S_ISDIR(ent->d_type))
        {
            std::string fname = pre + ent->d_name;
            if(!pattern || fnmatch(pattern, fname.c_str(), 0) == 0)
                names.insert(fname);
        }
        else
        {
            std::string newpath = path+"/"+ent->d_name;
            std::string newpre = pre+ent->d_name+"/";
            add_dir(newpath, newpre, pattern, names);
        }
    }

    closedir(dir);
}

std::set<std::string> Manager::list(const char *pattern) const
{
    std::set<std::string> files;

    auto piter = gRootPaths.rbegin();
    while(piter != gRootPaths.rend())
    {
        add_dir(*piter+".", "", pattern, files);
        ++piter;
    }

    auto iter = gArchives.rbegin();
    while(iter != gArchives.rend())
    {
        for(const std::string &name : (*iter)->list())
        {
            if(!pattern || fnmatch(pattern, name.c_str(), 0) == 0)
                files.insert(name);
        }
        ++iter;
    }

    return files;
}

} // namespace VFS
