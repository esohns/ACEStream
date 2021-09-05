include (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/scripts/SSLConfig.cmake)

if (UNIX)
 find_package (ZLIB)
 if (ZLIB_FOUND)
  add_definitions (-DZLIB_SUPPORT)
 endif (ZLIB_FOUND)
# pkg_check_modules (PKG_Z REQUIRED zlib)

# pkg_check_modules (PKG_XML2 REQUIRED libxml-2.0)
# pkg_check_modules (PKG_FFMPEG REQUIRED libswscale libavutil libavformat libavcodec)
#elseif (WIN32)
# *TODO*: repair pkgconfig support
# find_library (LIBXML2_LIBRARY libxml2.dll.a
#               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.1
#               PATH_SUFFIXES .libs
#               DOC "searching for libxml2.dll.a"
#               NO_DEFAULT_PATH)
# if (NOT LIBXML2_LIBRARY)
#  message (FATAL_ERROR "could not find libxml2 (was: \"libxml2.dll.a\"), aborting")
# endif ()
endif (UNIX)

if (UNIX)
 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_PATH})
   message (WARNING "OO_SDK_HOME not set, falling back to UNO_PATH (was: \"${UNO_PATH}\")")
   set (OO_SDK_DIRECTORY $ENV{UNO_PATH}/../sdk)
 else ()
  set (OO_SDK_DIRECTORY /usr/lib64/libreoffice/sdk)
  #set (OO_SDK_DIRECTORY /usr/lib/libreoffice/sdk)
  message (WARNING "OO_SDK_HOME|UNO_PATH not set, falling back to /usr/lib64/libreoffice/sdk")
 endif ()
 if (NOT EXISTS ${OO_SDK_DIRECTORY})
  message (FATAL_ERROR "OpenOffice SDK directory not found (was: \"${OO_SDK_DIRECTORY}\"), aborting")
 endif (NOT EXISTS ${OO_SDK_DIRECTORY})

 add_definitions (-DSAL_UNX)

# include_directories (${OO_SDK_DIRECTORY}/include)
 include_directories (/usr/include/libreoffice)
elseif (WIN32)
 include_directories (${LIB_ROOT}/zlib-1.2.8)

 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_PATH})
   set (OO_SDK_DIRECTORY $ENV{UNO_PATH}/../sdk)
 else ()
  message (WARNING "%OO_SDK_HOME%/%UNO_PATH% not set, falling back")
  set (OO_SDK_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../openoffice/sdk)
 endif ()
# *NOTE*: workaround
# set(_PF86 "ProgramFiles(x86)")
# set (OO_SDK_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/LibreOffice_5.1_SDK/sdk"
#       "C:/LibreOffice_5.1_SDK_installer/sdk")
#message (STATUS "Apache OpenOffice SDK directory: \"${OO_SDK_DIRECTORY}\"")
 include_directories (${OO_SDK_DIRECTORY}/include)

 if (DEFINED ENV{UNO_PATH})
  string (REGEX REPLACE "\\\\" "/" OO_APP_DIRECTORY $ENV{UNO_PATH})
#  set (OO_APP_DIRECTORY $ENV{UNO_PATH})
 else ()
  message (WARNING "%UNO_PATH% not set, falling back")
  string (REGEX REPLACE "\\\\" "/" OO_APP_DIRECTORY ${OO_SDK_DIRECTORY}/program)
 endif ()
# message (STATUS "Apache OpenOffice UNO directory: \"${OO_APP_DIRECTORY}\"")
# *NOTE*: the OpenOffice SDK provides its own snprintf; however MSVC complains
#         about mutliple definitions (and inconsistent linkage)
#         --> disable that header and use the native function
 if (MSVC)
  add_definitions (-D_SNPRINTF_H)
 endif ()
endif ()
#if (SSL_SUPPORT)
# add_definitions (-DSSL_SUPPORT)
#endif (SSL_SUPPORT)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
#find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
#              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${Common_DIR})
# message (FATAL_ERROR "could not find package \"Common\", aborting")
#endif ()

#find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
#              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${ACEStream_DIR})
# message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
#endif ()
#add_definitions (-DSTREAM_DB_HAS_DLL)

#string (TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 add_definitions (-DSAL_UNX) # libreoffice
 add_definitions (-DUNX)     # openoffice
 add_definitions (-DCPPU_ENV=gcc3)

 include_directories (${PKG_XML2_INCLUDE_DIRS})
# include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

# include_directories (/usr/local/src/ACE_wrappers)
# include_directories ($ENV{LIB_ROOT}/ACE_TAO/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/cmake)
elseif (WIN32)
# include_directories (${PKG_LIBXML2_INCLUDE_DIRS})
# *TODO*: repair pkgconfig support
 include_directories ($ENV{LIB_ROOT}/libiconv-1.9.2-1/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.4/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

 include_directories (${OO_SDK_DIRECTORY}/include)
# include_directories (${OO_SDK_DIRECTORY}/cli)

 add_definitions (-DCPPU_ENV=msci)
 add_definitions (-DWNT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories ($ENV{ACE_ROOT})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
 #include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/cmake)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/include)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/doc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/html)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net/protocols/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (EXECUTABLE HTTP_get_2)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_common_modules.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_spreadsheetwriter.cpp
     test_i_module_spreadsheetwriter.h
     test_i_module_htmlparser.cpp
     test_i_module_htmlparser.h
     test_i_module_httpget.cpp
     test_i_module_httpget.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_http_get.cpp
     test_i_http_get_common.h
     test_i_http_get_connection_common.h
     test_i_http_get_connection_manager_common.h
     test_i_http_get_defines.h
     test_i_http_get_signalhandler.cpp
     test_i_http_get_signalhandler.h
     test_i_http_get_stream.cpp
     test_i_http_get_stream.h
     test_i_http_get_stream.inl)
if (WIN32)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_module_msoffice_spreadsheetwriter.cpp
      test_i_module_msoffice_spreadsheetwriter.h)
endif (WIN32)
set (${EXECUTABLE}_EXE_SCRIPTS
     etc/soffice.ini
     etc/sofficerc
     etc/symbols.ini
     etc/template.ods)

#if (UNIX)
# add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
#endif (UNIX)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${HTTP_get_2_EXE_SRCS}
                ${HTTP_get_2_EXE_SCRIPTS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

if (UNIX)
 target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
elseif (WIN32)
 # *WARNING*: linking Common requires "/FORCE:MULTIPLE""
# target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
 target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-SAFESEH:NO>")

 if (SSL_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${SSL_LIBRARIES})
 endif (SSL_SUPPORT)
endif ()
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${ZLIB_LIBRARIES})
# target_link_libraries (${EXECUTABLE} ${PKG_Z_LIBRARIES})
# if (SSL_SUPPORT)
#  target_link_libraries (${EXECUTABLE} ${PKG_SSL_LIBRARIES})
# endif (SSL_SUPPORT)
# target_link_libraries (${EXECUTABLE} ${PKG_FFMPEG_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_XML2_LIBRARIES})

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/libuno_sal.so)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libgcc3_uno.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/libuno_cppu.so)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libstorelo.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libreglo.so)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libxmlreaderlo.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libunoidllo.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/libuno_cppuhelpergcc3.so)
 #target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/libuno_salhelpergcc3.so)
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
# target_link_libraries (${EXECUTABLE} ntdll.lib)
# target_link_libraries (${EXECUTABLE} dxguid.lib)
 target_link_libraries (${EXECUTABLE} mf.lib)
 target_link_libraries (${EXECUTABLE} mfplat.lib)
 set (LIB_FILE_SUFFIX "")
 if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
     CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set (LIB_FILE_SUFFIX "d")
 endif ()
 target_link_libraries (${EXECUTABLE} $ENV{LIB_ROOT}/zlib/cmake/${CMAKE_BUILD_TYPE}/zlib${LIB_FILE_SUFFIX}.lib)

# *TODO*: repair pkgconfig support
 #target_link_libraries (http_get ${LIBXML2_LIBRARY})
 target_link_libraries (${EXECUTABLE} $ENV{LIB_ROOT}/libxml2/lib/libxml2.lib)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/isal.lib)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/icppu.lib)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/icppuhelper.lib)
endif ()
#target_link_libraries (${EXECUTABLE} ${MYSQL_LIBRARY})

if (WIN32)
  # *WARNING*: linking Common requires "/FORCE:MULTIPLE"
# set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif (WIN32)
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonImage)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonSignal)
target_link_libraries (${EXECUTABLE} CommonTimer)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ACEStream_Device)
target_link_libraries (${EXECUTABLE} ACEStream_Document)
target_link_libraries (${EXECUTABLE} ACEStream_HTML)
target_link_libraries (${EXECUTABLE} ACEStream_MediaFramework)
target_link_libraries (${EXECUTABLE} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} ACEStream_Network)
target_link_libraries (${EXECUTABLE} ACEStream_Network_Protocol_HTTPGet)
target_link_libraries (${EXECUTABLE} ACEStream_Statistic)

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
                         ARGS "-c./etc/soffice.ini -f./etc/symbols.ini -h localhost -l -o ./output.ods -p 2083 -r -t -u https://www.tagesschau.de/wirtschaft/boersenkurse/suche/"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/http_get_2
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_LIBRARY_PATH)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libiconv/bin ICONV_LIBRARY_PATH)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libmysql/lib LIBMYSQL_LIBRARY_PATH)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libxml2/bin LIBXML_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/zlib/cmake/Debug ZLIB_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${ACE_LIBRARY_PATH}
                           ${FFMPEG_LIBRARY_PATH}
#                           ${ICONV_LIBRARY_PATH}
 #                          ${LIBXML_LIBRARY_PATH}
  #                         ${LIBMYSQL_LIBRARY_PATH}
                           ${SSL_LIBRARY_PATH}
                           ${ZLIB_LIBRARY_PATH})
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c.\\\\etc\\\\soffice.ini -f.\\\\etc\\\\template.ods -h localhost -i.\\\\etc\\\\symbols.ini -l -o .\\\\output.ods -p 2083 -r -t -u https://www.tagesschau.de/wirtschaft/boersenkurse/suche/"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/doc/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/html/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/net/protocols/http/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACENetwork/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../ACENetwork/cmake/src/protocol/http/Debug
                                              \"${OO_APP_DIRECTORY}\"
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/http_get_2
                         ENVIRONMENT foo=bar;_NO_DEBUG_HEAP=1)
endif ()
