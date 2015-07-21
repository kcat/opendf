#ifndef COMPONENTS_ARCHIVES_BSAARCHIVE_HPP
#define COMPONENTS_ARCHIVES_BSAARCHIVE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "archive.hpp"


namespace Archives
{

class BsaArchive : public Archive {
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

    IStreamPtr open(const Entry &entry);

public:
    void load(const std::string &fname);

    virtual IStreamPtr open(const char *name);
    IStreamPtr open(size_t id);

    virtual bool exists(const char *name);

    virtual const std::set<std::string> &list() const final { return mLookupName; };
};

} // namespace Archives

#endif /* COMPONENTS_ARCHIVES_BSAARCHIVE_HPP */
