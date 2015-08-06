#ifndef ACTIONS_LINKER_HPP
#define ACTIONS_LINKER_HPP

#include "misc/sparsearray.hpp"


namespace DF
{

/* Linker activators are used for actions that don't directly affect the object
 * it's attached to, but still trigger linked activators.
 *
 * This class is used so linkers can be marked as active for the frame they're
 * activated on, which the current Activator code assumes.
 */
class Linker {
    static Linker sLinkers;

    Misc::SparseArray<size_t> mActive;

public:
    void update();

    static void activateFunc(size_t idx) { sLinkers.mActive[idx] = idx; }
    static void deallocateFunc(size_t idx) { sLinkers.mActive.erase(idx); }
    static Linker &get() { return sLinkers; }
};

} // namespace DF

#endif /* ACTIONS_LINKER_HPP */
