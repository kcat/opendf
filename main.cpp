
#include <cstdint>

#include <stdexcept>
#include <iostream>

#include "engine.hpp"


int main(int argc, char **argv)
{
    try {
        DF::Engine app;
        if(!app.parseOptions(argc, argv))
            return 0;
        app.go();
    }
    catch(std::exception &e) {
        std::cerr<< "Uncaught exception: "<<e.what() <<std::endl;
        return 1;
    }

    return 0;
}
