/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
/***********************************************************************
*
* Copyright (c) 2015-2017 Ansel Sermersheim
* All rights reserved.
*
* This file is part of libguarded
*
* libguarded is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

#ifndef STAGED_GUARDED_HPP
#define STAGED_GUARDED_HPP

#include <memory>
#include <mutex>
#include <type_traits>
#include <atomic>

namespace libguarded
{

/**
   \headerfile staged_guarded.hpp <libguarded/staged_guarded.hpp>

   This templated class wraps an object and allows only one thread at a
   time to access the protected object until it is locked as a constant after which the structure is const

   This class will use std::mutex for the internal locking mechanism by
   default. Other classes which are useful for the mutex type are
   std::recursive_mutex, std::timed_mutex, and
   std::recursive_timed_mutex.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
template <typename T, typename M = std::mutex>
class guarded
{
  private:
    class deleter;

  public:
    using handle = std::unique_ptr<T, deleter>;

    /**
     Construct a guarded object. This constructor will accept any
     number of parameters, all of which are forwarded to the
     constructor of T.
    */
    template <typename... Us>
    staged_guarded(Us &&... data);

    /**
     Acquire a handle to the protected object. As a side effect, the
     protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle lock();

    /**
     Attempt to acquire a handle to the protected object. Returns a
     null handle if the object is already locked. As a side effect,
     the protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle
     is destroyed.
    */
    handle try_lock();

    /**
     Attempt to acquire a handle to the protected object. As a side
     effect, the protected object will be locked from access by any
     other thread. The lock will be automatically released when the
     handle is destroyed.

     Returns a null handle if the object is already locked, and does
     not become available for locking before the time duration has
     elapsed.

     Calling this method requires that the underlying mutex type M
     supports the try_lock_for method.  This is not true if M is the
     default std::mutex.
    */
    template <class Duration>
    handle try_lock_for(const Duration &duration);

    /**
     Attempt to acquire a handle to the protected object.  As a side
     effect, the protected object will be locked from access by any other
     thread. The lock will be automatically released when the handle is
     destroyed.

     Returns a null handle if the object is already locked, and does not
     become available for locking before reaching the specified timepoint.

     Calling this method requires that the underlying mutex type M
     supports the try_lock_until method.  This is not true if M is the
     default std::mutex.
    */
    template <class TimePoint>
    handle try_lock_until(const TimePoint &timepoint);

    /** generate a copy of the protected object
    */
    std::enable_if_t<std::is_copy_constructible<T>::value, T> load()
    {
        if (constant)
        {
            return m_obj;
        }
        else
        {
            std::lock_guard<M> lock(m_mutex);
            return m_obj;
        }
    }
    /** generate a copy of the protected object
    */
    std::enable_if_t<std::is_copy_constructible<T>::value, T> operator*()
    {
        return load();
    }

    /** store an updated value into the object*/
    template <typename objType>
    std::enable_if_t<std::is_copy_assignable<T>::value> store(objType &&newObj)
    {
        if (!constant)
        {
            std::lock_guard<M> lock(m_mutex);
            m_obj = std::forward<objType>(newObj);
        }
        else
        {
            throw(std::exception("cannot change after constant flag set"));
        }
    }

    /** store an updated value into the object*/
    template <typename objType>
    std::enable_if_t<std::is_copy_assignable<T>::value> operator=(objType &&newObj)
    {
        store(std::forward<objType>(newObj));
    }

    void transition()
    {
        std::lock_guard<M> lock(m_mutex);
        constant = true;
    }
  private:
    class deleter
    {
      public:
        using pointer = T *;

        deleter(std::unique_lock<M> lock) : m_lock(std::move(lock))
        {
        }

        void operator()(T *ptr)
        {
            if (m_lock.owns_lock()) {
                m_lock.unlock();
            }
        }

      private:
        std::unique_lock<M> m_lock;
    };

    T m_obj;
    M m_mutex;
    std::atomic<bool> constant{ false };
};

template <typename T, typename M>
template <typename... Us>
staged_guarded<T, M>::guarded(Us &&... data) : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M>
auto staged_guarded<T, M>::lock() -> handle
{
    std::unique_lock<M> lock=(constant)? std::unique_lock<M>(m_mutex, std::defer_lock) :
        std::unique_lock<M>(m_mutex);
    return handle(&m_obj, deleter(std::move(lock)));
}

template <typename T, typename M>
auto staged_guarded<T, M>::try_lock() -> handle
{
    if (!constant)
    {
        std::unique_lock<M> lock(m_mutex, std::try_to_lock);

        if (lock.owns_lock()) {
            return handle(&m_obj, deleter(std::move(lock)));
        }
        else {
            return handle(nullptr, deleter(std::move(lock)));
        }
    }
    else
    {
        return handle(&m_obj, deleter(std::unique_lock<M>(m_mutex,std::defer_lock)));
    }
}

template <typename T, typename M>
template <typename Duration>
auto staged_guarded<T, M>::try_lock_for(const Duration &d) -> handle
{
    if (!constant)
    {
        std::unique_lock<M> lock(m_mutex, d);

        if (lock.owns_lock()) {
            return handle(&m_obj, deleter(std::move(lock)));
        }
        else {
            return handle(nullptr, deleter(std::move(lock)));
        }
    }
    else
    {
        return handle(&m_obj, deleter(std::unique_lock<M>(m_mutex, std::defer_lock)));
    }
}

template <typename T, typename M>
template <typename TimePoint>
auto staged_guarded<T, M>::try_lock_until(const TimePoint &tp) -> handle
{
    if (!constant)
    {
        std::unique_lock<M> lock(m_mutex, tp);

        if (lock.owns_lock()) {
            return handle(&m_obj, deleter(std::move(lock)));
        }
        else {
            return handle(nullptr, deleter(std::move(lock)));
        }
    }
    else
    {
        return handle(&m_obj, deleter(std::unique_lock<M>(m_mutex, std::defer_lock)));
    }
}

}

#endif /*STAGED_GUARDED_HPP*/
