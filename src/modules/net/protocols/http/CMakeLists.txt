add_library (${PROJECT_NAME}_Network_Protocol_HTTPGet STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../modules/ACENetwork/src>)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../modules/ACENetwork/src/protocol/http>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${ACENetwork_ROOT}/src>)
 target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                             $<BUILD_INTERFACE:${ACENetwork_ROOT}/src/protocol/http>)
endif ()
target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../../../..>)
target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../..>)
target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../dec>)
target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../lib>)
target_include_directories (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../stat>)

############## next target ################
set (source_files
     stream_module_source_http_get.cpp
     stream_module_source_http_get.h
     stream_module_source_http_get.inl
     stream_net_http_defines.h)
#     stream_net_http_exports.h)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
      )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Network_Protocol_HTTPGet
               PRIVATE stdafx.cpp
               PRIVATE stdafx.h
               ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Network_Protocol_HTTPGet
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Network_Protocol_HTTPGet PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Network_Protocol_HTTPGet Common)
add_dependencies (${PROJECT_NAME}_Network_Protocol_HTTPGet ${PROJECT_NAME})
add_dependencies (${PROJECT_NAME}_Network_Protocol_HTTPGet ${PROJECT_NAME}_Network)

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC ${ACE_LIBRARY})
endif (WIN32)
target_link_libraries (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC ${PROJECT_NAME})
target_link_libraries (${PROJECT_NAME}_Network_Protocol_HTTPGet PUBLIC ${PROJECT_NAME}_Network)

#set_target_properties (${PROJECT_NAME}_Network_Protocol_HTTPGet PROPERTIES DEFINE_SYMBOL STREAM_NET_HTTP_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Network_Protocol_HTTPGet PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
#set_target_properties (${PROJECT_NAME}_Network_Protocol_HTTPGet PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${PROJECT_NAME}_Network_Protocol_HTTPGet)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Network_Protocol_HTTPGet
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_module_source_http_get.h
         stream_module_source_http_get.inl
         stream_net_http_defines.h
#         stream_net_http_exports.h
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Network_Protocol_HTTPGet
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
