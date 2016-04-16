cmake_minimum_required (VERSION 2.8)

include (FindPkgConfig)
if (UNIX)
# pkg_check_modules (PKG_MYSQL REQUIRED libmysqlclient-dev)
endif ()

# *TODO*: add dependencies
set (MYSQL_LIB_FILE libmysqlclient.so)
if (UNIX)
 find_library (MYSQL_LIBRARY ${MYSQL_LIB_FILE}
               PATHS /usr/lib
               PATH_SUFFIXES x86_64-linux-gnu
               DOC "searching for \"${MYSQL_LIB_FILE}\"")
elseif (WIN32)
 unset (MYSQL_LIB_FILE)
 set (MYSQL_LIB_FILE libmysql.lib)
 find_library (MYSQL_LIBRARY ${MYSQL_LIB_FILE}
#               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libmysql
#               PATH_SUFFIXES lib
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libmysql_2
               PATH_SUFFIXES cmake/libmysql/Debug
               DOC "searching for \"${MYSQL_LIB_FILE}\"")
endif ()
if (NOT MYSQL_LIBRARY)
 message (FATAL_ERROR "could not find libmysql (was: \"${MYSQL_LIB_FILE}\"), aborting")
endif ()
#add_dependencies (${PROJECT_NAME}_Database libmysqlpp.so)

if (UNIX)
# include_directories (${PKG_PKG_MYSQL_INCLUDE_DIRS})
elseif (WIN32)
# *TODO*: repair pkgconfig support
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libmysql/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libmysql_2/cmake/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libmysql_2/include)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)

############## next target ################
set (${PROJECT_NAME}_Database_LIB_SRCS
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
     stream_module_mysqlwriter.inl
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_Database SHARED ${ACEStream_Database_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Database Common)
add_dependencies (${PROJECT_NAME}_Database ${PROJECT_NAME})

if (WIN32)
# if (MSVC)
#  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_link_libraries (${PROJECT_NAME}_Database vcruntime${LIB_FILE_SUFFIX}.lib ucrt${LIB_FILE_SUFFIX}.lib)
#  endif ()
# endif ()
 target_link_libraries (${PROJECT_NAME}_Database ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME}_Database ${MYSQL_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Database PROPERTIES DEFINE_SYMBOL STREAM_DB_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Database PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

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
