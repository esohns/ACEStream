cmake_minimum_required (VERSION 2.8)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 find_package (Threads REQUIRED)
 check_library_exists (iwlib iw_sockets_open "" HAVE_LIB_IW)
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

set (ACE_SSL_LIB_FILE libACE_SSL.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES ace/lib build/linux/ace/lib
                DOC "searching for ${ACE_SSL_LIB_FILE}")
 else ()
  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                DOC "searching for ${ACE_SSL_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_SSL_LIB_FILE)
 set (ACE_SSL_LIB_FILE ACE_SSL${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_SSL_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_SSL_LIBRARY)
if (NOT ACE_SSL_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_SSL_LIB_FILE} (was: \"${ACE_SSL_LIBRARY}\"), aborting")
else ()
 message (STATUS "Found ACE SSL library \"${ACE_SSL_LIBRARY}\"")
endif ()

include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_Z REQUIRED zlib)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
 pkg_check_modules (PKG_SSL REQUIRED libssl)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
 pkg_check_modules (PKG_XML2 REQUIRED libxml-2.0)
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

if (UNIX)
 include_directories (/usr/lib/libreoffice/sdk/include)

 add_definitions (-DSAL_UNX)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../zlib-1.2.8)

 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME}/sdk)
 elseif (DEFINED ENV{UNO_PATH})
   set (OO_SDK_DIRECTORY $ENV{UNO_PATH}/../sdk)
 else ()
  message (FATAL_ERROR "%OO_SDK_HOME%/%UNO_PATH% not set, aborting")
 endif ()
# *NOTE*: workaround
# set(_PF86 "ProgramFiles(x86)")
# set (OO_SDK_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/LibreOffice_5.1_SDK/sdk"
#       "C:/LibreOffice_5.1_SDK_installer/sdk")
 message (STATUS "LibreOffice SDK directory: \"${OO_SDK_DIRECTORY}\"")
 include_directories (${OO_SDK_DIRECTORY}/include)

 if (DEFINED ENV{UNO_PATH})
  string (REGEX REPLACE "\\\\" "/" OO_APP_DIRECTORY $ENV{UNO_PATH})
#  set (OO_APP_DIRECTORY $ENV{UNO_PATH})
 else ()
  message (FATAL_ERROR "%UNO_PATH% not set, aborting")
 endif ()
 message (STATUS "LibreOffice directory: \"${OO_APP_DIRECTORY}\"")
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)

find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

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

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

# include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_DBUS_INCLUDE_DIRS})
 include_directories (${PKG_SSL_INCLUDE_DIRS})
 include_directories (${PKG_XML2_INCLUDE_DIRS})

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

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_HOME})
   set (OO_SDK_DIRECTORY $ENV{UNO_PATH}\..\sdk)
 else ()
  message (FATAL_ERROR "%OO_SDK_HOME%/%UNO_PATH% not set, aborting")
 endif ()
# *NOTE*: workaround
# set(_PF86 "ProgramFiles(x86)")
# set (OO_SDK_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/LibreOffice_5.1_SDK/sdk"
#       "C:/LibreOffice_5.1_SDK_installer/sdk")
 message (STATUS "LibreOffice SDK directory: \"${OO_SDK_DIRECTORY}\"")
 include_directories (${OO_SDK_DIRECTORY}/include)
# include_directories (${OO_SDK_DIRECTORY}/cli)

 add_definitions (-DCPPU_ENV=msci)
 add_definitions (-DWNT)

# *TODO*: repair pkgconfig support
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

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/doc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/html)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net/protocols/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (http_get_2_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_common_modules.h
#     test_i_connection_manager_common.h
     test_i_defines.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_spreadsheetwriter.cpp
     test_i_module_spreadsheetwriter.h
     test_i_module_htmlparser.cpp
     test_i_module_htmlparser.h
     test_i_module_httpget.cpp
     test_i_module_httpget.h
#     test_i_module_httpparser.cpp
#     test_i_module_httpparser.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_http_get.cpp
     test_i_http_get_common.h
     test_i_http_get_connection_common.h
     test_i_http_get_connection_manager_common.h
     test_i_http_get_signalhandler.cpp
     test_i_http_get_signalhandler.h
     test_i_http_get_stream.cpp
     test_i_http_get_stream.h
     test_i_http_get_stream.inl)
#  test_i_template_instantiations.cpp
set (http_get_2_ETC_SRCS
     etc/soffice.ini
     etc/sofficerc
     etc/symbols.ini
     etc/template.ods)

set (EXECUTABLE http_get_2)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${http_get_2_EXE_SRCS}
                ${http_get_2_ETC_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (NOT BUILD_TEST_I AND WIN32)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
endif ()

if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
 target_link_libraries (${EXECUTABLE} iw)
elseif (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE"
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)

# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/lib/libeay32.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/lib/ssleay32.lib)
endif ()
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
target_link_libraries (${EXECUTABLE} ${ACE_SSL_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_Z_LIBRARIES})
# *NOTE*: libCommon requires libcap
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_DBUS_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_SSL_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_XML2_LIBRARIES})

 target_link_libraries (${EXECUTABLE} /usr/lib/libreoffice/sdk/lib/libuno_sal.so)

 target_link_libraries (${EXECUTABLE} /usr/lib/ure/lib/libgcc3_uno.so)
 target_link_libraries (${EXECUTABLE} /usr/lib/libreoffice/sdk/lib/libuno_cppu.so)

 target_link_libraries (${EXECUTABLE} /usr/lib/ure/lib/libstorelo.so)
 target_link_libraries (${EXECUTABLE} /usr/lib/ure/lib/libreglo.so)

 target_link_libraries (${EXECUTABLE} /usr/lib/ure/lib/libxmlreaderlo.so)
 target_link_libraries (${EXECUTABLE} /usr/lib/ure/lib/libunoidllo.so)
 target_link_libraries (${EXECUTABLE} /usr/lib/libreoffice/sdk/lib/libuno_cppuhelpergcc3.so)

 #target_link_libraries (${EXECUTABLE} /usr/lib/libreoffice/sdk/lib/libuno_salhelpergcc3.so)
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} ntdll.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../zlib-1.2.8/zdll.lib)

# *TODO*: repair pkgconfig support
 #target_link_libraries (http_get ${LIBXML2_LIBRARY})
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.4/win32/bin.msvc/libxml2.lib)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/isal.lib)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/icppu.lib)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/icppuhelper.lib)
endif ()
#target_link_libraries (${EXECUTABLE} ${MYSQL_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ACEStream_Document)
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

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c./../../../test_i/http_get_2/etc/soffice.ini -f./../../../test_i/http_get_2/etc/symbols.ini -h localhost -l -o ./output.ods -p 2083 -t -u http://kurse.boerse.ard.de/ard/kurse_einzelkurs_suche.htn"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/http_get_2
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c.\\\\..\\\\..\\\\..\\\\test_i\\\\http_get_2\\\\etc\\\\soffice.ini -d -f.\\\\..\\\\..\\\\..\\\\test_i\\\\http_get_2\\\\etc\\\\template.ods -h localhost -i.\\\\..\\\\..\\\\..\\\\test_i\\\\http_get_2\\\\etc\\\\symbols.ini -l -o .\\\\output.ods -p 2083 -r -t -u http://kurse.boerse.ard.de/ard/kurse_einzelkurs_suche.htn"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/doc/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/html/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/net/protocols/http/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/protocol/http/Debug
                                              \"${OO_APP_DIRECTORY}\"
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/http_get_2
                         ENVIRONMENT foo=bar;_NO_DEBUG_HEAP=1)
endif ()
