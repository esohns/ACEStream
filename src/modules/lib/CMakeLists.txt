if (WIN32)
 if (${CMAKE_SYSTEM_VERSION} LESS 5.1)      # < Windows XP (see: <sdkddkver.h>)
 elseif (${CMAKE_SYSTEM_VERSION} LESS 6.0)  # < Windows Vista
  option (VMR9_SUPPORT "prefer DirectShow Video Mixing Renderer 9 (VMR-9) over VMR-7"
          OFF)
 else ()                                    # Some other Windows
 endif ()
endif ()

if (WIN32)
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")

 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ffmpeg)
endif ()

if (UNIX)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/math)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
if (WIN32)
 set (${PROJECT_NAME}_MediaFramework_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_lib_common.h
      stream_lib_defines.h
      stream_lib_directshow_allocator.cpp
      stream_lib_directshow_allocator.h
      stream_lib_directshow_allocator.inl
      stream_lib_directshow_asynch_source_filter.cpp
      stream_lib_directshow_asynch_source_filter.h
      stream_lib_directshow_asynch_source_filter.inl
      stream_lib_directshow_source_filter.cpp
      stream_lib_directshow_source_filter.h
      stream_lib_directshow_source_filter.inl
      stream_lib_directshow_target.cpp
      stream_lib_directshow_target.h
      stream_lib_directshow_target.inl
      stream_lib_exports.h
      stream_lib_mediafoundation_callback.cpp
      stream_lib_mediafoundation_callback.h
      stream_lib_mediafoundation_callback.inl
      stream_lib_mediafoundation_mediasource.cpp
      stream_lib_mediafoundation_mediasource.h
      stream_lib_mediafoundation_mediasource.inl
      stream_lib_mediafoundation_source.cpp
      stream_lib_mediafoundation_source.h
      stream_lib_mediafoundation_source.inl
#      stream_lib_mediafoundation_source_stream.cpp
#      stream_lib_mediafoundation_source_stream.h
#      stream_lib_mediafoundation_source_stream.inl
      stream_lib_mediafoundation_target.cpp
      stream_lib_mediafoundation_target.h
      stream_lib_mediafoundation_target.inl
      targetver.h)
elseif (UNIX)
 set (${PROJECT_NAME}_MediaFramework_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_lib_common.h
      stream_lib_defines.h
      stream_lib_exports.h)
endif ()

if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
 # *NOTE*: DirectShow uses 'DEBUG'
 add_definitions (-DDEBUG)
endif ()

add_library (${PROJECT_NAME}_MediaFramework SHARED
             ${ACEStream_MediaFramework_LIB_SRCS})
set_property (TARGET ${PROJECT_NAME}_MediaFramework PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_MediaFramework Common)
add_dependencies (${PROJECT_NAME}_MediaFramework ${PROJECT_NAME})

if (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous strmbase.lib)
 endif ()
# target_link_libraries (${PROJECT_NAME}_Miscellaneous winmm.lib)
 target_link_libraries (${PROJECT_NAME}_MediaFramework ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES DEFINE_SYMBOL STREAM_LIB_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_MediaFramework PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_MediaFramework)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_MediaFramework
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (WIN32)
 install (FILES
          stream_lib_common.h
          stream_lib_defines.h
          stream_lib_directshow_allocator.h
          stream_lib_directshow_allocator.inl
          stream_lib_directshow_asynch_source_filter.h
          stream_lib_directshow_asynch_source_filter.inl
          stream_lib_directshow_source_filter.h
          stream_lib_directshow_source_filter.inl
          stream_lib_directshow_target.h
          stream_lib_directshow_target.inl
          stream_lib_exports.h
          stream_lib_mediafoundation_callback.h
          stream_lib_mediafoundation_callback.inl
          stream_lib_mediafoundation_mediasource.h
          stream_lib_mediafoundation_mediasource.inl
          stream_lib_mediafoundation_source.h
          stream_lib_mediafoundation_source.inl
          stream_lib_mediafoundation_target.h
          stream_lib_mediafoundation_target.inl
          targetver.h
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
elseif (UNIX)
 install (FILES
          stream_lib_common.h
          stream_lib_defines.h
          stream_lib_exports.h
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()

install (TARGETS ${PROJECT_NAME}_MediaFramework
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)