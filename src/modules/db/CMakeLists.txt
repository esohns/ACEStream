set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (mysql)
include (SOCI)

add_library (${PROJECT_NAME}_Database STATIC "")

if (MYSQL_SUPPORT)
# message (STATUS "MYSQL_INCLUDE_DIRS: \"${MYSQL_INCLUDE_DIRS}\"")
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             "$<BUILD_INTERFACE:${MYSQL_INCLUDE_DIRS}>")
endif (MYSQL_SUPPORT)
if (SOCI_SUPPORT)
# message (STATUS "SOCI_INCLUDE_DIRS: \"${SOCI_INCLUDE_DIRS}\"")
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${SOCI_INCLUDE_DIRS}>)
endif (SOCI_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
endif ()
target_include_directories (${PROJECT_NAME}_Database PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
                            $<INSTALL_INTERFACE:include/ACEStream>)
target_include_directories (${PROJECT_NAME}_Database PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>
                            $<INSTALL_INTERFACE:include/ACEStream/lib>)
target_include_directories (${PROJECT_NAME}_Database PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../stat>
                            $<INSTALL_INTERFACE:include/ACEStream/stat>)

############## next target ################
set (source_files
     stream_module_db_common.h
     stream_module_db_defines.h
     stream_module_db_tools.cpp
     stream_module_db_tools.h)
if (MYSQL_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_mysqlreader.cpp
      stream_module_mysqlreader.h
      stream_module_mysqlreader.inl
      stream_module_mysqlwriter.cpp
      stream_module_mysqlwriter.h
      stream_module_mysqlwriter.inl)
endif (MYSQL_SUPPORT)

# Include dir for installed targets
#set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
      )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Database
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})

target_precompile_headers (${PROJECT_NAME}_Database
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Database PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Database Common)
add_dependencies (${PROJECT_NAME}_Database ${PROJECT_NAME})

if (MYSQL_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Database PUBLIC ${MYSQL_LIBRARIES})
endif (MYSQL_SUPPORT)
if (WIN32)
 if (MSVC)
  if (MSVC_VERSION GREATER 1600)
   target_link_libraries (${PROJECT_NAME}_Database PUBLIC ucrt${LIB_FILE_SUFFIX}.lib)
   target_link_libraries (${PROJECT_NAME}_Database PUBLIC vcruntime${LIB_FILE_SUFFIX}.lib)
  endif (MSVC_VERSION GREATER 1600)
 endif (MSVC)
 target_link_libraries (${PROJECT_NAME}_Database PUBLIC ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Database PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_Database PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Database PROPERTIES DEFINE_SYMBOL STREAM_DB_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Database PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
#set_target_properties (${PROJECT_NAME}_Database PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${PROJECT_NAME}_Database)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Database
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (source_files
#     stream_db_exports.h
     stream_module_db_common.h
     stream_module_db_defines.h
     stream_module_db_tools.h)
if (MYSQL_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_mysqlreader.h
      stream_module_mysqlreader.inl
      stream_module_mysqlwriter.h
      stream_module_mysqlwriter.inl)
endif (MYSQL_SUPPORT)
install (FILES
         ${source_files}
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Database
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
