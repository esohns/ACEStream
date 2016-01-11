cmake_minimum_required (VERSION 2.8)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACENetwork/src/protocol/http)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../libACENetwork/src/protocol/http)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
set (${PROJECT_NAME}_Network_Protocol_HTTPGet_LIB_SRCS
     stream_module_source_http_get.cpp
     stream_module_source_http_get.h
     stream_module_source_http_get.inl
     stream_net_http_exports.h
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_Network_Protocol_HTTPGet SHARED ${ACEStream_Network_Protocol_HTTPGet_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Network_Protocol_HTTPGet Common)
add_dependencies (${PROJECT_NAME}_Network_Protocol_HTTPGet ${PROJECT_NAME})
add_dependencies (${PROJECT_NAME}_Network_Protocol_HTTPGet ${PROJECT_NAME}_Network)

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Network_Protocol_HTTPGet ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Network_Protocol_HTTPGet PROPERTIES DEFINE_SYMBOL STREAM_NET_HTTP_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Network_Protocol_HTTPGet PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Network_Protocol_HTTPGet
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_module_source_http_get.h
         stream_module_source_http_get.inl
         stream_net_http_exports.h
         DESTINATION include)
install (TARGETS ${PROJECT_NAME}_Network_Protocol_HTTPGet
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
