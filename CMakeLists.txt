# Copyright (C) embedded ocean GmbH, Munich, Germany.
#
# This file is part of the Charconv Float Library.  This library is free
# software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Under Section 7 of GPL version 3, you are granted additional
# permissions described in the GCC Runtime Library Exception, version
# 3.1, as published by the Free Software Foundation.
#
# You should have received a copy of the GNU General Public License and
# a copy of the GCC Runtime Library Exception along with this program;
# see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
# <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.19)

project(c++20-concurrency 
	VERSION 1.0.0
	DESCRIPTION "substitutes for C++20 std::counting_semaphore, std::binary_semaphore, std::barrier, and std::latch in libstdc++ 10 and earlier"
	HOMEPAGE_URL "https://gitlab.com/embedded-ocean/oss/cpp20-concrurrency.git"
	LANGUAGES CXX)

###########################################################################
# Options and settings

set(CXX20_CONCURRENCY_TARGET_TYPE SHARED CACHE STRING "the cmake target type for the library to build")
set(CXX20_CONCURRENCY_NAMESPACE "__cxx20_concurrency" CACHE STRING "the inline namespace used for the types")
set(CXX20_CONCURRENCY_VISIBILITY "default" CACHE STRING "the ELF visibility used for the namespace")

set_property(CACHE CXX20_CONCURRENCY_TARGET_TYPE PROPERTY STRINGS SHARED STATIC OBJECT)

###########################################################################
# Check system features

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

try_compile(HAVE_GTHREADS
	${CMAKE_CURRENT_BINARY_DIR}/Checks/check_gthreads
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_gthreads.cc
)

try_compile(HAVE_POSIX_SEMAPHORE
	${CMAKE_CURRENT_BINARY_DIR}/Checks/check_posix_semaphore
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_posix_semaphore.cc
)

try_compile(HAVE_PTHREAD_COND_CLOCKWAIT
	${CMAKE_CURRENT_BINARY_DIR}/Checks/check_pthread_cond_clockwait
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_pthread_cond_clockwait.cc
)

try_compile(HAVE_LINUX_FUTEX
	${CMAKE_CURRENT_BINARY_DIR}/Checks/check_linux_futex
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_linux_futex.cc
)

if(HAVE_LINUX_FUTEX)
	try_compile(HAVE_LINUX_FUTEX_PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/Checks/check_linux_futex_private
		${CMAKE_CURRENT_SOURCE_DIR}/config/check_linux_futex_private.cc
	)
endif()

try_compile(HAVE_COMPATIBLE_GLIBCXX
	${CMAKE_CURRENT_BINARY_DIR}/Checks/check_libstdcxx_version
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_libstdcxx_version.cc
)

try_compile(HAVE_INCLUDE_NEXT
	${CMAKE_CURRENT_BINARY_DIR}/Checks/check_include_next
	${CMAKE_CURRENT_SOURCE_DIR}/config/check_include_next.cc
)

if(HAVE_INCLUDE_NEXT AND HAVE_COMPATIBLE_GLIBCXX)
	set(PATCH_STD_HEADERS TRUE)
endif()

###########################################################################
# generate the config include

if(HAVE_GTHREADS)
	set(CXX20_CONCURRENCY_HAVE_GTHREADS TRUE)
endif()

if(HAVE_POSIX_SEMAPHORE)
	set(CXX20_CONCURRENCY_HAVE_POSIX_SEMAPHORE TRUE)
endif()

if(HAVE_PTHREAD_COND_CLOCKWAIT)
	set(CXX20_CONCURRENCY_HAVE_PTHREAD_COND_CLOCKWAIT TRUE)
endif()

if(HAVE_LINUX_FUTEX)
	set(CXX20_CONCURRENCY_HAVE_LINUX_FUTEX TRUE)
endif()

if(HAVE_LINUX_FUTEX_PRIVATE)
	set(CXX20_CONCURRENCY_HAVE_LINUX_FUTEX_PRIVATE TRUE)
endif()

set(CONFIG_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/include/bits/c++20-concurrency/config.h")
configure_file(config/config.h.in "${CONFIG_HEADER_FILE}" @ONLY)

###########################################################################
# define the library

add_library(
	${PROJECT_NAME} ${CXX20_CONCURRENCY_TARGET_TYPE}

	src/dummy.cc
)

target_include_directories(
	${PROJECT_NAME}
	
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
		"$<INSTALL_INTERFACE:include/c++20-concurrency>"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if(PATCH_STD_HEADERS)

target_include_directories(
	${PROJECT_NAME}
	
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/std>"
)

endif(PATCH_STD_HEADERS)

set_target_properties(
	${PROJECT_NAME}
	
	PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON

		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
)

###########################################################################
# test applications

add_executable(
	semaphore-test
	
	"test/semaphore-test.cc"
)

target_link_libraries(semaphore-test PRIVATE ${PROJECT_NAME})

add_executable(
	barrier-test
	
	"test/barrier-test.cc"
)

target_link_libraries(barrier-test PRIVATE ${PROJECT_NAME})

###########################################################################
# export and install the project

# Skip this entire section if no installation is desired
if(NOT CMAKE_SKIP_INSTALL_RULES)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/c++20-concurrency")
set(INSTALL_MODULEDIR "${CMAKE_INSTALL_DATADIR}/cmake/Cxx20Concurrency")

set(MODULE_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/module/Cxx20ConcurrencyConfig.cmake")
set(MODULE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/module/Cxx20ConcurrencyVersion.cmake")

configure_package_config_file(
	config/Cxx20ConcurrencyConfig.cmake.in
	${MODULE_CONFIG_FILE}

	INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/Cxx20Concurrency"
)

write_basic_package_version_file(
	${MODULE_VERSION_FILE}

	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets RUNTIME ARCHIVE)
install(EXPORT ${PROJECT_NAME}-targets NAMESPACE Cxx20Concurrency:: FILE Cxx20ConcurrencyTargets.cmake DESTINATION "${INSTALL_MODULEDIR}")

install(
	FILES
		include/bits/c++20-concurrency/atomic_timed_wait.h
		include/bits/c++20-concurrency/atomic_wait.h
		include/bits/c++20-concurrency/global.h
		include/bits/c++20-concurrency/semaphore_base.h
		include/c++20-barrier.h
		include/c++20-concurrency.h
		include/c++20-semaphore.h
		"${CONFIG_HEADER_FILE}"
	
	DESTINATION "${INSTALL_INCLUDEDIR}"
)

install(
	FILES
		include/std/barrier
		include/std/semaphore
		
	DESTINATION "${INSTALL_INCLUDEDIR}/std"
)

install(
	FILES
		"${MODULE_CONFIG_FILE}"
		"${MODULE_VERSION_FILE}"
		config/PatchLibstdcxx.cmake
		config/check_include_next.cc
		config/check_libstdcxx_version.cc
	
	DESTINATION "${INSTALL_MODULEDIR}"
)

endif(NOT CMAKE_SKIP_INSTALL_RULES)
