
#include <sys/stat.h>
#include <sys/types.h>

#include <stdexcept>
#include <fstream>
#include <cstring>
#include <array>

#include "components/archives/bsaarchive.hpp"


int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cerr<< "Usage: "<<argv[0]<<" [option]" <<std::endl
                 << "  Available options:" <<std::endl
                 << "    -e <archive.bsa>  - Extract files from BSA" <<std::endl
                 <<std::endl;
        return 1;
    }

    const char *archname = nullptr;
    for(int i = 1;i < argc;++i)
    {
        if(strcmp(argv[i], "-e") == 0)
        {
            if(argc-1 <= i)
                throw std::runtime_error("Missing archive filename");
            archname = argv[++i];
            break;
        }
        else
            throw std::runtime_error(std::string("Invalid option: ")+argv[i]);
    }

    if(!archname)
        throw std::runtime_error("No input specified");

    Archives::BsaArchive archive;
    archive.load(archname);

    const char *fname = strrchr(archname, '/');
    if(fname) archname = fname+1;
    fname = strrchr(archname, '\\');
    if(!fname) fname = archname;
    else ++fname;

    std::string dirname(fname);
    size_t pos = dirname.rfind('.');
    if(pos != std::string::npos)
        dirname[pos] = '_';
    else
        dirname += '_';

    if(mkdir(dirname.c_str(), S_IRWXU) != 0 && errno != EEXIST)
        throw std::runtime_error("Failed to create output dir "+dirname);

    std::set<std::string> files = archive.list();
    for(const std::string &name : files)
    {
        Archives::IStreamPtr instream = archive.open(name.c_str());
        if(!instream)
        {
            std::cerr<< "Failed to open "<<name<<" in archive" <<std::endl;
            continue;
        }

        std::string ofname = dirname+"/"+name;
        std::cout<< "Writing "<<ofname<<"... ";
        std::cout.flush();

        std::ofstream outstream(ofname.c_str(), std::ios_base::binary);
        if(!outstream.is_open())
        {
            std::cerr<< "Failed to open for writing" <<std::endl;
            continue;
        }

        size_t total = 0;
        while(!instream->eof())
        {
            std::array<char,4096> buf;
            instream->read(buf.data(), buf.size());
            size_t got = instream->gcount();

            if(got > 0)
            {
                if(!outstream.write(buf.data(), got))
                    break;
                total += got;
            }
        }
        if(!outstream.good())
            std::cerr<< "failed after writing "<<outstream.tellp()<<" bytes" <<std::endl;
        else
            std::cout<< "wrote "<<total<<" bytes" <<std::endl;
    }

    return 0;
}
