cmake_minimum_required(VERSION 3.15)
project(queue_manager)

set(CMAKE_CXX_STANDARD 11)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${MAIN_LIB_DIRECTORY} )

aux_source_directory( consumer SRC_LIST )
aux_source_directory( manager SRC_LIST )
aux_source_directory( producer  SRC_LIST )
aux_source_directory( queue SRC_LIST )
