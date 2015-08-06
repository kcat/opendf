#ifndef ACTIONS_UNKNOWN_HPP
#define ACTIONS_UNKNOWN_HPP

#include <array>

#include "misc/sparsearray.hpp"


namespace DF
{

class UnknownAction {
    static UnknownAction sActions;

    typedef std::pair<uint8_t,std::array<uint8_t,5>> TypeDataPair;
    Misc::SparseArray<TypeDataPair> mUnknowns;
    Misc::SparseArray<TypeDataPair> mActive;

public:
    void allocate(size_t idx, uint8_t type, const std::array<uint8_t,5> &data);
    void deallocate(size_t idx);

    void activate(size_t idx);

    void update();

    static void activateFunc(size_t idx) { sActions.activate(idx); }
    static void deallocateFunc(size_t idx) { sActions.deallocate(idx); }
    static UnknownAction &get() { return sActions; }
};

} // namespace DF

#endif /* ACTIONS_UNKNOWN_HPP */
