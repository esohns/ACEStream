option (GTK3_SUPPORT "compile Gtk3 support"
        ON)

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
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()
add_definitions (-DACE_HAS_DLL)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L2 REQUIRED libv4l2)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavformat libavcodec libavutil)
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
endif ()
# *TODO*: repair WIN32 pkgconfig support

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
add_definitions (-DSTREAM_HAS_DLL)

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)
#if (WIN32)
# if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#  add_definitions (-DDEBUG_DEBUGGER)
# endif ()
#endif ()

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_V4L2_INCLUDE_DIRS})
 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
# *TODO*: repair pkgconfig support
 include_directories ("$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
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

# libav
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE camsave)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.cpp
     stdafx.h
     ../test_u_common.h
     ../test_u_defines.h
     targetver.h
     test_u_camsave.cpp
     test_u_camsave_callbacks.cpp
     test_u_camsave_callbacks.h
     test_u_camsave_common.h
     test_u_camsave_common_modules.h
     test_u_camsave_defines.h
     test_u_camsave_eventhandler.cpp
     test_u_camsave_eventhandler.h
     test_u_camsave_message.cpp
     test_u_camsave_message.h
     test_u_camsave_module_eventhandler.cpp
     test_u_camsave_module_eventhandler.h
     test_u_camsave_session_message.cpp
     test_u_camsave_session_message.h
     test_u_camsave_signalhandler.cpp
     test_u_camsave_signalhandler.h
     test_u_camsave_stream.cpp
     test_u_camsave_stream.h)

add_executable (${EXECUTABLE}
               EXCLUDE_FROM_ALL
                ${camsave_EXE_SRCS}
                etc/${EXECUTABLE}.glade)
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
if (WIN32 AND NOT ${BUILD_TEST_U})
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)
#add_dependencies (${PROJECT_NAME} ACEStream)
#add_dependencies (${PROJECT_NAME} ACEStream_Decoder)
#add_dependencies (${PROJECT_NAME} ACEStream_Device)
#add_dependencies (${PROJECT_NAME} ACEStream_Miscellaneous)
#add_dependencies (${PROJECT_NAME} ACEStream_Visualization)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} X11)
 target_link_libraries (${EXECUTABLE} ${PKG_V4L2_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_FFMPEG_LIBRARIES})
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} dxguid.lib)
 target_link_libraries (${EXECUTABLE} evr.lib)
 target_link_libraries (${EXECUTABLE} mfplat.lib)
 target_link_libraries (${EXECUTABLE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE} strmiids.lib)
# *TODO*: repair WIN32 pkgconfig support
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk_pixbuf-2.0.lib)
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

 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavformat/avformat.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavcodec/avcodec.lib)
endif ()
target_link_libraries (${EXECUTABLE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE"
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ACEStream_Device)
target_link_libraries (${EXECUTABLE} ACEStream_Visualization)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/${EXECUTABLE}.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 install (FILES ${ACE_DLL_FQPATH}
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libatk-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/freetype6.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/intl.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libcairo-2.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libfontconfig-1.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgio-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libglib-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgmodule-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgobject-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgthread-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgdk_pixbuf-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgdk-win32-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgtk-win32-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpango-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangocairo-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangoft2-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangowin32-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpng14-14.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/zlib1.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/bin/libglade-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.1/.libs/libxml2-2.dll
                G:/software/Development/dll/libgcc_s_dw2-1.dll
                G:/software/Development/dll/libexpat-1.dll
                G:/software/Development/dll/libiconv-2.dll
                ${Common_DIR}/src/Debug/Common.dll
                ${Common_DIR}/src/ui/Debug/CommonUI.dll
                ${ACENetwork_DIR}/src/Debug/ACENetwork.dll
                ${ACENetwork_DIR}/src/protocol/http/Debug/ACENetwork_Protocol_HTTP.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libmysql_2/cmake/libmysql/Debug/libmysql.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
 install (FILES etc/${EXECUTABLE}.glade
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-g./etc/camsave.glade -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/camsave
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 if (GTK3_SUPPORT)
  create_target_launcher (${EXECUTABLE}
                          ARGS "-g.\\\\etc\\\\camsave.glade -l -t"
#                        FORWARD_ARGS
                          RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                               ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libavcodec
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libswresample
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libswscale
                                               G:/software/Development/dll
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/camsave
                          ENVIRONMENT foo=bar)
 else ()
  create_target_launcher (${EXECUTABLE}
                          ARGS "-g.\\\\etc\\\\camsave.glade -l -t"
#                        FORWARD_ARGS
                          RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                               ${PROJECT_SOURCE_DIR}/../gtk2/bin
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libavcodec
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libswresample
                                               ${PROJECT_SOURCE_DIR}/../ffmpeg/libswscale
                                               G:/software/Development/dll
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/camsave
                          ENVIRONMENT foo=bar)
 endif ()
endif ()

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
