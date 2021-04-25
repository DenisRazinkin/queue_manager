cmake_minimum_required(VERSION 3.15)
project(queue_manager_tests)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${MAIN_LIB_DIRECTORY}
)

add_executable(test1
        test1.cpp
)

target_link_libraries(test1

        boost_system
        boost_thread
        boost_date_time
        pthread
        )
