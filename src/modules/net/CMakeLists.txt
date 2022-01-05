add_library (${PROJECT_NAME}_Network STATIC "")

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)
message (STATUS "BUILD_PATH_SUFFIX: ${BUILD_PATH_SUFFIX}")
find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
              PATH_SUFFIXES ACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find ACENetwork, aborting")
endif (NOT EXISTS ${ACENetwork_DIR})

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACENetwork/src>)
 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACENetwork/src/client_server>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACENetwork/src>)
 target_include_directories (${PROJECT_NAME}_Network PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACENetwork/src/client_server>)
endif ()
target_include_directories (${PROJECT_NAME}_Network PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Network PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Network PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
target_include_directories (${PROJECT_NAME}_Network PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../stat>)

############## next target ################
set (source_files
     stream_net_common.h
     stream_net_defines.h
#     stream_net_exports.h
     stream_net_io.cpp
     stream_net_io.h
     stream_net_io.inl
     stream_net_io_stream.cpp
     stream_net_io_stream.h
     stream_net_io_stream.inl
     stream_net_source.cpp
     stream_net_source.h
     stream_net_source.inl
     stream_net_target.cpp
     stream_net_target.h
     stream_net_target.inl)

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
target_sources (${PROJECT_NAME}_Network
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Network
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Network PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Network Common)
add_dependencies (${PROJECT_NAME}_Network ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Network PUBLIC ${ACE_LIBRARY})
endif (WIN32)
target_link_libraries (${PROJECT_NAME}_Network PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Network PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Network PROPERTIES DEFINE_SYMBOL STREAM_NET_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Network PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Network
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_net_common.h
         stream_net_defines.h
#         stream_net_exports.h
         stream_net_io.h
         stream_net_io.inl
         stream_net_io_stream.h
         stream_net_io_stream.inl
         stream_net_source.h
         stream_net_source.inl
         stream_net_target.h
         stream_net_target.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Network
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)

add_subdirectory (protocols)
