
#include "bsaarchive.hpp"

#include <sstream>
#include <fstream>


namespace Archives
{

void BsaArchive::loadIndexed(size_t count, std::istream &stream)
{
    std::vector<size_t> idxs; idxs.reserve(count);
    std::vector<Entry> entries; entries.reserve(count);

    std::streamsize base = stream.tellg();
    if(!stream.seekg(std::streampos(count) * -8, std::ios_base::end))
        throw std::runtime_error("Failed to seek to archive footer ("+std::to_string(count)+" entries)");
    for(size_t i = 0;i < count;++i)
    {
        idxs.push_back(read_le32(stream));
        Entry entry;
        entry.mStart = ((i == 0) ? base : entries[i-1].mEnd);
        entry.mEnd = entry.mStart + read_le32(stream);
        entries.push_back(entry);
    }
    if(!stream.good())
        throw std::runtime_error("Failed reading archive footer");

    for(size_t id : idxs)
    {
        if(!mLookupId.insert(id).second)
        {
#if 0
            std::cerr<< "Duplicate entry ID "<<std::to_string(id)<<" in "+mFilename <<std::endl;
#endif
        }
    }
    mEntries.resize(mLookupId.size());
    for(size_t i = 0;i < count;++i)
        mEntries[std::distance(mLookupId.begin(), mLookupId.find(idxs[i]))] = entries[i];
}

void BsaArchive::loadNamed(size_t count, std::istream& stream)
{
    std::vector<std::string> names; names.reserve(count);
    std::vector<Entry> entries; entries.reserve(count);

    std::streamsize base = stream.tellg();
    if(!stream.seekg(std::streampos(count) * -18, std::ios_base::end))
        throw std::runtime_error("Failed to seek to archive footer ("+std::to_string(count)+" entries)");
    for(size_t i = 0;i < count;++i)
    {
        std::array<char,12> name;
        stream.read(name.data(), name.size());
        names.push_back(std::string(name.data(), name.size()));
        int iscompressed = read_le16(stream);
        if(iscompressed != 0)
            throw std::runtime_error("Compressed entries not supported");
        Entry entry;
        entry.mStart = ((i == 0) ? base : entries[i-1].mEnd);
        entry.mEnd = entry.mStart + read_le32(stream);
        entries.push_back(entry);
    }
    if(!stream.good())
        throw std::runtime_error("Failed reading archive footer");

    for(const std::string &name : names)
    {
        if(!mLookupName.insert(name).second)
            throw std::runtime_error("Duplicate entry name \""+name+"\" in "+mFilename);
    }
    mEntries.resize(mLookupName.size());
    for(size_t i = 0;i < count;++i)
        mEntries[std::distance(mLookupName.begin(), mLookupName.find(names[i]))] = entries[i];
}

void BsaArchive::load(const std::string &fname)
{
    mFilename = fname;

    std::ifstream stream(mFilename.c_str(), std::ios::binary);
    if(!stream.is_open())
        throw std::runtime_error("Failed to open "+mFilename);

    size_t count = read_le16(stream);
    int type = read_le16(stream);

    mEntries.reserve(count);
    if(type == 0x0100)
        loadNamed(count, stream);
    else if(type == 0x0200)
        loadIndexed(count, stream);
    else
    {
        std::stringstream sstr;
        sstr<< "Unhandled BSA type: 0x"<<std::hex<<type;
        throw std::runtime_error(sstr.str());
    }
}

IStreamPtr BsaArchive::open(const Entry &entry)
{
    std::unique_ptr<std::istream> stream(new std::ifstream(mFilename.c_str(), std::ios::binary));
    if(!stream->seekg(entry.mStart))
        return IStreamPtr(nullptr);
    return IStreamPtr(new ConstrainedFileStream(std::move(stream), entry.mStart, entry.mEnd));
}

IStreamPtr BsaArchive::open(const char *name)
{
    auto iter = mLookupName.find(name);
    if(iter == mLookupName.end())
        return IStreamPtr(nullptr);
    return open(mEntries[std::distance(mLookupName.begin(), iter)]);
}

IStreamPtr BsaArchive::open(size_t id)
{
    auto iter = mLookupId.find(id);
    if(iter == mLookupId.end())
        return IStreamPtr(nullptr);
    return open(mEntries[std::distance(mLookupId.begin(), iter)]);
}

bool BsaArchive::exists(const char *name)
{
    return (mLookupName.find(name) != mLookupName.end());
}

} // namespace Archives
