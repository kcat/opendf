#ifndef REFERENCEALBLE_HPP
#define REFERENCEALBLE_HPP

#include <atomic>
#include <map>


namespace DF
{

class Referenceable {
    std::atomic<unsigned int> mRefCount;

protected:
    virtual ~Referenceable() { }

public:
    Referenceable() : mRefCount(0) { }

    unsigned int ref() { return ++mRefCount; }
    unsigned int unref()
    {
        unsigned int ref = --mRefCount;
        if(ref == 0)
            delete this;
        return ref;
    }
    unsigned int unref_nodelete() { return --mRefCount; }
};

// An intrusive pointer. The object type being held is expected to implement
// reference counting methods (see the Referenceable class above).
template<class T>
class ref_ptr
{
    T *mPtr;

    template<class Other>
    void assign(const ref_ptr<Other> &other)
    {
        if(mPtr == other.mPtr)
            return;

        T *tmp_ptr = mPtr;
        mPtr = other.mPtr;
        // unref second to prevent any deletion of any object which might
        // be referenced by the other object. i.e other is child of the
        // original _ptr.
        if(mPtr) mPtr->ref();
        if(tmp_ptr) tmp_ptr->unref();
    }
    template<class Other>
    friend class ref_ptr;

public:
    typedef T element_type;

    ref_ptr() : mPtr(nullptr) {}
    ref_ptr(T *ptr) : mPtr(ptr) { if(mPtr) mPtr->ref(); }
    ref_ptr(const ref_ptr &rhs) : mPtr(rhs.mPtr) { if(mPtr) mPtr->ref(); }
    ref_ptr(ref_ptr&& rhs) : mPtr(rhs.mPtr) { if(mPtr) mPtr->ref(); }
    template<class Other>
    ref_ptr(const ref_ptr<Other> &rhs) : mPtr(rhs.mPtr) { if(mPtr) mPtr->ref(); }
    template<class Other>
    ref_ptr(ref_ptr<Other>&& rhs) : mPtr(rhs.mPtr) { if(mPtr) mPtr->ref(); }
    ~ref_ptr()
    {
        if(mPtr)
            mPtr->unref();
        mPtr = nullptr;
    }

    ref_ptr& operator=(const ref_ptr &rhs)
    { assign(rhs); return *this; }
    ref_ptr& operator=(ref_ptr&& rhs)
    { assign(rhs); return *this; }

    template<class Other>
    ref_ptr& operator=(const ref_ptr<Other> &rhs)
    { assign(rhs); return *this; }
    template<class Other>
    ref_ptr& operator=(ref_ptr<Other>&& rhs)
    { assign(rhs); return *this; }

    ref_ptr& operator=(T *ptr)
    {
        if(mPtr == ptr)
            return *this;

        T *tmp_ptr = mPtr;
        mPtr = ptr;
        if(mPtr) mPtr->ref();
        if(tmp_ptr) tmp_ptr->unref();
        return *this;
    }

    // implicit output conversion
    operator T*() const { return mPtr; }

    T& operator*() const { return *mPtr; }
    T* operator->() const { return mPtr; }
    T* get() const { return mPtr; }

    bool operator!() const { return mPtr==nullptr; }
    bool valid() const     { return mPtr!=nullptr; }

    T* release()
    {
        T *tmp = mPtr;
        if(mPtr)
            mPtr->unref_nodelete();
        mPtr = nullptr;
        return tmp;
    }

    void swap(ref_ptr &rhs)
    {
        std::swap(mPtr, rhs.mPtr);
    }
};

} // namespace DF

#endif /* REFERENCEALBLE_HPP */
