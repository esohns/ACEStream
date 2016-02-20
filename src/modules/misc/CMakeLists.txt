cmake_minimum_required (VERSION 2.8)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
if (WIN32)
 set (${PROJECT_NAME}_Miscellaneous_LIB_SRCS
      stream_misc_defines.h
      stream_misc_directshow_source.cpp
      stream_misc_directshow_source.h
      stream_misc_directshow_source.inl
      stream_misc_directshow_source_filter.cpp
      stream_misc_directshow_source_filter.h
      stream_misc_dump.cpp
      stream_misc_dump.h
      stream_misc_dump.inl
      stream_misc_exports.h
      stream_misc_messagehandler.cpp
      stream_misc_messagehandler.h
      stream_misc_messagehandler.inl
      stream_misc_queue_source.cpp
      stream_misc_queue_source.h
      stream_misc_queue_source.inl
      stream_misc_runtimestatistic.cpp
      stream_misc_runtimestatistic.h
      stream_misc_runtimestatistic.inl
      stream_misc_splitter.cpp
      stream_misc_splitter.h
      stream_misc_splitter.inl
      stdafx.cpp
      stdafx.h)
elseif (UNIX)
 set (${PROJECT_NAME}_Miscellaneous_LIB_SRCS
      stream_misc_defines.h
      stream_misc_dump.cpp
      stream_misc_dump.h
      stream_misc_dump.inl
      stream_misc_exports.h
      stream_misc_messagehandler.cpp
      stream_misc_messagehandler.h
      stream_misc_messagehandler.inl
      stream_misc_queue_source.cpp
      stream_misc_queue_source.h
      stream_misc_queue_source.inl
      stream_misc_runtimestatistic.cpp
      stream_misc_runtimestatistic.h
      stream_misc_runtimestatistic.inl
      stream_misc_splitter.cpp
      stream_misc_splitter.h
      stream_misc_splitter.inl
      stdafx.cpp
      stdafx.h)
endif ()

add_library (${PROJECT_NAME}_Miscellaneous SHARED ${ACEStream_Miscellaneous_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Miscellaneous Common)
add_dependencies (${PROJECT_NAME}_Miscellaneous ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Miscellaneous ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES DEFINE_SYMBOL STREAM_MISC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Miscellaneous
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (WIN32)
 install (FILES
          stream_misc_defines.h
          stream_misc_directshow_source.h
          stream_misc_directshow_source.inl
          stream_misc_directshow_source_filter.h
          stream_misc_dump.h
          stream_misc_dump.inl
          stream_misc_exports.h
          stream_misc_messagehandler.h
          stream_misc_messagehandler.inl
          stream_misc_queue_source.h
          stream_misc_queue_source.inl
          stream_misc_runtimestatistic.h
          stream_misc_runtimestatistic.inl
          DESTINATION include)
elseif (UNIX)
 install (FILES
          stream_misc_defines.h
          stream_misc_dump.h
          stream_misc_dump.inl
          stream_misc_exports.h
          stream_misc_messagehandler.h
          stream_misc_messagehandler.inl
          stream_misc_queue_source.h
          stream_misc_queue_source.inl
          stream_misc_runtimestatistic.h
          stream_misc_runtimestatistic.inl
          stream_misc_splitter.h
          stream_misc_splitter.inl
          DESTINATION include)
endif ()
install (TARGETS ${PROJECT_NAME}_Miscellaneous
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)

