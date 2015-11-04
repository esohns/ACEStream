cmake_minimum_required (VERSION 2.8)

#include (CheckLibraryExists)
#check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
#find_library (HAVE_LIB_ACE libACE)
##add_dependencies (libCommon ACE)

#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/ATCD/ACE)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
set (${PROJECT_NAME}_File_LIB_SRCS
     stream_module_filereader.cpp
     stream_module_filereader.h
     stream_module_filereader.inl
     stream_module_filewriter.cpp
     stream_module_filewriter.h
     stream_module_filewriter.inl
     stream_file_exports.h
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_File SHARED ${ACEStream_File_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_File Common)
add_dependencies (${PROJECT_NAME}_File ${PROJECT_NAME})

#target_link_libraries (${PROJECT_NAME}_File libACEStream)
#target_link_libraries (${PROJECT_NAME}_File libCommon)
#target_link_libraries (${PROJECT_NAME}_File ${ACE_LIBRARY})

set_target_properties (${PROJECT_NAME}_File PROPERTIES DEFINE_SYMBOL STREAM_FILE_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_File PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_File
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_module_filereader.h
         stream_module_filereader.inl
         stream_module_filewriter.h
         stream_module_filewriter.inl
         stream_file_exports.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_File
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)