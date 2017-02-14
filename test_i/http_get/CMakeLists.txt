cmake_minimum_required (VERSION 2.8)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 find_package (Threads REQUIRED)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES ace build/linux/ace
                DOC "searching for ${ACE_LIB_FILE}")
 else ()
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                DOC "searching for ${ACE_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_LIB_FILE)
 set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()

#set (ACE_SSL_LIB_FILE libACE_SSL.so)
#if (UNIX)
# if (DEFINED ENV{ACE_ROOT})
#  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
#                PATHS ENV ACE_ROOT
#                PATH_SUFFIXES ace/lib build/linux/ace/lib
#                DOC "searching for ${ACE_SSL_LIB_FILE}")
# else ()
#  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
#                DOC "searching for ${ACE_SSL_LIB_FILE} (system paths)")
# endif ()
#elseif (WIN32)
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  set (LIB_FILE_SUFFIX d)
# endif ()
# unset (ACE_SSL_LIB_FILE)
# set (ACE_SSL_LIB_FILE ACE_SSL${LIB_FILE_SUFFIX}.lib)
# find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
#               PATHS ENV ACE_ROOT
#               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
#               DOC "searching for ${ACE_SSL_LIB_FILE}"
#               NO_DEFAULT_PATH)
#endif ()
##if (NOT EXISTS ACE_SSL_LIBRARY)
#if (NOT ACE_SSL_LIBRARY)
# message (FATAL_ERROR "could not find ${ACE_SSL_LIB_FILE} (was: \"${ACE_SSL_LIBRARY}\"), aborting")
#else ()
# message (STATUS "Found ACE SSL library \"${ACE_SSL_LIBRARY}\"")
#endif ()

include (FindPkgConfig)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_SSL REQUIRED libssl)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
 pkg_check_modules (PKG_LIBXML2 REQUIRED libxml-2.0)
endif ()

#if (WIN32)
# *TODO*: repair pkgconfig support
# find_library (LIBXML2_LIBRARY libxml2.dll.a
#               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.1
#               PATH_SUFFIXES .libs
#               DOC "searching for libxml2.dll.a"
#               NO_DEFAULT_PATH)
# if (NOT LIBXML2_LIBRARY)
#  message (FATAL_ERROR "could not find libxml2 (was: \"libxml2.dll.a\"), aborting")
# endif ()
#endif ()

#pkg_check_modules (PKG_MYSQL REQUIRED libmysql)
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

find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_DB_HAS_DLL)

#string (TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libACENetwork/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()
add_definitions (-DHTTP_HAS_DLL)

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_SSL_INCLUDE_DIRS})
 include_directories (${PKG_LIBXML2_INCLUDE_DIRS})
 include_directories (${PKG_MYSQL_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/cmake)

else ()
 include_directories ($ENV{ACE_ROOT})

# include_directories (${PKG_LIBXML2_INCLUDE_DIRS})
# *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libiconv-1.9.2-1/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.4/include)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)

# *TODO*: repair pkgconfig support
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libmysql/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libmysql_2/cmake/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libmysql_2/include)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/glib-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/gtk-2.0)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/include/libglade-2.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/cmake)

endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/db)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/html)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net/protocols/http)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (http_get_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_common.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_defines.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_databasewriter.cpp
     test_i_module_databasewriter.h
     test_i_module_htmlparser.cpp
     test_i_module_htmlparser.h
     test_i_module_htmlwriter.cpp
     test_i_module_htmlwriter.h
#     test_i_module_httpparser.cpp
#     test_i_module_httpparser.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_http_get.cpp
     test_i_http_get_common.h
     test_i_http_get_network.h
     test_i_http_get_signalhandler.cpp
     test_i_http_get_signalhandler.h
     test_i_http_get_stream.cpp
     test_i_http_get_stream.h
     test_i_http_get_stream.inl)
#  test_i_template_instantiations.cpp

set (EXECUTABLE http_get)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${http_get_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")

if (CMAKE_BUILD_TYPE STREQUAL Debug AND WIN32)
 set_target_properties (${EXECUTABLE} PROPERTIES COMPILE_FLAGS "/wd4503")
endif ()

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
elseif (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE"
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
target_link_libraries (${EXECUTABLE} ${ACE_SSL_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBXML2_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} ntdll.lib)
# *TODO*: repair pkgconfig support
 #target_link_libraries (http_get ${LIBXML2_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.4/win32/bin.msvc/libxml2.lib)
endif ()
target_link_libraries (${EXECUTABLE} ${MYSQL_LIBRARY})
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Database)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ACEStream_HTML)
target_link_libraries (${EXECUTABLE} ACENetwork)
target_link_libraries (${EXECUTABLE} ACENetwork_Protocol_HTTP)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)

#set_tests_properties (${EXECUTABLE} PROPERTIES ENVIRONMENT "PATH=%PATH%;$(ACE_ROOT)\\lib;D:\\projects\\libiconv-1.9.2\\bin;D:\\projects\\libxml2-2.9.4\\win32\\bin;$(ProgramFiles)\\LibreOffice 5\\program;$(ProjectDir)..\\..\\..\\..\\libCommon\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\libCommon\\cmake\\src\\err\\$(Configuration);$(ProjectDir)..\\..\\src\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\dec\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\html\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\libACENetwork\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\libACENetwork\\cmake\\src\\protocol\\http\\$(Configuration)")

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c.\\\\..\\\\..\\\\..\\\\test_i\\\\http_get\\\\etc\\\\options.cnf -d news -e articles -f output.txt -l -p 80 -t -u http://www.heise.de/newsticker.html"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.4/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/db/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/html/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/net/protocols/http/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/protocol/http/Debug
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/http_get
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c.\\\\etc\\\\options.cnf -f output.txt -l -p 80 -r -t -u http://www.heise.de/newsticker.html"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../libiconv-1.9.2/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.4/win32/bin
                                              ${PROJECT_SOURCE_DIR}/../libmysql_2/cmake/libmysql/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/db/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/html/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/net/protocols/http/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/protocol/http/Debug
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/http_get
                         ENVIRONMENT foo=bar)
endif ()
