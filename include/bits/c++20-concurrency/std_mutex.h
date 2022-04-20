// std::mutex implementation -*- C++ -*-

// Copyright (C) 2003-2021 Free Software Foundation, Inc.
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

#ifndef CXX20_CONCURRENCY_STD_MUTEX_H
#define CXX20_CONCURRENCY_STD_MUTEX_H 1

#include <bits/c++20-concurrency/global.h>

#include <bits/std_mutex.h>

#if __cplusplus < 201103L
# include <bits/c++0x_warning.h>
#else

#include <system_error>
#include <bits/functexcept.h>
#include <bits/gthr.h>

namespace std CXX20_CONCURRENCY_VISIBILITY_ATTRIBUTE
{
    
inline namespace CXX20_CONCURRENCY_NAMESPACE
{

#ifdef _GLIBCXX_HAS_GTHREADS
  // Implementation details for std::condition_variable
  class __condvar
  {
    using timespec = __gthread_time_t;

  public:
    __condvar() noexcept
    {
#ifndef __GTHREAD_COND_INIT
      __GTHREAD_COND_INIT_FUNCTION(&_M_cond);
#endif
    }

    ~__condvar()
    {
      int __e __attribute__((__unused__)) = __gthread_cond_destroy(&_M_cond);
      __glibcxx_assert(__e != EBUSY); // threads are still blocked
    }

    __condvar(const __condvar&) = delete;
    __condvar& operator=(const __condvar&) = delete;

    __gthread_cond_t* native_handle() noexcept { return &_M_cond; }

    // Expects: Calling thread has locked __m.
    void
    wait(mutex& __m) noexcept
    {
      int __e __attribute__((__unused__))
	= __gthread_cond_wait(&_M_cond, __m.native_handle());
      __glibcxx_assert(__e == 0);
    }

    void
    wait_until(mutex& __m, timespec& __abs_time) noexcept
    {
      __gthread_cond_timedwait(&_M_cond, __m.native_handle(), &__abs_time);
    }

#ifdef _GLIBCXX_USE_PTHREAD_COND_CLOCKWAIT
    void
    wait_until(mutex& __m, clockid_t __clock, timespec& __abs_time) noexcept
    {
      pthread_cond_clockwait(&_M_cond, __m.native_handle(), __clock,
			     &__abs_time);
    }
#endif

    void
    notify_one() noexcept
    {
      int __e __attribute__((__unused__)) = __gthread_cond_signal(&_M_cond);
      __glibcxx_assert(__e == 0);
    }

    void
    notify_all() noexcept
    {
      int __e __attribute__((__unused__)) = __gthread_cond_broadcast(&_M_cond);
      __glibcxx_assert(__e == 0);
    }

  protected:
#ifdef __GTHREAD_COND_INIT
    __gthread_cond_t _M_cond = __GTHREAD_COND_INIT;
#else
    __gthread_cond_t _M_cond;
#endif
  };

#endif // _GLIBCXX_HAS_GTHREADS

} // namespace CXX20_CONCURRENCY_NAMESPACE
} // namespace
#endif // C++11
#endif // CXX20_CONCURRENCY_STD_MUTEX_H
