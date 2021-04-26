cmake_minimum_required(VERSION 3.15)
project(queue_manager)


set( CMAKE_CXX_STANDARD 17 )
set( MAIN_DIRECTORY     ${CMAKE_SOURCE_DIR} )
set( MAIN_LIB_DIRECTORY ${MAIN_DIRECTORY}/lib)
#set(GOOGLETEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/googletest" CACHE PATH "") # Mind the quotes

include(${CMAKE_ROOT}/Modules/FetchContent.cmake)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.10.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#set(GOOGLETEST_PATH ./_deps/googletest-src)
#set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
#FetchContent_Declare(
#        googlebenchmark
#        GIT_REPOSITORY https://github.com/google/benchmark.git
#        GIT_TAG        v1.5.3
#)
# configure build of googletest
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
#set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
#FetchContent_MakeAvailable( googlebenchmark)

#ExternalProject_Get_Property(gtest_src install_dir)
#set(GTEST_INCLUDE_DIR ${install_dir}/include)
#set(GTEST_LIBRARY_PATH ${install_dir}/lib/libgtest.a)

add_subdirectory(lib)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)
