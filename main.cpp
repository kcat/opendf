
#include <cstdint>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#endif

#include <stdexcept>
#include <iostream>

#include "engine.hpp"


static void DoErrorMessage(const char *ttl, const char *msg)
{
#ifndef _WIN32
    std::cerr<<std::endl
             << "*** "<<ttl<<" ***" <<std::endl
             << msg <<std::endl;
#else
    MessageBoxA(NULL, msg, ttl, MB_OK | MB_ICONERROR | MB_TASKMODAL);
#endif
}

int main(int argc, char **argv)
{
    try {
        DF::Engine app;
        if(!app.parseOptions(argc, argv))
            return 0;
        app.go();
    }
    catch(std::exception &e) {
        DoErrorMessage("An exception has occurred!", e.what());
        return 1;
    }

    return 0;
}
