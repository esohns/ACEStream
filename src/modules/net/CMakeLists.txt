cmake_minimum_required (VERSION 2.8)

#include (CheckLibraryExists)
#check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
#find_library (HAVE_LIB_ACE libACE)
##add_dependencies (libCommon ACE)

# *TODO*: add dependencies
#find_package (libACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
#              PATH_SUFFIXES libACENetwork/cmake
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${libACENetwork_DIR})
# message (FATAL_ERROR "could not find libACENetwork, aborting")
#endif ()
##add_dependencies (libCommon ACE)

#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/ATCD/ACE)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
set (libACEStream_Net_LIB_SRCS
     stream_module_io.cpp
     stream_module_io.h
     stream_module_io.inl
     stream_module_io_stream.cpp
     stream_module_io_stream.h
     stream_module_io_stream.inl
     stream_module_net_common.h
     stream_module_source.cpp
     stream_module_source.h
     stream_module_source.inl
     stream_module_target.cpp
     stream_module_target.h
     stream_module_target.inl
     stream_net_exports.h
     stdafx.cpp
     stdafx.h)

#if (CMAKE_COMPILER_IS_GNUCC)
#    set_source_files_properties (${libACEStream_Net_LIB_SRCS} PROPERTIES COMPILE_FLAGS "-g -Wall")
#endif ()
set_source_files_properties (${libACEStream_Net_LIB_SRCS} PROPERTIES COMPILE_DEFINITIONS STREAM_BUILD_DLL)

add_library (${PROJECT_NAME}_Net SHARED ${libACEStream_Net_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Net Common)
add_dependencies (${PROJECT_NAME}_Net ${PROJECT_NAME})

#target_link_libraries (${PROJECT_NAME}_Net libACEStream)
#target_link_libraries (${PROJECT_NAME}_Net libCommon)
#target_link_libraries (${PROJECT_NAME}_Net ${ACE_LIBRARY})

set_target_properties (${PROJECT_NAME}_Net PROPERTIES DEFINE_SYMBOL STREAM_NET_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Net PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Net
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_module_io.h
         stream_module_io.inl
         stream_module_io_stream.h
         stream_module_io_stream.inl
         stream_module_net_common.h
         stream_module_source.h
         stream_module_source.inl
         stream_module_target.h
         stream_module_target.inl
         stream_net_exports.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Net
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)

add_subdirectory (protocols)
