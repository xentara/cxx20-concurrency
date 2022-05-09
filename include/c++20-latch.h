// <latch> -*- C++ -*-

// Copyright (C) 2020-2021 Free Software Foundation, Inc.
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

/** @file include/latch
 *  This is a Standard C++ Library header.
 */

#ifndef CXX20_LATCH_H
#define CXX20_LATCH_H 1

#include <version>

#if __cplusplus < 202002L && (!defined __GNUC__ || __cplusplus < 201709L) // GCC 10 defines __cplusplus as 201709L when using -std=c++2a
#	error c++20-latch.h requires C++20
#endif

#if defined __GLIBCXX__ && !defined __cpp_lib_latch

#include <bits/c++20-concurrency/global.h>

#include <bits/atomic_base.h>
#include <bits/c++20-concurrency/atomic_wait.h>
#include <ext/numeric_traits.h>

#if defined CXX20_CONCURRENCY_HAVE_ATOMIC_WAIT
namespace std CXX20_CONCURRENCY_VISIBILITY_ATTRIBUTE
{

inline namespace CXX20_CONCURRENCY_NAMESPACE
{

  class latch
  {
  public:
    static constexpr ptrdiff_t
    max() noexcept
    { return __gnu_cxx::__int_traits<CXX20_CONCURRENCY_DECORATE_NAME(__detail)::__platform_wait_t>::__max; }

    constexpr explicit latch(ptrdiff_t __expected) noexcept
      : _M_a(__expected) { }

    ~latch() = default;
    latch(const latch&) = delete;
    latch& operator=(const latch&) = delete;

    _GLIBCXX_ALWAYS_INLINE void
    count_down(ptrdiff_t __update = 1)
    {
      auto const __old = __atomic_impl::fetch_sub(&_M_a,
				    __update, memory_order::release);
      if (__old == __update)
	  std::__atomic_notify_address(&_M_a, true);
    }

    _GLIBCXX_ALWAYS_INLINE bool
    try_wait() const noexcept
    { return __atomic_impl::load(&_M_a, memory_order::acquire) == 0; }

    _GLIBCXX_ALWAYS_INLINE void
    wait() const noexcept
    {
      auto const __pred = [this] { return this->try_wait(); };
      std::__atomic_wait_address(&_M_a, __pred);
    }

    _GLIBCXX_ALWAYS_INLINE void
    arrive_and_wait(ptrdiff_t __update = 1) noexcept
    {
      count_down(__update);
      wait();
    }

  private:
    alignas(__alignof__(CXX20_CONCURRENCY_DECORATE_NAME(__detail)::__platform_wait_t)) CXX20_CONCURRENCY_DECORATE_NAME(__detail)::__platform_wait_t _M_a;
  };
} // namespace CXX20_CONCURRENCY_NAMESPACE
} // namespace
#endif // CXX20_CONCURRENCY_HAVE_ATOMIC_WAIT
#endif // defined __GLIBCXX__ && !defined __cpp_lib_latch
#endif // CXX20_LATCH_H
