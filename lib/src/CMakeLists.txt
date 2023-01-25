cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0094 NEW)

set(Python_FIND_REGISTRY NEVER)
set(Python_FIND_FRAMEWORK NEVER)
set(Python_FIND_STRATEGY LOCATION)

find_package(Threads REQUIRED)
find_package(Python3 ${PY_VERSION} EXACT REQUIRED COMPONENTS Development)
find_package(Boost 1.64.0 REQUIRED COMPONENTS system python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR})

set(SRC ${PROJECT_SOURCE_DIR}/lib/src)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Python3_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/ApproxMVBB/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/ApproxMVBB/include/ApproxMVBB/Config/Config.hpp)

set(LIBS )
list(APPEND LIBS ${CMAKE_THREAD_LIBS_INIT})
list(APPEND LIBS ${Boost_LIBRARIES})
list(APPEND LIBS ${Python3_LIBRARIES})
list(APPEND LIBS ApproxMVBB_s)

set(LIB )
list(APPEND LIB ${SRC}/bindings.cpp)

add_library(omvbb SHARED ${LIB})

target_compile_definitions(omvbb PRIVATE ${DEFINES})

target_link_directories(omvbb PRIVATE ${Boost_LIBRARY_DIRS} ${Python3_LIBRARY_DIRS})
target_link_libraries(omvbb ${LIBS})

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    string(REPLACE " " ";" VISUAL_STUDIO_GENERATOR ${CMAKE_GENERATOR})
    list(LENGTH VISUAL_STUDIO_GENERATOR VISUAL_STUDIO_GENERATOR_LENGTH)
    math(EXPR VISUAL_STUDIO_GENERATOR_YEAR "${VISUAL_STUDIO_GENERATOR_LENGTH}-1")
    list(GET VISUAL_STUDIO_GENERATOR ${VISUAL_STUDIO_GENERATOR_YEAR} VISUAL_STUDIO_YEAR)
    #set(ENV{LIBPATH} "C:/Program Files (x86)/Microsoft Visual Studio/${VISUAL_STUDIO_YEAR}/Community/Common7/IDE/VC/vcpackages")
    set_target_properties(omvbb PROPERTIES COMPILE_FLAGS "/wd4068 /wd4715")
endif()

set_property(TARGET omvbb PROPERTY CXX_STANDARD 14)
set_property(TARGET omvbb PROPERTY CXX_STANDARD_REQUIRED ON)

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/src/__init__.py COPYONLY)

set_target_properties(omvbb PROPERTIES PREFIX "")

install(TARGETS omvbb DESTINATION "${CMAKE_INSTALL_PREFIX}")
install(FILES __init__.py DESTINATION "${CMAKE_INSTALL_PREFIX}")

