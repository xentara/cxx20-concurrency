// <semaphore> -*- C++ -*-

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

#ifndef CXX20_SEMAPHORE_H
#define CXX20_SEMAPHORE_H 1

#include <version>

#if __cplusplus < 202002L
#   error c++20-semaphore.h requires C++20 
#endif

#if defined __GLIBCXX__ && !defined __cpp_lib_semaphore

#define CXX20_CONCURRENCY_PREVENT_RECURSION

#include <bits/c++20-concurrency/global.h>

#if defined CXX20_CONCURRENCY_HAVE_ATOMIC_WAIT || defined CXX20_CONCURRENCY_HAVE_POSIX_SEMAPHORE

#include <bits/c++20-concurrency/semaphore_base.h>

namespace std CXX20_CONCURRENCY_VISIBILITY_ATTRIBUTE
{

inline namespace CXX20_CONCURRENCY_NAMESPACE
{

  template<ptrdiff_t __least_max_value = __semaphore_impl::_S_max>
    class counting_semaphore
    {
      static_assert(__least_max_value >= 0);
      static_assert(__least_max_value <= __semaphore_impl::_S_max);

      __semaphore_impl _M_sem;

    public:
      explicit counting_semaphore(ptrdiff_t __desired) noexcept
	: _M_sem(__desired)
      { }

      ~counting_semaphore() = default;

      counting_semaphore(const counting_semaphore&) = delete;
      counting_semaphore& operator=(const counting_semaphore&) = delete;

      static constexpr ptrdiff_t
      max() noexcept
      { return __least_max_value; }

      void
      release(ptrdiff_t __update = 1) noexcept(noexcept(_M_sem._M_release(1)))
      { _M_sem._M_release(__update); }

      void
      acquire() noexcept(noexcept(_M_sem._M_acquire()))
      { _M_sem._M_acquire(); }

      bool
      try_acquire() noexcept(noexcept(_M_sem._M_try_acquire()))
      { return _M_sem._M_try_acquire(); }

      template<typename _Rep, typename _Period>
	bool
	try_acquire_for(const std::chrono::duration<_Rep, _Period>& __rtime)
	{ return _M_sem._M_try_acquire_for(__rtime); }

      template<typename _Clock, typename _Dur>
	bool
	try_acquire_until(const std::chrono::time_point<_Clock, _Dur>& __atime)
	{ return _M_sem._M_try_acquire_until(__atime); }
    };

  using binary_semaphore = std::counting_semaphore<1>;
  
} // namespace CXX20_CONCURRENCY_NAMESPACE
} // namespace std

#endif // CXX20_CONCURRENCY_HAVE_ATOMIC_WAIT || CXX20_CONCURRENCY_HAVE_POSIX_SEMAPHORE

#undef	CXX20_CONCURRENCY_PREVENT_RECURSION

#endif // defined __GLIBCXX__ && !defined __cpp_lib_semaphore
#endif // CXX20_SEMAPHORE_H