set (MYSQL_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
# pkg_check_modules (PKG_MYSQL REQUIRED libmysqlclient-dev)
 pkg_check_modules (PKG_MYSQL mysqlclient) # Fedora 29
 if (PKG_MYSQL_FOUND)
  set (MYSQL_FOUND TRUE)
  set (MYSQL_INCLUDE_DIRS "${PKG_MYSQL_INCLUDE_DIRS}")
  set (MYSQL_LIBRARIES "${PKG_MYSQL_LIBRARIES}")
  set (MYSQL_LINK_DIRS "${PKG_MYSQL_LIBRARY_DIRS}")
 else ()
  set (MYSQL_LIB_FILE libmysqlclient.so)
  find_library (MYSQL_LIBRARY ${MYSQL_LIB_FILE}
                PATHS /usr/lib64
                PATH_SUFFIXES mysql
                DOC "searching for ${MYSQL_LIB_FILE}")
  if (MYSQL_LIBRARY)
   set (MYSQL_FOUND TRUE)
   set (MYSQL_INCLUDE_DIRS "/usr/include/mysql")
   set (MYSQL_LIBRARIES "${MYSQL_LIBRARY}")
  endif (MYSQL_LIBRARY)
 endif (PKG_MYSQL_FOUND)
elseif (WIN32)
 if (VCPKG_SUPPORT)
#  cmake_policy (SET CMP0074 OLD)
  find_package (mysql CONFIG)
  if (mysql_FOUND)
   set (MYSQL_FOUND TRUE)
   if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
       CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set (MYSQL_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/bin")
   else ()
    set (MYSQL_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin")
   endif ()
  endif (mysql_FOUND)
 endif (VCPKG_SUPPORT)
 if (NOT MYSQL_FOUND)
  set (MYSQL_LIB_FILE libmysql.lib)
  find_library (MYSQL_LIBRARY ${MYSQL_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/libmysql
                PATH_SUFFIXES lib
                DOC "searching for ${MYSQL_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT MYSQL_LIBRARY)
   message (WARNING "could not find ${MYSQL_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${MYSQL_LIB_FILE} library \"${MYSQL_LIBRARY}\"")
   set (MYSQL_FOUND TRUE)
   set (MYSQL_INCLUDE_DIRS "$ENV{LIB_ROOT}/libmysql/include")
   set (MYSQL_LIBRARIES "${MYSQL_LIBRARY}")
   set (MYSQL_LIB_DIR "$ENV{LIB_ROOT}/libmysql/bin")
  endif (NOT MYSQL_LIBRARY)
 endif (NOT MYSQL_FOUND)
endif ()
if (MYSQL_FOUND)
 option (MYSQL_SUPPORT "enable mysql support" ${MYSQL_SUPPORT_DEFAULT})
 if (MYSQL_SUPPORT)
  add_definitions (-DMYSQL_SUPPORT)
#  include_directories (${MYSQL_INCLUDE_DIRS})
 endif (MYSQL_SUPPORT)
endif (MYSQL_FOUND)
