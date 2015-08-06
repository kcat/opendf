#ifndef CLASS_ACTIVATOR_HPP
#define CLASS_ACTIVATOR_HPP

#include <cstdint>

#include "misc/sparsearray.hpp"


namespace DF
{

typedef void (*ActivatorCallback)(size_t idx);
struct ActivatorData {
    ActivatorCallback mCallback;
    size_t mLink;
};
enum ActionFlags {
    // Specifies if the object can be directly activated (otherwise only via an action link)
    ActionFlag_Activatable = 0x02,
};

class Activator {
    static Activator sActivators;

    Misc::SparseArray<ActivatorCallback> mDeallocators;
    Misc::SparseArray<uint32_t> mFlags;
    Misc::SparseArray<ActivatorData> mInactive;
    Misc::SparseArray<ActivatorData> mActive;

public:
    void allocate(size_t idx, uint32_t flags, size_t link, ActivatorCallback callback, ActivatorCallback dealloc);
    void deallocate(size_t idx);

    void activate(size_t idx);
    void deactivate(size_t idx);

    static Activator &get() { return sActivators; }
};

} // namespace DF

#endif /* CLASS_ACTIVATOR_HPP */
