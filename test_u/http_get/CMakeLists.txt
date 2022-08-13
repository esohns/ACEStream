include (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/scripts/SSLConfig.cmake)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()
add_definitions (-DHTTP_HAS_DLL)

if (UNIX)
 if (SSL_SUPPORT)
  include_directories (${PKG_SSL_INCLUDE_DIRS})
 endif (SSL_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../ACENetwork/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../ACENetwork/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../ACENetwork/src/client_server)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../ACENetwork/src/protocol/http)
elseif (WIN32)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

# if (SSL_SUPPORT)
#  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl_win32_bin/include)
# endif (SSL_SUPPORT)

 # *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/http)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net/protocols/http)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (EXECUTABLE HTTP_GET)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     http_get.cpp
     http_get_common.h
     http_get_common_modules.h
     http_get_connection_manager_common.h
     http_get_defines.h
     http_get_eventhandler.cpp
     http_get_eventhandler.h
     http_get_message.cpp
     http_get_message.h
     http_get_module_eventhandler.cpp
     http_get_module_eventhandler.h
     http_get_network.h
     http_get_session_message.cpp
     http_get_session_message.h
     http_get_signalhandler.cpp
     http_get_signalhandler.h
     http_get_stream.cpp
     http_get_stream.h
     http_get_stream.inl
     http_get_stream_common.h)
#  test_i_template_instantiations.cpp
if (GUI_SUPPORT)
 if (GTK_USE)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       http_get_callbacks.cpp
       http_get_callbacks.h)
# elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE}_EXE_SRCS
#       ${${EXECUTABLE}_EXE_SRCS})
 endif (GTK_USE)
endif (GUI_SUPPORT)

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
endif (UNIX)

cmake_policy (SET CMP0060 OLD) # link libraries by full path even in implicit directories
cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                etc/httpget.glade)
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_U)
  set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)

# target_link_libraries (${EXECUTABLE} ${PKG_FFMPEG_LIBRARIES})
elseif (WIN32)
 if (MSVC)
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"libcmt.lib\"")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"libcpmt.lib\"")
 endif (MSVC)
# target_link_libraries (${EXECUTABLE} mfplat.lib)
# *TODO*: repair WIN32 pkgconfig support
# gtk
# set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /SAFESEH:NO)

 if (SSL_SUPPORT)
  target_link_libraries (${EXECUTABLE} $ENV{LIB_ROOT}/openssl/libcrypto.lib)
  target_link_libraries (${EXECUTABLE} $ENV{LIB_ROOT}/openssl/libssl.lib)
 endif (SSL_SUPPORT)
endif ()
if (UNIX)
# target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
endif (UNIX)
#target_link_libraries (${EXECUTABLE} ${MYSQL_LIBRARY})

target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} ${SSL_LIBRARIES})

 target_link_libraries (${EXECUTABLE} ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonEvent)
target_link_libraries (${EXECUTABLE} CommonImage)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonSignal)
target_link_libraries (${EXECUTABLE} CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE} CommonUI)
 if (GTK_SUPPORT)
  target_link_libraries (${EXECUTABLE} CommonUI_GTK)
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Network_Protocol_HTTPGet)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

target_link_libraries (${EXECUTABLE} ACENetwork)
target_link_libraries (${EXECUTABLE} ACENetwork_Protocol_HTTP)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

# add PCH
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${EXECUTABLE})
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 set (ACE_SSL_DLL_FILE ACE_SSL${LIB_FILE_SUFFIX}.dll)
 set (ACE_SSL_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_SSL_DLL_FILE})
 file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
 string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
 install (FILES ${ACE_DLL_FQPATH}
                ${ACE_SSL_DLL_FQPATH}
                ${LIB_ROOT}/openssl/libssl-3-x64.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
endif (WIN32)

#set_tests_properties (${EXECUTABLE} PROPERTIES ENVIRONMENT "PATH=%PATH%;$(ACE_ROOT)\\lib;D:\\projects\\libiconv-1.9.2\\bin;D:\\projects\\libxml2-2.9.4\\win32\\bin;$(ProgramFiles)\\LibreOffice 5\\program;$(ProjectDir)..\\..\\..\\..\\Common\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\Common\\cmake\\src\\err\\$(Configuration);$(ProjectDir)..\\..\\src\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\dec\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\html\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\ACENetwork\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\ACENetwork\\cmake\\src\\protocol\\http\\$(Configuration)")

set (ARGS "-l -r -t -u https://www.mirc.com/servers.ini")
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE}" COMMAND)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "$ENV{LIB_ROOT}/openssl" SSL_PATH)
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 set (RUNTIME_LIBRARY_DIRS ${ACE_LIBRARY_PATH}
                           ${FFMPEG_PATH}
                           ${SSL_PATH}
                           ${GTK_PATH}
                           ${WXWIDGETS_PATH})
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        ENVIRONMENT ${ENVIRONMENT})
