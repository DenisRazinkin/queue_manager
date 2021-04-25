cmake_minimum_required(VERSION 3.15)
project(queue_manager)

set(CMAKE_CXX_STANDARD 17)

set( MAIN_DIRECTORY     ${CMAKE_SOURCE_DIR} )
set( MAIN_LIB_DIRECTORY ${MAIN_DIRECTORY}/lib)

#include(${CMAKE_ROOT}/Modules/FetchContent.cmake)
#FetchContent_Declare( gtest
#        URL https://github.com/google/googletest/archive/release-1.10.0.tar.gz
#)

#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable(gtest)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
set( EXT_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external )
ExternalProject_Add( gtest
        GIT_REPOSITORY https://github.com/google/googletest/archive/release-1.10.0.tar.gz
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        UPDATE_COMMAND git pull https://github.com/google/googletest.git
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXT_INSTALL_LOCATION}
        INSTALL_COMMAND "")

ExternalProject_Get_Property( gtest source_dir binary_dir )

include_directories(${EXT_INSTALL_LOCATION}/include)
link_directories(${EXT_INSTALL_LOCATION}/lib)

add_subdirectory(lib)
add_subdirectory(tests)
#add_subdirectory(benchmarks)
add_subdirectory(examples)
