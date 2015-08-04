#ifndef DELEGATES_HPP
#define DELEGATES_HPP

#include <typeinfo>
#include <list>

#include "referenceable.hpp"


namespace DF
{

class IDelegateUnlink {
    IDelegateUnlink *mBaseDelegateUnlink;

public:
    IDelegateUnlink() : mBaseDelegateUnlink(this) { }
    virtual ~IDelegateUnlink() { }

    bool compare(IDelegateUnlink *unlink) const
    {
        return mBaseDelegateUnlink == unlink->mBaseDelegateUnlink;
    }
};
inline IDelegateUnlink *GetDelegateUnlink(void*)
{
    return nullptr;
}
inline IDelegateUnlink *GetDelegateUnlink(IDelegateUnlink *base)
{
    return base;
}


template<typename ...Args>
class IDelegate : public Referenceable
{
public:
    virtual ~IDelegate() { }
    virtual bool isType(const std::type_info& _type) = 0;
    virtual void invoke(Args...) = 0;
    virtual bool compare(IDelegate<Args...> *_delegate) const = 0;
    virtual bool compare(IDelegateUnlink *_unlink) const
    { return false; }
};

template<typename ...Args>
class CStaticDelegate : public IDelegate<Args...>
{
    typedef void (*Func)(Args...);

    Func mFunc;

public:
    CStaticDelegate(Func _func=nullptr) : mFunc(_func) { }

    virtual bool isType(const std::type_info& _type)
    {
        return typeid(CStaticDelegate<Args...>) == _type;
    }

    virtual void invoke(Args...args)
    {
        mFunc(args...);
    }

    virtual bool compare(IDelegate<Args...> *_delegate) const
    {
        if(nullptr == _delegate || !_delegate->isType(typeid(CStaticDelegate<Args...>)))
            return false;
        CStaticDelegate<Args...> *cast = static_cast<CStaticDelegate<Args...>*>(_delegate);
        return cast->mFunc == mFunc;
    }
    virtual bool compare(IDelegateUnlink *_unlink) const
    {
        return false;
    }
};

template<typename T, typename ...Args>
class CMethodDelegate : public IDelegate<Args...>
{
    typedef void (T::*Method)(Args...);

    IDelegateUnlink *mUnlink;
    T *mObject;
    Method mMethod;

public:
    CMethodDelegate(IDelegateUnlink *_unlink, T *_object, Method _method)
      : mUnlink(_unlink), mObject(_object), mMethod(_method)
    { }

    virtual bool isType(const std::type_info &_type)
    {
        return typeid(CMethodDelegate<T,Args...>) == _type;
    }

    virtual void invoke(Args...args)
    {
        (mObject->*mMethod)(args...);
    }

    virtual bool compare(IDelegate<Args...> *_delegate) const
    {
        if (nullptr == _delegate || !_delegate->isType(typeid(CMethodDelegate<T,Args...>)))
            return false;
        CMethodDelegate<T,Args...> *cast = static_cast<CMethodDelegate<T,Args...>*>(_delegate);
        return cast->mObject == mObject && cast->mMethod == mMethod;
    }

    virtual bool compare(IDelegateUnlink *_unlink) const
    {
        return mUnlink == _unlink;
    }
};

template<typename ...Args>
class CDelegate
{
    typedef IDelegate<Args...> DelegateT;

    ref_ptr<DelegateT> mDelegate;

public:
    CDelegate(DelegateT *_delegate=nullptr) : mDelegate(_delegate) { }
    CDelegate(const CDelegate<Args...> &_event) : mDelegate(_event) { }
    CDelegate(CDelegate<Args...> &&_event) : mDelegate(std::move(_event)) { }
    ~CDelegate()
    {
        clear();
    }

    bool empty() const
    {
        return !mDelegate.valid();
    }

    void clear()
    {
        mDelegate = nullptr;
    }

    CDelegate<Args...>& operator=(const CDelegate<Args...> &_event)
    {
        mDelegate = _event.mDelegate;
        return *this;
    }
    CDelegate<Args...>& operator=(CDelegate<Args...>&& _event)
    {
        // take ownership
        mDelegate = std::move(_event.mDelegate);
        return *this;
    }

    void operator()(Args...args)
    {
        if(!mDelegate.valid())
            return;
        mDelegate->invoke(args...);
    }
};

template<typename ...Args>
class CMultiDelegate
{
    typedef IDelegate<Args...> DelegateT;
    typedef typename std::list<ref_ptr<DelegateT>> ListDelegate;

    ListDelegate mListDelegates;

public:
    CMultiDelegate() { }
    CMultiDelegate(const CMultiDelegate<Args...> &_event) : mListDelegates(_event) { }
    CMultiDelegate(CMultiDelegate<Args...>&& _event) : mListDelegates(std::move(_event)) { }

    ~CMultiDelegate()
    {
        clear();
    }

    bool empty() const
    {
        for(const auto &_deleg : mListDelegates)
        {
            if(_deleg.valid())
                return false;
        }
        return true;
    }

    void clear()
    {
        mListDelegates.clear();
    }

    void clear(IDelegateUnlink *_unlink)
    {
        for(auto &_deleg : mListDelegates)
        {
            if(_deleg.valid() && _deleg->compare(_unlink))
                _deleg = nullptr;
        }
    }

    CMultiDelegate<Args...>& operator+=(DelegateT *_delegate)
    {
        for(auto _deleg : mListDelegates)
        {
            if(_deleg && _deleg->compare(_delegate))
                throw std::runtime_error("Trying to add same delegate twice.");
        }
        mListDelegates.push_back(_delegate);
        return *this;
    }

    CMultiDelegate<Args...>& operator-=(DelegateT *_delegate)
    {
        for(auto &_deleg : mListDelegates)
        {
            if(_deleg && _deleg->compare(_delegate))
            {
                // проверяем на идентичность делегатов
                _deleg = nullptr;
                break;
            }
        }
        return *this;
    }

    void operator()(Args...args)
    {
        auto iter = mListDelegates.begin();
        while(iter != mListDelegates.end())
        {
            if(nullptr == *iter)
                iter = mListDelegates.erase(iter);
            else
            {
                (*iter)->invoke(args...);
                ++iter;
            }
        }
    }

    CMultiDelegate<Args...>& operator=(const CMultiDelegate<Args...> &_event)
    {
        mListDelegates = _event.mListDelegates;
        return *this;
    }
    CMultiDelegate<Args...>& operator=(CMultiDelegate<Args...>&& _event)
    {
        // take ownership
        mListDelegates = std::move(_event.mListDelegates);
        return *this;
    }
};


template<typename ...Args>
inline ref_ptr<IDelegate<Args...>> makeDelegate(void (*func)(Args...))
{
    return new CStaticDelegate<Args...>(func);
}

template<typename T, typename ...Args>
inline ref_ptr<IDelegate<Args...>> makeDelegate(T *obj, void (T::*Func)(Args...))
{
    return new CMethodDelegate<T,Args...>(GetDelegateUnlink(obj), obj, Func);
}

} // namespace DF

#endif /* DELEGATES_HPP */
