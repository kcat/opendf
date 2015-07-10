
#include "osg_callbacks.hpp"

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>


namespace VFS
{

VFS::IStreamPtr OSGReadCallback::open(const std::string &fname, const osgDB::Options *options)
{
    VFS::IStreamPtr istream;
    // try to find the proper path in vfs
    if((istream=VFS::Manager::get().open(fname.c_str())))
        return istream;
    if(options)
    {
        const osgDB::FilePathList &pl = options->getDatabasePathList();
        for(const auto &path : pl)
        {
            std::string searchpath = path + "/" + fname;
            if((istream=VFS::Manager::get().open(searchpath.c_str())))
                return istream;
        }
    }
    const osgDB::FilePathList &pl = osgDB::Registry::instance()->getDataFilePathList();
    for(const auto &path : pl)
    {
        std::string searchpath = path + "/" + fname;
        if((istream=VFS::Manager::get().open(searchpath.c_str())))
            return istream;
    }
    return istream;
}

#define WRAP_READER(func)                                                      \
OSGReadCallback::ReadResult OSGReadCallback::func(const std::string &fname, const osgDB::Options *options)\
{                                                                              \
    VFS::IStreamPtr istream = open(fname, options);                            \
    if(!istream) return ReadResult::FILE_NOT_FOUND;                            \
                                                                               \
    const osgDB::ReaderWriter *rw = osgDB::Registry::instance()->getReaderWriterForExtension(osgDB::getFileExtension(fname)); \
    if(rw) return rw->func(*istream, options);                                 \
    return ReadResult::ERROR_IN_READING_FILE;                                  \
}
WRAP_READER(readObject)
WRAP_READER(readImage)
WRAP_READER(readHeightField)
WRAP_READER(readNode)
WRAP_READER(readShader)
#undef WRAP_READER

} // namespace VFS
