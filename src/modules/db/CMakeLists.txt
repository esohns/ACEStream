set (MYSQL_LIB_FILE libmysqlclient.so)
if (UNIX)
# pkg_check_modules (PKG_MYSQL REQUIRED libmysqlclient-dev)
 pkg_check_modules (PKG_MYSQL REQUIRED mysqlclient) # Fedora 29
# find_library (MYSQL_LIBRARY ${MYSQL_LIB_FILE}
#               PATHS /usr/lib
#               PATH_SUFFIXES x86_64-linux-gnu
#               DOC "searching for \"${MYSQL_LIB_FILE}\"")
elseif (WIN32)
 unset (MYSQL_LIB_FILE)
 set (MYSQL_LIB_FILE libmysql.lib)
 find_library (MYSQL_LIBRARY ${MYSQL_LIB_FILE}
#               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libmysql
#               PATH_SUFFIXES lib
               PATHS $ENV{LIB_ROOT}/libmysql
               PATH_SUFFIXES lib
#               PATH_SUFFIXES cmake/libmysql/Debug
               DOC "searching for \"${MYSQL_LIB_FILE}\"")
 if (NOT MYSQL_LIBRARY)
  message (FATAL_ERROR "could not find libmysql (was: \"${MYSQL_LIB_FILE}\"), aborting")
 endif (NOT MYSQL_LIBRARY)
endif ()
#add_dependencies (${PROJECT_NAME}_Database libmysqlpp.so)

add_library (${PROJECT_NAME}_Database STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${PKG_MYSQL_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules/ACENetwork/src>)
elseif (WIN32)
# *TODO*: repair pkgconfig support
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libmysql/include)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/libmysql_2/cmake/include>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/libmysql_2/include>)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Database PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Database PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/lib>)
 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Database PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACENetwork/src>)
endif ()
target_include_directories (${PROJECT_NAME}_Database PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Database PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
target_include_directories (${PROJECT_NAME}_Database PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../stat>)

############## next target ################
set (source_files
     stream_db_exports.h
     stream_module_db_common.h
     stream_module_db_defines.h
     stream_module_db_tools.cpp
     stream_module_db_tools.h
     stream_module_mysqlreader.cpp
     stream_module_mysqlreader.h
     stream_module_mysqlreader.inl
     stream_module_mysqlwriter.cpp
     stream_module_mysqlwriter.h
     stream_module_mysqlwriter.inl)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
      )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Database
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

if (CMAKE_CXX_COMPILER_ID STREQUAL "CLang")
 add_compile_options (-fPIC)
endif ()

set_property (TARGET ${PROJECT_NAME}_Database PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Database Common)
add_dependencies (${PROJECT_NAME}_Database ${PROJECT_NAME})

if (WIN32)
 if (MSVC)
#  message (STATUS "MSVC_VERSION: ${MSVC_VERSION}")
  if (MSVC_VERSION GREATER 1600)
#  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_link_libraries (${PROJECT_NAME}_Database ucrt${LIB_FILE_SUFFIX}.lib)
   target_link_libraries (${PROJECT_NAME}_Database vcruntime${LIB_FILE_SUFFIX}.lib)
  endif ()
 endif (MSVC)
 target_link_libraries (${PROJECT_NAME}_Database ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Database ${MYSQL_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Database Common)
target_link_libraries (${PROJECT_NAME}_Database ${PROJECT_NAME})

set_target_properties (${PROJECT_NAME}_Database PROPERTIES DEFINE_SYMBOL STREAM_DB_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Database PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Database PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Database)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Database
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_db_exports.h
         stream_module_db_common.h
         stream_module_db_defines.h
         stream_module_db_tools.h
         stream_module_mysqlreader.h
         stream_module_mysqlreader.inl
         stream_module_mysqlwriter.h
         stream_module_mysqlwriter.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Database
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
