// <barrier> -*- C++ -*-

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

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// This implementation is based on libcxx/include/barrier
//===-- barrier.h --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------===//

#ifndef CXX20_BARRIER_H
#define CXX20_BARRIER_H 1

#include <version>

#if __cplusplus < 202002L && (!defined __GNUC__ || __cplusplus < 201709L) // GCC 10 defines __cplusplus as 201709L when using -std=c++2a
#	error c++20-barrier.h requires C++20
#endif

#if defined __GLIBCXX__ && !defined __cpp_lib_barrier

#include <bits/c++20-concurrency/global.h>

#include <bits/atomic_base.h>
#if defined CXX20_CONCURRENCY_HAVE_ATOMIC_WAIT && __cpp_aligned_new
#include <bits/c++20-concurrency/std_thread.h>
#include <bits/unique_ptr.h>
#include <bits/c++20-concurrency/atomic_wait.h>

#include <array>

namespace std CXX20_CONCURRENCY_VISIBILITY_ATTRIBUTE
{

inline namespace CXX20_CONCURRENCY_NAMESPACE
{

  struct __empty_completion
  {
    _GLIBCXX_ALWAYS_INLINE void
    operator()() noexcept
    { }
  };

/*

The default implementation of __tree_barrier is a classic tree barrier.

It looks different from literature pseudocode for two main reasons:
 1. Threads that call into std::barrier functions do not provide indices,
    so a numbering step is added before the actual barrier algorithm,
    appearing as an N+1 round to the N rounds of the tree barrier.
 2. A great deal of attention has been paid to avoid cache line thrashing
    by flattening the tree structure into cache-line sized arrays, that
    are indexed in an efficient way.

*/

  enum class __barrier_phase_t : unsigned char { };

  template<typename _CompletionF>
    class __tree_barrier
    {
      using __atomic_phase_ref_t = std::__atomic_ref<__barrier_phase_t>;
      using __atomic_phase_const_ref_t = std::__atomic_ref<const __barrier_phase_t>;
      static constexpr auto __phase_alignment =
		      __atomic_phase_ref_t::required_alignment;

      using __tickets_t = std::array<__barrier_phase_t, 64>;
      struct alignas(64) /* naturally-align the heap state */ __state_t
      {
	alignas(__phase_alignment) __tickets_t __tickets;
      };

      ptrdiff_t _M_expected;
      unique_ptr<__state_t[]> _M_state;
      __atomic_base<ptrdiff_t> _M_expected_adjustment;
      _CompletionF _M_completion;

      alignas(__phase_alignment) __barrier_phase_t  _M_phase;

      bool
      _M_arrive(__barrier_phase_t __old_phase, size_t __current)
      {
	const auto __old_phase_val = static_cast<unsigned char>(__old_phase);
	const auto __half_step =
			   static_cast<__barrier_phase_t>(__old_phase_val + 1);
	const auto __full_step =
			   static_cast<__barrier_phase_t>(__old_phase_val + 2);

	size_t __current_expected = _M_expected;
	__current %= ((_M_expected + 1) >> 1);

	for (int __round = 0; ; ++__round)
	  {
	    if (__current_expected <= 1)
		return true;
	    size_t const __end_node = ((__current_expected + 1) >> 1),
			 __last_node = __end_node - 1;
	    for ( ; ; ++__current)
	      {
		if (__current == __end_node)
		  __current = 0;
		auto __expect = __old_phase;
		__atomic_phase_ref_t __phase(_M_state[__current]
						.__tickets[__round]);
		if (__current == __last_node && (__current_expected & 1))
		  {
		    if (__phase.compare_exchange_strong(__expect, __full_step,
						        memory_order_acq_rel))
		      break;     // I'm 1 in 1, go to next __round
		  }
		else if (__phase.compare_exchange_strong(__expect, __half_step,
						         memory_order_acq_rel))
		  {
		    return false; // I'm 1 in 2, done with arrival
		  }
		else if (__expect == __half_step)
		  {
		    if (__phase.compare_exchange_strong(__expect, __full_step,
						        memory_order_acq_rel))
		      break;    // I'm 2 in 2, go to next __round
		  }
	      }
	    __current_expected = __last_node + 1;
	    __current >>= 1;
	  }
      }

    public:
      using arrival_token = __barrier_phase_t;

      static constexpr ptrdiff_t
      max() noexcept
      { return __PTRDIFF_MAX__; }

      __tree_barrier(ptrdiff_t __expected, _CompletionF __completion)
	  : _M_expected(__expected), _M_expected_adjustment(0),
	    _M_completion(move(__completion)),
	    _M_phase(static_cast<__barrier_phase_t>(0))
      {
	size_t const __count = (_M_expected + 1) >> 1;

	_M_state = std::make_unique<__state_t[]>(__count);
      }

      [[nodiscard]] arrival_token
      arrive(ptrdiff_t __update)
      {
	std::hash<std::CXX20_CONCURRENCY_DECORATE_NAME(thread)::id> __hasher;
	size_t __current = __hasher(std::CXX20_CONCURRENCY_DECORATE_NAME(this_thread)::get_id());
	__atomic_phase_ref_t __phase(_M_phase);
	const auto __old_phase = __phase.load(memory_order_relaxed);
	const auto __cur = static_cast<unsigned char>(__old_phase);
	for(; __update; --__update)
	  {
	    if(_M_arrive(__old_phase, __current))
	      {
		_M_completion();
		_M_expected += _M_expected_adjustment.load(memory_order_relaxed);
		_M_expected_adjustment.store(0, memory_order_relaxed);
		auto __new_phase = static_cast<__barrier_phase_t>(__cur + 2);
		__phase.store(__new_phase, memory_order_release);
		__atomic_notify_address(&_M_phase, true);
	      }
	  }
	return __old_phase;
      }

      void
      wait(arrival_token&& __old_phase) const
      {
	__atomic_phase_const_ref_t __phase(_M_phase);
	auto const __test_fn = [=]
	  {
	    return __phase.load(memory_order_acquire) != __old_phase;
	  };
	std::__atomic_wait_address(&_M_phase, __test_fn);
      }

      void
      arrive_and_drop()
      {
	_M_expected_adjustment.fetch_sub(1, memory_order_relaxed);
	(void)arrive(1);
      }
    };

  template<typename _CompletionF = __empty_completion>
    class barrier
    {
      // Note, we may introduce a "central" barrier algorithm at some point
      // for more space constrained targets
      using __algorithm_t = __tree_barrier<_CompletionF>;
      __algorithm_t _M_b;

    public:
      class arrival_token final
      {
      public:
	arrival_token(arrival_token&&) = default;
	arrival_token& operator=(arrival_token&&) = default;
	~arrival_token() = default;

      private:
	friend class barrier;
	using __token = typename __algorithm_t::arrival_token;
	explicit arrival_token(__token __tok) noexcept : _M_tok(__tok) { }
	__token _M_tok;
      };

      static constexpr ptrdiff_t
      max() noexcept
      { return __algorithm_t::max(); }

      explicit
      barrier(ptrdiff_t __count, _CompletionF __completion = _CompletionF())
      : _M_b(__count, std::move(__completion))
      { }

      barrier(barrier const&) = delete;
      barrier& operator=(barrier const&) = delete;

      [[nodiscard]] arrival_token
      arrive(ptrdiff_t __update = 1)
      { return arrival_token{_M_b.arrive(__update)}; }

      void
      wait(arrival_token&& __phase) const
      { _M_b.wait(std::move(__phase._M_tok)); }

      void
      arrive_and_wait()
      { wait(arrive()); }

      void
      arrive_and_drop()
      { _M_b.arrive_and_drop(); }
    };

} // namespace CXX20_CONCURRENCY_NAMESPACE
} // namespace std
#endif // CXX20_CONCURRENCY_HAVE_ATOMIC_WAIT && __cpp_aligned_new
#endif // __GLIBCXX__ && !defined __cpp_lib_barrier
#endif // CXX20_BARRIER_H
