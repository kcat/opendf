
#include <cstdint>

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <set>
#include <array>


static uint32_t read_le32(std::istream &stream)
{
    char buf[4];
    if(!stream.read(buf, sizeof(buf)) || stream.gcount() != sizeof(buf))
        return 0;
    return ((uint32_t(buf[0]    )&0x000000ff) | (uint32_t(buf[1]<< 8)&0x0000ff00) |
            (uint32_t(buf[2]<<16)&0x00ff0000) | (uint32_t(buf[3]<<24)&0xff000000));
}

static uint16_t read_le16(std::istream &stream)
{
    char buf[2];
    if(!stream.read(buf, sizeof(buf)) || stream.gcount() != sizeof(buf))
        return 0;
    return ((uint16_t(buf[0]   )&0x00ff) | (uint16_t(buf[1]<<8)&0xff00));
}


class BsaArchive {
    std::set<std::string> mLookupName;
    std::set<size_t> mLookupId;

    struct Entry {
        std::streamsize mStart;
        std::streamsize mEnd;
    };
    std::vector<Entry> mEntries;

    std::string mFilename;

    void loadIndexed(size_t count, std::istream &stream);
    void loadNamed(size_t count, std::istream &stream);

public:
    void load(const std::string &fname);
};

void BsaArchive::loadIndexed(size_t count, std::istream &stream)
{
    std::vector<size_t> idxs; idxs.reserve(count);
    std::vector<Entry> entries; entries.reserve(count);

    std::streamsize base = stream.tellg();
    if(!stream.seekg(std::streampos(count) * -8, std::ios_base::end))
    {
        std::stringstream sstr;
        sstr<< "Failed to seek to archive footer ("<<count<<" entries)";
        throw std::runtime_error(sstr.str());
    }
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
            std::cerr<< "Duplicate entry ID "<<std::to_string(id)<<" in "+mFilename <<std::endl;
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
    {
        std::stringstream sstr;
        sstr<< "Failed to seek to archive footer ("<<count<<" entries)";
        throw std::runtime_error(sstr.str());
    }
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


namespace GameData
{

std::string gRootPath;
BsaArchive gMaps;
BsaArchive gBlocks;
BsaArchive gArchitecture;
BsaArchive gMonster;
BsaArchive gSound;
BsaArchive gMusic;

}


int main(int argc, char **argv)
{
    if(argc >= 2)
        GameData::gRootPath = argv[1];
    if(!GameData::gRootPath.empty() && GameData::gRootPath.back() != '/' && GameData::gRootPath.back() != '\\')
        GameData::gRootPath += "/";

    GameData::gMaps.load(GameData::gRootPath+"MAPS.BSA");
    GameData::gBlocks.load(GameData::gRootPath+"BLOCKS.BSA");
    GameData::gArchitecture.load(GameData::gRootPath+"ARCH3D.BSA");
    GameData::gMonster.load(GameData::gRootPath+"MONSTER.BSA");
    GameData::gSound.load(GameData::gRootPath+"DAGGER.SND");
    GameData::gMusic.load(GameData::gRootPath+"MIDI.BSA");

    return 0;
}
