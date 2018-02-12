 # *NOTE*: implies GTK_SUPPORT on
CMAKE_DEPENDENT_OPTION (GTKGL_SUPPORT "enable GTK OpenGL support" ON
                        "GTK_SUPPORT" OFF)
# *IMPORTANT NOTE*: to use gtkglarea on gtk2, check out the 'gtkglarea-2' branch
#                   of the project
#option (GTKGLAREA_SUPPORT "enable GtkGLArea support" OFF)
CMAKE_DEPENDENT_OPTION (GTKGLAREA_SUPPORT "enable GtkGLArea support" OFF
                        "GTK_SUPPORT; GTKGL_SUPPORT" OFF)

# Threads
if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif ()

# ACE
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
 find_library (ACE_LIBRARY
               NAMES ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 #message (STATUS "Found ACE library \"${ACE_LIBRARY}\"")
endif ()
add_definitions (-DACE_HAS_DLL)

# OpenGL
if (GTK_SUPPORT)
 if (GTKGL_SUPPORT)
  include (FindOpenGL)
  if (NOT OPENGL_FOUND)
   set (OPENGL_LIB_FILE libGL.so)
   if (UNIX)
# find_package (OPENGL REQUIRED)
    find_library (OPENGL_LIBRARY
                  NAMES ${OPENGL_LIB_FILE}
                  PATHS /usr/lib/x86_64-linux-gnu
                  DOC "searching for ${OPENGL_LIB_FILE}")
   endif () # UNIX
   if (NOT OPENGL_FOUND AND NOT OPENGL_LIBRARY)
    message (FATAL_ERROR "could not find ${OPENGL_LIB_FILE} (was: \"${OPENGL_LIBRARY}\"), aborting")
   else ()
    message (STATUS "Found OpenGL library \"${OPENGL_LIBRARY}\"")
   endif () # NOT OPENGL_FOUND AND NOT OPENGL_LIBRARY
  endif () # OPENGL_FOUND
 endif () # GTKGL_SUPPORT
endif () # GTK_SUPPORT

# pkg-config
include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_LIBALSA REQUIRED alsa)
 pkg_check_modules (PKG_SOX REQUIRED sox)
# pkg_check_modules (PKG_LIBPNG REQUIRED libpng)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil libavformat libswscale)
 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
   pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
  else ()
   pkg_check_modules (PKG_GTK2 REQUIRED gmodule-2.0 gthread-2.0 gtk+-2.0)
  endif ()
  if (GTKGL_SUPPORT)
   pkg_check_modules (PKG_OPENGL REQUIRED gl glu)
   if (GTK3_SUPPORT)
    if (GTKGLAREA_SUPPORT)
#     pkg_check_modules (PKG_GTKGLAREA REQUIRED gtkglarea)
    else ()
#     pkg_check_modules (PKG_GTKGL REQUIRED gtkgl-3.0)
    endif ()
   else ()
    if (GTKGLAREA_SUPPORT)
#    pkg_check_modules (PKG_GTKGLAREA REQUIRED gtkglarea)
    else ()
     pkg_check_modules (PKG_GTKGL REQUIRED gtkgl-2.0)
#     pkg_check_modules (PKG_GTKGL REQUIRED gdkglext-1.0 gtkglext-1.0)
    endif ()
   endif ()
  endif ()
 endif ()
elseif (WIN32)
# *TODO*: repair WIN32 pkgconfig support
endif ()

if (GTK_SUPPORT)
 if (GTKGL_SUPPORT)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS GTKGL_SUPPORT)
  add_definitions (-DGTKGL_SUPPORT)
  if (GTKGLAREA_SUPPORT)
   add_definitions (-DGTKGLAREA_SUPPORT)
  endif (GTKGLAREA_SUPPORT) # GTKGLAREA_SUPPORT

  set (GLUT_LIB_FILE libglut.so.3)
  if (UNIX)
   find_library (GLUT_LIBRARY ${GLUT_LIB_FILE}
                 HINTS /usr/lib
                 PATH_SUFFIXES x86_64-linux-gnu
                 DOC "searching for ${GLUT_LIB_FILE} (system paths)")
  elseif (WIN32)
   unset (LIB_FILE_SUFFIX)
   set (GLUT_LIB_FILE freeglut)
   if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (LIB_FILE_SUFFIX d)
   endif ()
   set (GLUT_LIB_FILE ${GLUT_LIB_FILE}${LIB_FILE_SUFFIX}.lib)
#  message (STATUS "${GLUT_LIB_FILE} search path: \"${CMAKE_CURRENT_SOURCE_DIR}/../../../FreeGLUT/freeglut/freeglut/cmake/lib/${CMAKE_BUILD_TYPE}\"")
   find_library (GLUT_LIBRARY ${GLUT_LIB_FILE}
                 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../FreeGLUT/freeglut/freeglut/cmake/lib/${CMAKE_BUILD_TYPE}
                 DOC "searching for ${GLUT_LIB_FILE}")
  endif () # UNIX
#if (NOT EXISTS GLUT_LIBRARY)
  if (GLUT_LIBRARY)
   message (STATUS "Found Glut library \"${GLUT_LIBRARY}\"")
  else ()
   message (FATAL_ERROR "could not find ${GLUT_LIB_FILE} (was: \"${GLUT_LIBRARY}\"), aborting")
  endif (GLUT_LIBRARY) # GLUT_LIBRARY
 endif (GTKGL_SUPPORT) # GTKGL_SUPPORT
endif (GTK_SUPPORT) # GTK_SUPPORT

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

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
if (GTK_SUPPORT)
 add_definitions (-DCOMMON_UI_HAS_DLL)
 add_definitions (-DCOMMON_UI_GTK_HAS_DLL)
 if (GTKGL_SUPPORT)
  add_definitions (-DCOMMON_GL_HAS_DLL)
 endif ()
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_HAS_DLL)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_ALSA_INCLUDE_DIRS})
# include_directories (${PKG_LIBPNG_INCLUDE_DIRS})
 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
   include_directories (${PKG_GTK3_INCLUDE_DIRS})
  else ()
   include_directories (${PKG_GTK2_INCLUDE_DIRS})
  endif () # GTK3_SUPPORT
  if (GTKGL_SUPPORT)
   include_directories (${PKG_OPENGL_INCLUDE_DIRS})
   include_directories (${PKG_GTKGL_INCLUDE_DIRS})
   if (GTKGLAREA_SUPPORT)
    include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea)
   endif () # GTKGLAREA_SUPPORT
  endif () # GTKGL_SUPPORT
 endif () # GTK_SUPPORT

 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/gl)
  endif ()
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/timer)
 if (GTK_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui/gtk)
 endif ()
elseif (WIN32)
# glm
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../glm)
  endif ()
 endif ()

 # *NOTE*: workaround
 set(_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

# DirectShow
 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})

 # assimp
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../assimp/include)

# libav
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg)

# gtk
 if (GTK_SUPPORT)
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
  if (GTKGL_SUPPORT)
   if (GTKGLAREA_SUPPORT)
    include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea)
   endif ()
  endif ()
 endif ()

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/3rd_party)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/gl)
  endif ()
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/timer)
 if (GTK_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui/gtk)
 endif ()
endif () # UNIX
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/image)

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
set (audioeffect_EXE_SRCS
     stdafx.cpp
     stdafx.h
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

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
set (EXECUTABLE audioeffect)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${audioeffect_EXE_SRCS}
                etc/${EXECUTABLE}.gtk2
                etc/${EXECUTABLE}.gtk3)
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
if (WIN32)
 add_dependencies (${EXECUTABLE} CommonError)
endif ()
add_dependencies (${EXECUTABLE} CommonMath)
if (GTK_SUPPORT)
 add_dependencies (${EXECUTABLE} CommonUI)
 add_dependencies (${EXECUTABLE} CommonUI_GTK)
endif ()

add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACEStream_Decoder)
add_dependencies (${EXECUTABLE} ACEStream_Device)
add_dependencies (${EXECUTABLE} ACEStream_Visualization)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} X11)
# target_link_libraries (${EXECUTABLE} ${PKG_LIBALSA_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBAV_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBPNG_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_SOX_LIBRARIES})
 if (GTK_SUPPORT)
  if (GTK3_SUPPORT)
   target_link_libraries (${EXECUTABLE} ${PKG_GTK3_LIBRARIES})
  else ()
   target_link_libraries (${EXECUTABLE} ${PKG_GTK2_LIBRARIES})
  endif ()
  if (GTKGL_SUPPORT)
   target_link_libraries (${EXECUTABLE} ${PKG_OPENGL_LIBRARIES})
   if (GTK3_SUPPORT)
    if (GTKGLAREA_SUPPORT)
     target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea/gtkgl/.libs/libgtkgl-3.0.so)
    else ()
     target_link_libraries (${EXECUTABLE} ${PKG_GTKGL_LIBRARIES})
    endif ()
   else ()
    if (GTKGLAREA_SUPPORT)
     target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea/gtkgl/.libs/libgtkgl-2.0.so)
    else ()
     target_link_libraries (${EXECUTABLE} ${PKG_GTKGL_LIBRARIES})
    endif ()
   endif ()
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} mfplat.lib)
 target_link_libraries (${EXECUTABLE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE} Dmoguids.lib)
 target_link_libraries (${EXECUTABLE} Msdmo.lib)
 target_link_libraries (${EXECUTABLE} strmiids.lib)
# OpenGL
 target_link_libraries (${EXECUTABLE} OpenGL32.lib)
 target_link_libraries (${EXECUTABLE} GlU32.lib)

# *TODO*: repair WIN32 pkgconfig support
# gtk
 if (GTK_SUPPORT)
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
#  target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtkglarea/gtkgl/.libs/libgtkglarea-win32-1.0.dll.a)
  endif ()
 endif (GTK_SUPPORT)

 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavformat/avformat.lib)

 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../assimp/cmake/code/${CMAKE_BUILD_TYPE}/assimp-vc140-mt.lib)
endif ()
target_link_libraries (${EXECUTABLE} ${GLUT_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)
if (GTK_SUPPORT)
 if (GTKGL_SUPPORT)
  target_link_libraries (${EXECUTABLE} CommonOpenGL)
 endif ()
endif ()
target_link_libraries (${EXECUTABLE} CommonImage)
target_link_libraries (${EXECUTABLE} CommonMath)
target_link_libraries (${EXECUTABLE} CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
if (GTK_SUPPORT)
 #target_link_libraries (${EXECUTABLE} CommonUI)
 target_link_libraries (${EXECUTABLE} CommonUI_GTK)
endif ()

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ACEStream_Device)
target_link_libraries (${EXECUTABLE} ACEStream_File)
target_link_libraries (${EXECUTABLE} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} ACEStream_Statistic)
target_link_libraries (${EXECUTABLE} ACEStream_Visualization)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/${EXECUTABLE}.gtk2 etc/${EXECUTABLE}.gtk3
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 install (FILES etc/${EXECUTABLE}.gtk2 etc/${EXECUTABLE}.gtk3
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

cmake_policy (SET CMP0053 OLD)
if (UNIX)
 if (GTK3_SUPPORT)
  create_target_launcher (${EXECUTABLE}
                          ARGS "-g./etc/audioeffect.gtk3 -l -t"
#                         FORWARD_ARGS
                          RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
#                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/gtk
                                               ${PROJECT_SOURCE_DIR}/cmake/src
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev
#                                              /usr/local/lib
                          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/audioeffect
                          ENVIRONMENT GDK_BACKEND=x11)
 else ()
  create_target_launcher (${EXECUTABLE}
                          ARGS "-g./etc/audioeffect.gtk2 -l -t"
#                         FORWARD_ARGS
                          RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
#                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/gtk
                                               ${PROJECT_SOURCE_DIR}/cmake/src
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev
#                                              /usr/local/lib
                          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/audioeffect
                          ENVIRONMENT GDK_BACKEND=x11)
 endif ()
elseif (WIN32)
 if (GTK3_SUPPORT)
  create_target_launcher (${EXECUTABLE}
                          ARGS "-g.\\\\etc\\\\audioeffect.gtk3 -l -t"
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
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/image/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/math/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/file/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/misc/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/stat/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/audioeffect
                          ENVIRONMENT foo=bar)
 else ()
  create_target_launcher (${EXECUTABLE}
                          ARGS "-g.\\\\etc\\\\audioeffect.gtk2 -l -t"
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
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/image/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/math/Debug
                                               ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/file/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/misc/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/stat/Debug
                                               ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/audioeffect
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
