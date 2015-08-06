
#include "linker.hpp"

#include "class/activator.hpp"


namespace DF
{

Linker Linker::sLinkers;


void Linker::update()
{
    auto iter = mActive.begin();
    while(iter != mActive.end())
        Activator::get().deactivate(*(iter++));
    mActive.clear();
}

} // namespace DF
