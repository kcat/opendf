#ifndef COMPONENTS_VFS_OSG_CALLBACKS_HPP
#define COMPONENTS_VFS_OSG_CALLBACKS_HPP

#include <string>

#include <osgDB/Callbacks>

#include "manager.hpp"


namespace VFS
{

class OSGReadCallback : public osgDB::ReadFileCallback {
    typedef osgDB::ReaderWriter::ReadResult ReadResult;

    static VFS::IStreamPtr open(const std::string &fname, const osgDB::Options *options);

#define WRAP_READER(func)                                                           \
    virtual ReadResult func(const std::string &fname, const osgDB::Options *options);
    WRAP_READER(readObject)
    WRAP_READER(readImage)
    WRAP_READER(readHeightField)
    WRAP_READER(readNode)
    WRAP_READER(readShader)
#undef WRAP_READER

public:
    OSGReadCallback() { }
};

} // namespace VFS

#endif /* COMPONENTS_VFS_OSG_CALLBACKS_HPP */
