cmake_minimum_required(VERSION 3.15)
project(queue_manager_examples)

set(CMAKE_CXX_STANDARD 17)
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -Wconversion -Wall -Wextra -Wno-unused-local-typedefs -pedantic -Wno-long-long -Wctor-dtor-privacy -Woverloaded-virtual -fstack-protector -Wno-deprecated-declarations -Wno-unused-function -Wno-ignored-attributes -pthread")


include_directories(
        ${CMAKE_SOURCE_DIR}
        ${MAIN_LIB_DIRECTORY}
)

add_executable(ex1 ex1.cpp)
add_executable(ex2 ex2.cpp)
#add_executable(ex3 ex3.cpp)

target_link_libraries(test1

        boost_system
        boost_thread
        boost_date_time

        )
