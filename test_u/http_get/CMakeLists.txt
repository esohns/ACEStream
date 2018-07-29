option (SSL_SUPPORT "enable SSL support" OFF)
option (GTK3_SUPPORT "enable Gtk3 support" ON)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
 check_library_exists (iwlib iw_sockets_open "" HAVE_LIB_IW)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               HINTS ${CMAKE_CURRENT_BINARY_DIR}/../../../ATCD/ACE
#               HINTS /usr/local
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib
               DOC "searching for ${ACE_LIB_FILE}"
               NO_DEFAULT_PATH)
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
 #message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()

if (SSL_SUPPORT)
 set (ACE_SSL_LIB_FILE libACE_SSL.so)
 if (UNIX)
  find_library (ACE_SSL_LIBRARY ${ACE_SSL_LIB_FILE}
                HINTS ${CMAKE_CURRENT_BINARY_DIR}/../../../ATCD/ACE
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES lib
                DOC "searching for ${ACE_SSL_LIB_FILE}"
                NO_DEFAULT_PATH)
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
  #message (STATUS "Found ACE SSL library \"${ACE_SSL_LIBRARY}\"")
 endif ()
endif ()

include (FindPkgConfig)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 #pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
 if (SSL_SUPPORT)
  pkg_check_modules (PKG_SSL REQUIRED libssl)
 endif ()
 pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil libavformat libswscale)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 gtkglext-1.0 libglade-2.0)
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 gtkglext-1.0)
 endif ()
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

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_DB_HAS_DLL)

#string (TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()
add_definitions (-DHTTP_HAS_DLL)

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 if (SSL_SUPPORT)
  include_directories (${PKG_SSL_INCLUDE_DIRS})
 endif ()

 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()

 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui/gtk)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACENetwork/src/client_server)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACENetwork/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libACENetwork/cmake)
elseif (WIN32)
 include_directories ($ENV{ACE_ROOT})

 if (SSL_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

 # *TODO*: repair pkgconfig support
# gtk
 if (GTK3_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/atk-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gdk-pixbuf-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/cairo)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/pango-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/glib-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gtk-3.0)
 else ()
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/atk-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/gdk-pixbuf-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/cairo)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/pango-1.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/glib-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/glib-2.0)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/gtk-2.0/include)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/gtk-2.0)
 endif ()

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/flex)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui/gtk)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/protocol/http)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/include)
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
     stdafx.cpp
     stdafx.h
     http_get.cpp
     http_get_callbacks.cpp
     http_get_callbacks.h
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

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
endif ()

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11)
 #add_compile_options (-fPIC)
 add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

cmake_policy (SET CMP0060 OLD) # link libraries by full path even in implicit directories
cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${HTTP_GET_EXE_SRCS}
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
# target_link_libraries (${EXECUTABLE} iw)
 target_link_libraries (${EXECUTABLE} X11)

 target_link_libraries (${EXECUTABLE} ${PKG_FFMPEG_LIBRARIES})

 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
# target_link_libraries (${EXECUTABLE} ntdll.lib)
 #target_link_libraries (${EXECUTABLE} dxguid.lib)
# target_link_libraries (${EXECUTABLE} evr.lib)
 target_link_libraries (${EXECUTABLE} mfplat.lib)
# target_link_libraries (${EXECUTABLE} mfuuid.lib)
# target_link_libraries (${EXECUTABLE} strmiids.lib)
# *TODO*: repair WIN32 pkgconfig support
# gtk
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk-3.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gtk-3.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/pango-1.0.lib)
 else ()
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgio-2.0.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libglib-2.0.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgobject-2.0.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgthread-2.0.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgdk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgdk_pixbuf-2.0.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgtk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libcairo.dll.a)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libpango-1.0.dll.a)
 endif ()

 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE"
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 #target_link_libraries (${EXECUTABLE} ${PKG_DBUS_LIBRARIES})
endif ()
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} ${ACE_SSL_LIBRARY})
endif ()
target_link_libraries (${EXECUTABLE} ${MYSQL_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonTimer)
#target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} CommonUI_GTK)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Device)
target_link_libraries (${EXECUTABLE} ACEStream_File)
target_link_libraries (${EXECUTABLE} ACEStream_MediaFramework)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
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

#set_tests_properties (${EXECUTABLE} PROPERTIES ENVIRONMENT "PATH=%PATH%;$(ACE_ROOT)\\lib;D:\\projects\\libiconv-1.9.2\\bin;D:\\projects\\libxml2-2.9.4\\win32\\bin;$(ProgramFiles)\\LibreOffice 5\\program;$(ProjectDir)..\\..\\..\\..\\libCommon\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\libCommon\\cmake\\src\\err\\$(Configuration);$(ProjectDir)..\\..\\src\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\dec\\$(Configuration);$(ProjectDir)..\\..\\src\\modules\\html\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\libACENetwork\\cmake\\src\\$(Configuration);$(ProjectDir)..\\..\\..\\..\\libACENetwork\\cmake\\src\\protocol\\http\\$(Configuration)")

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-l -t -u http://www.mirc.com/servers.ini"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/file/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/net/protocols/http/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/protocol/http/Debug
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/httpget
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 if (GTK3_SUPPORT)
  create_target_launcher (${EXECUTABLE}
                          ARGS "-l -t -u http://www.mirc.com/servers.ini"
#                        FORWARD_ARGS
                          RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
#                                              ${PROJECT_SOURCE_DIR}/../openssl/bin
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libswscale
                                               ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/file/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/net/protocols/http/Debug
                                               ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/protocol/http/Debug
                          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/httpget
                          ENVIRONMENT foo=bar)
 else ()
  create_target_launcher (${EXECUTABLE}
                          ARGS "-l -t -u http://www.mirc.com/servers.ini"
#                        FORWARD_ARGS
                          RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
#                                              ${PROJECT_SOURCE_DIR}/../openssl/bin
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libswscale
                                               ${PROJECT_SOURCE_DIR}/../gtk2/bin
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/file/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/net/protocols/http/Debug
                                               ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/protocol/http/Debug
                          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/httpget
                          ENVIRONMENT foo=bar)
 endif ()
endif ()
