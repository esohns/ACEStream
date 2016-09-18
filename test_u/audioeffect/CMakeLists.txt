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
 pkg_check_modules (PKG_LIBALSA REQUIRED alsa)
 pkg_check_modules (PKG_LIBAV REQUIRED libavformat libavcodec)
 pkg_check_modules (PKG_LIBSOX REQUIRED sox)
 pkg_check_modules (PKG_GTK REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
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

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_LIBALSA_INCLUDE_DIRS})
 include_directories (${PKG_GTK_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
elseif (WIN32)
 # *NOTE*: workaround
 set(_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

# DirectShow
 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})

# libav
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libav-11.7)

# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gtk-3.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (audioeffect_EXE_SRCS
     stdafx.cpp
     stdafx.h
     ../test_u_common.h
     ../test_u_defines.h
     targetver.h
     test_u_audioeffect.cpp
     test_u_audioeffect_callbacks.cpp
     test_u_audioeffect_callbacks.h
     test_u_audioeffect_common.h
     test_u_audioeffect_common_modules.h
     test_u_audioeffect_defines.h
     test_u_audioeffect_eventhandler.cpp
     test_u_audioeffect_eventhandler.h
     test_u_audioeffect_message.cpp
     test_u_audioeffect_message.h
     test_u_audioeffect_module_eventhandler.cpp
     test_u_audioeffect_module_eventhandler.h
     test_u_audioeffect_session_message.cpp
     test_u_audioeffect_session_message.h
     test_u_audioeffect_signalhandler.cpp
     test_u_audioeffect_signalhandler.h
     test_u_audioeffect_stream.cpp
     test_u_audioeffect_stream.h)

set (EXECUTABLE audioeffect)
add_executable (${EXECUTABLE} ${audioeffect_EXE_SRCS} etc/audioeffect.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${EXECUTABLE} ACEStream_Decoder)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
# target_link_libraries (${EXECUTABLE} ${PKG_LIBALSA_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBAV_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBSOX_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_GTK_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} mfplat.lib)
 target_link_libraries (${EXECUTABLE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE} strmiids.lib)
# *TODO*: repair WIN32 pkgconfig support
# gtk
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/pango-1.0.lib)
endif ()
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonMath)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
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
 install (FILES etc/${EXECUTABLE}.glade
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

if (UNIX)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-g./etc/audioeffect.glade -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk-win32/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/audioeffect
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-g.\\\\etc\\\\audioeffect.glade -l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk-win32/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavcodec
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavresample
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavutil
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/math/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/audioeffect
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug