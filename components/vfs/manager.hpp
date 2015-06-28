#ifndef COMPONENTS_VFS_MANAGER_HPP
#define COMPONENTS_VFS_MANAGER_HPP

#include <string>
#include <iostream>
#include <memory>


namespace VFS
{

typedef std::shared_ptr<std::istream> IStreamPtr;

class Manager {
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    Manager();

public:
    void initialize(std::string&& root_path);

    IStreamPtr open(const char *name);
    IStreamPtr openSoundId(size_t id);
    IStreamPtr openArchId(size_t id);

    static Manager &get()
    {
        static Manager manager;
        return manager;
    }
};

} // namespace VFS

#endif /* COMPONENTS_VFS_MANAGER_HPP */
