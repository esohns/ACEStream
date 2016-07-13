cmake_minimum_required (VERSION 2.8)

if (WIN32)
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")

 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dev)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
if (WIN32)
 set (${PROJECT_NAME}_Miscellaneous_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_misc_common.h
      stream_misc_defines.h
      stream_misc_directshow_allocator.cpp
      stream_misc_directshow_allocator.h
      stream_misc_directshow_allocator.inl
      stream_misc_directshow_asynch_source_filter.cpp
      stream_misc_directshow_asynch_source_filter.h
      stream_misc_directshow_asynch_source_filter.inl
      stream_misc_directshow_source.cpp
      stream_misc_directshow_source.h
      stream_misc_directshow_source.inl
      stream_misc_directshow_source_filter.cpp
      stream_misc_directshow_source_filter.h
      stream_misc_directshow_source_filter.inl
      stream_misc_dump.cpp
      stream_misc_dump.h
      stream_misc_dump.inl
      stream_misc_exports.h
      stream_misc_mediafoundation_mediasource.cpp
      stream_misc_mediafoundation_mediasource.h
      stream_misc_mediafoundation_mediasource.inl
      stream_misc_mediafoundation_source.cpp
      stream_misc_mediafoundation_source.h
      stream_misc_mediafoundation_source.inl
#      stream_misc_mediafoundation_source_stream.cpp
#      stream_misc_mediafoundation_source_stream.h
#      stream_misc_mediafoundation_source_stream.inl
      stream_misc_mediafoundation_target.cpp
      stream_misc_mediafoundation_target.h
      stream_misc_mediafoundation_target.inl
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
      stream_misc_splitter.inl)
elseif (UNIX)
 set (${PROJECT_NAME}_Miscellaneous_LIB_SRCS
      stream_misc_common.h
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
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
# else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous strmbase.lib)
# endif ()
# target_link_libraries (${PROJECT_NAME}_Miscellaneous winmm.lib)
 target_link_libraries (${PROJECT_NAME}_Miscellaneous ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES DEFINE_SYMBOL STREAM_MISC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Miscellaneous)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Miscellaneous
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (WIN32)
 install (FILES
          stream_misc_common.h
          stream_misc_defines.h
          stream_misc_directshow_allocator.h
          stream_misc_directshow_allocator.inl
          stream_misc_directshow_asynch_source_filter.h
          stream_misc_directshow_asynch_source_filter.inl
          stream_misc_directshow_source.h
          stream_misc_directshow_source.inl
          stream_misc_directshow_source_filter.h
          stream_misc_directshow_source_filter.inl
          stream_misc_dump.h
          stream_misc_dump.inl
          stream_misc_exports.h
          stream_misc_messagehandler.h
          stream_misc_messagehandler.inl
          stream_misc_queue_source.h
          stream_misc_queue_source.inl
          stream_misc_runtimestatistic.h
          stream_misc_runtimestatistic.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
elseif (UNIX)
 install (FILES
          stream_misc_common.h
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
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()

install (TARGETS ${PROJECT_NAME}_Miscellaneous
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
