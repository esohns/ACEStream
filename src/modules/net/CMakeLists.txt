include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
endif ()

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

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../stat)
if (UNIX)
 include_directories (${PKG_DBUS_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACENetwork/src/client_server)
endif ()
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../..)

############## next target ################
set (${PROJECT_NAME}_Network_LIB_SRCS
     stdafx.cpp
     stdafx.h
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
     stream_net_exports.h)

add_library (${PROJECT_NAME}_Network SHARED
             ${ACEStream_Network_LIB_SRCS})
set_property (TARGET ${PROJECT_NAME}_Network PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Network Common)
add_dependencies (${PROJECT_NAME}_Network ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Network ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Network PROPERTIES DEFINE_SYMBOL STREAM_NET_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Network PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Network PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Network)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Network
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
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Network
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)

add_subdirectory (protocols)
