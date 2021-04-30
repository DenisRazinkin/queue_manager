cmake_minimum_required(VERSION 3.15)
project(queue_manager_tests)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${MAIN_LIB_DIRECTORY}
)

#add_executable(test1
#        test1.cpp
#)

#target_link_libraries(test1
##        boost_system
  #      boost_thread
  #      boost_date_time
  #      pthread
  #      )

add_executable(
        unit_tests
        test_bc_queue.cpp
        test_lf_queue.cpp
        test_mpsc_mq_manager.cpp
)

target_link_libraries(unit_tests
        PRIVATE
        gtest_main
)

add_test(NAME unit_tests
        COMMAND unit_tests)

add_custom_command(TARGET unit_tests
        POST_BUILD
        COMMAND ctest -C $<CONFIGURATION> --output-on-failure)

add_subdirectory( benchmarks )



