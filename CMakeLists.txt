cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

set(LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FindPkgConfig)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)

project("OMVBB")

macro(set_option var default type docstring)
	if(NOT DEFINED ${var})
		set(${var} ${default})
	endif()
	set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

set_option(CMAKE_BUILD_TYPE Release STRING "Choose the type of build (Debug or Release)")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/ApproxMVBB/cmake)
set(ApproxMVBB_BUILD_LIBRARY_STATIC ON)

if ("${TARGET_PLATFORM}" STREQUAL "Linux")
    add_definitions("-fPIC")
endif()

add_subdirectory(ApproxMVBB)
add_subdirectory(lib)

