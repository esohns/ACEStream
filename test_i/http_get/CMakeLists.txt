set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/xml/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/xml/scripts")
#include (PreXMLImplementation)
include (libxml2)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/db/scripts")
include (mysql)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/scripts")
include (SSLConfig)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

if (LIBXML2_SUPPORT)
 include_directories (${LIBXML2_INCLUDE_DIRS})
endif (LIBXML2_SUPPORT)
if (MYSQL_SUPPORT)
 include_directories (${MYSQL_INCLUDE_DIRS})
endif (MYSQL_SUPPORT)
if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/db)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/html)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net/protocols/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE HTTP_get_1)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
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
     test_i_http_get_defines.h
     test_i_http_get_signalhandler.h
     test_i_session_message.h
     test_i_http_get.cpp
     test_i_http_get_common.h
     test_i_http_get_network.h
     test_i_http_get_signalhandler.cpp
     test_i_http_get_stream.cpp
     test_i_http_get_stream.h
     test_i_http_get_stream.inl
     test_i_http_get_stream_common.h)
#  test_i_template_instantiations.cpp

cmake_policy (SET CMP0060 OLD) # link libraries by full path even in implicit directories
cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${HTTP_get_1_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (WIN32)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

if (LIBXML2_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBXML2_LIBRARIES})
endif (LIBXML2_SUPPORT)
if (MYSQL_SUPPORT)
 target_link_directories (${EXECUTABLE} PUBLIC ${MYSQL_LINK_DIRS})
 target_link_libraries (${EXECUTABLE} PUBLIC ${MYSQL_LIBRARIES})
endif (MYSQL_SUPPORT)
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SSL_LIBRARIES})
endif (SSL_SUPPORT)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
elseif (WIN32)
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
#if (SSL_SUPPORT)
# target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_SSL_LIBRARY})
#endif (SSL_SUPPORT)
if (UNIX)
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")

 target_link_libraries (${EXECUTABLE} PUBLIC mf.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Database)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_HTML)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ACENetwork)
target_link_libraries (${EXECUTABLE} PUBLIC ACENetwork_Protocol_HTTP)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${EXECUTABLE})
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
 string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
 install (FILES ${ACE_DLL_FQPATH}
                ${LIB_ROOT}/libmysql/lib/libmysql.dll
                ${LIB_ROOT}/openssl/libssl-3-x64.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
endif (WIN32)

#set_tests_properties (${EXECUTABLE} PROPERTIES ENVIRONMENT "PATH=%PATH%;$(ACE_ROOT)\\lib;D:\\projects\\libiconv-1.9.2\\bin;D:\\projects\\libxml2-2.9.4\\win32\\bin;$(ProgramFiles)\\LibreOffice 5\\program;$(ProjectDir)..\\..\\..\\..\\Common\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\Common\\cmake\\src\\err\\$(Configuration);$(ProjectDir)..\\..\\src\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\dec\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\html\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\ACENetwork\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\ACENetwork\\cmake\\src\\protocol\\http\\$(Configuration)")

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c./etc/options.cnf -d news -e articles -f output.txt -l -p 80 -t -u http://www.heise.de/newsticker.html"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.4/.libs
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/db/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/html/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/net/protocols/http/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/../ACENetwork/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/../ACENetwork/${BUILD_PATH_SUFFIX}/src/protocol/http/${CMAKE_BUILD_TYPE}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/http_get
                         ENVIRONMENT foo=bar)
elseif (WIN32)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libiconv/bin ICONV_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libmysql/lib LIBMYSQL_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libxml2/bin LIBXML_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_LIBRARY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/zlib/${BUILD_PATH_SUFFIX}/${CMAKE_BUILD_TYPE} ZLIB_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${ACE_LIBRARY_PATH}
                           ${FFMPEG_LIBRARY_PATH}
                           ${ICONV_LIBRARY_PATH}
                           ${LIBXML_LIBRARY_PATH}
                           ${LIBMYSQL_LIBRARY_PATH}
                           ${SSL_LIBRARY_PATH}
                           ${ZLIB_LIBRARY_PATH})
 create_target_launcher (${EXECUTABLE}
                         ARGS "-c.\\\\etc\\\\options.cnf -f output.txt -l -p 80 -r -t -u http://www.heise.de/newsticker.html"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
#                                              ${PROJECT_SOURCE_DIR}/../libiconv-1.9.2/bin
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/err/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/db/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/dec/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/html/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/modules/net/protocols/http/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/../ACENetwork/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/../ACENetwork/${BUILD_PATH_SUFFIX}/src/protocol/http/${CMAKE_BUILD_TYPE}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/http_get
                         ENVIRONMENT foo=bar)
endif ()
