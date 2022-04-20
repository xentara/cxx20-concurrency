// std::thread declarations -*- C++ -*-

// Copyright (C) 2008-2021 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file bits/std_thread.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{thread}
 */

#ifndef CXX20_CONCURRENCY_THREAD_H
#define CXX20_CONCURRENCY_THREAD_H 1

#if __cplusplus >= 201103L
#include <bits/c++config.h>
#include <bits/c++20-concurrency/global.h>

#include <exception>		// std::terminate
#include <iosfwd>		// std::basic_ostream
#include <tuple>		// std::tuple
#include <bits/functional_hash.h> // std::hash
#include <bits/invoke.h>	// std::__invoke
#include <bits/refwrap.h>       // not required, but helpful to users
#include <bits/unique_ptr.h>	// std::unique_ptr

#ifdef _GLIBCXX_HAS_GTHREADS
# include <bits/gthr.h>
#else
# include <errno.h>
# include <bits/functexcept.h>
#endif

namespace std CXX20_CONCURRENCY_VISIBILITY_ATTRIBUTE
{
    
inline namespace CXX20_CONCURRENCY_NAMESPACE
{

  /// thread
  class CXX20_CONCURRENCY_DECORATE_NAME(thread)
  {
  public:
#ifdef _GLIBCXX_HAS_GTHREADS
    // Abstract base class for types that wrap arbitrary functors to be
    // invoked in the new thread of execution.
    struct _State
    {
      virtual ~_State();
      virtual void _M_run() = 0;
    };
    using _State_ptr = unique_ptr<_State>;

    using native_handle_type = __gthread_t;
#else
    using native_handle_type = int;
#endif

    /// thread::id
    class id
    {
      native_handle_type	_M_thread;

    public:
      id() noexcept : _M_thread() { }

      explicit
      id(native_handle_type __id) : _M_thread(__id) { }

    private:
      friend class thread;
      friend struct hash<id>;

      friend bool
      operator==(id __x, id __y) noexcept;

#if __cpp_lib_three_way_comparison
      friend strong_ordering
      operator<=>(id __x, id __y) noexcept;
#else
      friend bool
      operator<(id __x, id __y) noexcept;
#endif
    };
  };

  inline bool
  operator==(CXX20_CONCURRENCY_DECORATE_NAME(thread)::id __x, CXX20_CONCURRENCY_DECORATE_NAME(thread)::id __y) noexcept
  {
    // pthread_equal is undefined if either thread ID is not valid, so we
    // can't safely use __gthread_equal on default-constructed values (nor
    // the non-zero value returned by this_thread::get_id() for
    // single-threaded programs using GNU libc). Assume EqualityComparable.
    return __x._M_thread == __y._M_thread;
  }

} // namespace CXX20_CONCURRENCY_NAMESPACE

  // N.B. other comparison operators are defined in <thread>

  // DR 889.
  /// std::hash specialization for thread::id.
  template<>
    struct hash<CXX20_CONCURRENCY_DECORATE_NAME(thread)::id>
    : public __hash_base<size_t, CXX20_CONCURRENCY_DECORATE_NAME(thread)::id>
    {
      size_t
      operator()(const CXX20_CONCURRENCY_DECORATE_NAME(thread)::id& __id) const noexcept
      { return std::_Hash_impl::hash(__id._M_thread); }
    };

inline namespace CXX20_CONCURRENCY_NAMESPACE
{

  namespace CXX20_CONCURRENCY_DECORATE_NAME(this_thread)
  {
    /// this_thread::get_id
    inline CXX20_CONCURRENCY_DECORATE_NAME(thread)::id
    get_id() noexcept
    {
#ifndef _GLIBCXX_HAS_GTHREADS
      return CXX20_CONCURRENCY_DECORATE_NAME(thread)::id(1);
#elif defined _GLIBCXX_NATIVE_THREAD_ID
      return CXX20_CONCURRENCY_DECORATE_NAME(thread)::id(_GLIBCXX_NATIVE_THREAD_ID);
#else
      return CXX20_CONCURRENCY_DECORATE_NAME(thread)::id(__gthread_self());
#endif
    }

    /// this_thread::yield
    inline void
    yield() noexcept
    {
#if defined _GLIBCXX_HAS_GTHREADS && defined _GLIBCXX_USE_SCHED_YIELD
      __gthread_yield();
#endif
    }

  } // namespace this_thread

  /// @}

} // namespace CXX20_CONCURRENCY_NAMESPACE
} // namespace
#endif // C++11

#endif // CXX20_CONCURRENCY_THREAD_H
