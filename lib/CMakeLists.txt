cmake_minimum_required(VERSION 3.15)
project(queue_manager)

set(CMAKE_CXX_STANDARD 17)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${MAIN_LIB_DIRECTORY} )

aux_source_directory( consumer SRC_LIST )
aux_source_directory( manager SRC_LIST )
aux_source_directory( producer  SRC_LIST )
aux_source_directory( queue SRC_LIST )

#add_library(${PROJECT_NAME} SHARED
#        ${SRC_LIST}
#        common.h )

#target_link_libraries( ${PROJECT_NAME}
#        boost_system
#        boost_thread
#        boost_date_time
#        )
