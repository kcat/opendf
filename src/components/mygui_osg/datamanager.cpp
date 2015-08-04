
#include "datamanager.h"

#include "components/vfs/manager.hpp"


namespace
{

class DataStream : public MyGUI::IDataStream {
    VFS::IStreamPtr mStream;
    std::streampos mSize;

public:
    DataStream(VFS::IStreamPtr stream) : mStream(stream), mSize(-1) { }

    virtual bool eof() final
    {
        return mStream->eof();
    }

    virtual size_t size() final
    {
        if(mSize == std::streampos(-1))
        {
            mStream->clear();
            std::streampos pos = mStream->tellg();
			if (pos != std::streampos(-1) && mStream->seekg(0, std::ios_base::end))
            {
                mSize = mStream->tellg();
                mStream->seekg(pos);
            }
        }
        return mSize;
    }

    virtual void readline(std::string &_source, MyGUI::Char _delim = '\n') final
    {
        int c;
        mStream->clear();
        std::string().swap(_source);
        while((c=mStream->get()) != std::istream::traits_type::eof() && (MyGUI::Char)c != _delim)
            _source += (char)c;
    }

    virtual size_t read(void *_buf, size_t _count) final
    {
        mStream->clear();
        mStream->read(reinterpret_cast<char*>(_buf), _count);
        return mStream->gcount();
    }
};

}

namespace MyGUI_OSG
{

MyGUI::IDataStream *DataManager::getData(const std::string &_name)
{
    VFS::IStreamPtr stream = VFS::Manager::get().open(_name.c_str());
    if(!stream) return nullptr;
    return new DataStream(stream);
}

void DataManager::freeData(MyGUI::IDataStream *_data)
{
    delete _data;
}

bool DataManager::isDataExist(const std::string &_name)
{
    return VFS::Manager::get().exists(_name.c_str());
}

const MyGUI::VectorString &DataManager::getDataListNames(const std::string &_pattern)
{
    static MyGUI::VectorString namelist;

    std::set<std::string> list = VFS::Manager::get().list(_pattern.empty() ? nullptr : _pattern.c_str());

    MyGUI::VectorString().swap(namelist);
    namelist.insert(namelist.end(), list.begin(), list.end());
    return namelist;
}

const std::string &DataManager::getDataPath(const std::string &_name)
{
    static std::string path;

    std::set<std::string> list = VFS::Manager::get().list(("*"+_name).c_str());
    if(!list.empty())
        path = *list.begin();
    else
        std::string().swap(path);
    return path;
}

} // namespace MyGUI_OSG
