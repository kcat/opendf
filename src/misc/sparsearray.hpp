#ifndef MISC_SPARSEARRAY_HPP
#define MISC_SPARSEARRAY_HPP

#include <vector>
#include <stdexcept>
#include <cstddef>


namespace Misc
{

/* A sparse array implementation, to handle non-contiguous objects in a
 * contiguous array (i.e. objects are stored contiguously in memory, but have
 * array lookup indices that contain many gaps).
 *
 * To do this, we store objects in a vector. A separate key vector is used to
 * translate externally-visible indices into vector indices, such that when
 * index 'i' is stored in the key vector at location 'n', the corresponding
 * object is in the vector at location 'n'.
 *
 * Essentially, this amounts to a map where the keys are stored separately
 * from the values. This is more efficient for lookup since the keys are packed
 * together, so there's less jumping around in memory while searching them, and
 * more efficient for iterating values since those are packed together as well.
 * However, there is a hit when iterating the keys and values simultaneously
 * since they're now separated.
 */
template<typename T>
class SparseArray {
public:
    typedef T value_type;

private:
    typedef std::vector<size_t> keylist_type;
    typedef std::vector<value_type> datalist_type;

    keylist_type mIdxLookup;
    datalist_type mData;

    keylist_type::const_iterator lookupKey(size_t key) const
    {
        if(mIdxLookup.size() > 0)
        {
            size_t low = 0;
            size_t high = mIdxLookup.size() - 1;
            while(low < high)
            {
                size_t mid = low + (high-low)/2;
                if(mIdxLookup[mid] < key)
                    low = mid + 1;
                else
                    high = mid;
            }
            if(mIdxLookup[low] == key)
                return mIdxLookup.cbegin()+low;
        }
        return mIdxLookup.cend();
    }

    std::pair<keylist_type::const_iterator,bool> insertKey(size_t key)
    {
        size_t pos = mIdxLookup.size();
        if(mIdxLookup.size() > 0)
        {
            size_t low = 0;
            size_t high = mIdxLookup.size() - 1;
            while(low < high)
            {
                size_t mid = low + (high-low)/2;
                if(mIdxLookup[mid] < key)
                    low = mid + 1;
                else
                    high = mid;
            }
            if(mIdxLookup[low] == key)
                return std::make_pair(mIdxLookup.cbegin()+low, true);
            if(mIdxLookup[low] < key)
                ++low;
            pos = low;
        }
        mIdxLookup.insert(mIdxLookup.begin()+pos, key);
        return std::make_pair(mIdxLookup.cbegin()+pos, true);
    }

public:
    typedef typename datalist_type::iterator iterator;
    typedef typename datalist_type::const_iterator const_iterator;
    typedef typename datalist_type::reverse_iterator reverse_iterator;
    typedef typename datalist_type::const_reverse_iterator const_reverse_iterator;

    void reserve(size_t size)
    {
        mIdxLookup.reserve(size);
        mData.reserve(size);
    }

    bool empty() const { return mData.empty(); }
    size_t size() const { return mData.size(); }

    void clear()
    {
        mIdxLookup.clear();
        mData.clear();
    }

    bool exists(size_t idx) const
    { return lookupKey(idx) != mIdxLookup.end(); }

    std::pair<iterator,bool> insert(size_t idx, const T &obj)
    {
        auto ins = insertKey(idx);
        if(ins.second)
        {
            idx = std::distance(mIdxLookup.cbegin(), ins.first);
            mData.insert(mData.begin()+idx, obj);
            return std::make_pair(mData.begin()+idx, true);
        }

        idx = std::distance(mIdxLookup.cbegin(), ins.first);
        return std::make_pair(mData.begin()+idx, false);
    }

    std::pair<iterator,bool> insert(size_t idx, T&& obj)
    {
        auto ins = insertKey(idx);
        if(ins.second)
        {
            idx = std::distance(mIdxLookup.cbegin(), ins.first);
            mData.insert(mData.begin()+idx, std::move(obj));
            return std::make_pair(mData.begin()+idx, true);
        }

        idx = std::distance(mIdxLookup.cbegin(), ins.first);
        return std::make_pair(mData.begin()+idx, false);
    }

    // Look up the object for the given index. If an object at the given index
    // doesn't exist, it will be allocated.
    T& operator[](size_t idx)
    {
        auto iter = lookupKey(idx);
        if(iter == mIdxLookup.cend())
        {
            iter = insertKey(idx).first;
            idx = std::distance(mIdxLookup.cbegin(), iter);
            return *mData.insert(mData.begin()+idx, T());
        }

        return mData[std::distance(mIdxLookup.cbegin(), iter)];
    }
    T& at(size_t idx)
    {
        auto iter = lookupKey(idx);
        if(iter == mIdxLookup.cend())
            throw std::out_of_range("SparseArray::at: index "+std::to_string(idx)+" does not exist");
        return mData[std::distance(mIdxLookup.cbegin(), iter)];
    }
    const T& at(size_t idx) const
    {
        auto iter = lookupKey(idx);
        if(iter == mIdxLookup.cend())
            throw std::out_of_range("SparseArray::at: index "+std::to_string(idx)+" does not exist");
        return mData[std::distance(mIdxLookup.cbegin(), iter)];
    }

    void erase(size_t idx)
    {
        auto iter = lookupKey(idx);
        if(iter == mIdxLookup.cend())
            return;

        mData.erase(std::next(mData.cbegin(), std::distance(mIdxLookup.cbegin(), iter)));
        mIdxLookup.erase(iter);
    }

    iterator erase(iterator iter)
    {
        size_t idx = std::distance(mData.begin(), iter);
        mIdxLookup.erase(mIdxLookup.begin()+idx);
        return mData.erase(iter);
    }

    iterator find(size_t idx)
    {
        auto iter = lookupKey(idx);
        if(iter == mIdxLookup.cend())
            mData.end();

        return mData.begin() + std::distance(mIdxLookup.cbegin(), iter);
    }
    const_iterator find(size_t idx) const
    {
        auto iter = lookupKey(idx);
        if(iter == mIdxLookup.cend())
            mData.cend();

        return mData.cbegin() + std::distance(mIdxLookup.cbegin(), iter);
    }

    size_t getKey(const_iterator iter) const
    {
        size_t idx = std::distance(mData.cbegin(), iter);
        return mIdxLookup[idx];
    }
    size_t getKey(const value_type *ptr) const
    {
        size_t idx = std::distance(mData.data(), ptr);
        return mIdxLookup[idx];
    }

    keylist_type::const_iterator getIdList() const { return mIdxLookup.cbegin(); }

    iterator begin() { return mData.begin(); }
    iterator end() { return mData.end(); }
    const_iterator begin() const { return mData.begin(); }
    const_iterator end() const { return mData.end(); }

    const_iterator cbegin() const { return mData.cbegin(); }
    const_iterator cend() const { return mData.cend(); }

    reverse_iterator rbegin() { return mData.rbegin(); }
    reverse_iterator rend() { return mData.rend(); }
    const_reverse_iterator rbegin() const { return mData.rbegin(); }
    const_reverse_iterator rend() const { return mData.rend(); }

    const_reverse_iterator crbegin() const { return mData.crbegin(); }
    const_reverse_iterator crend() const { return mData.crend(); }
};

} // namespace Misc

#endif /* MISC_SPARSEARRAY_HPP */
