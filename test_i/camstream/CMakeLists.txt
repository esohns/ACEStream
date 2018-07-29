option (GTK_SUPPORT "compile Gtk support" ON)
option (GTK3_SUPPORT "compile Gtk3 support" ON)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
 check_library_exists (iwlib iw_sockets_open "" HAVE_LIB_IW)
endif ()

set (LIB_FILE_SUFFIX)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 set (LIB_FILE_SUFFIX d)
endif ()
set (ACE_LIB_FILE libACE.so)
if (UNIX)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               HINTS ${CMAKE_CURRENT_BINARY_DIR}/../../../ATCD/ACE /usr/local
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib
               DOC "searching for ${ACE_LIB_FILE}"
               NO_DEFAULT_PATH)
elseif (WIN32)
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
 #message (STATUS "found ACE library \"${ACE_LIBRARY}\"")
endif ()
add_definitions (-DACE_HAS_DLL)

if (WIN32)
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "D:/projects/DShowBaseClasses")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
endif (WIN32)

include (FindPkgConfig)
if (UNIX)
 #pkg_check_modules (PKG_ACE REQUIRED ACE)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L2 REQUIRED libv4l2)
 pkg_check_modules (PKG_FFMPEG REQUIRED libswscale libavutil libavcodec)
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
endif ()

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

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_V4L2_INCLUDE_DIRS})
 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui/gtk)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
else ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)

# *TODO*: repair pkgconfig support
# include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
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

 include_directories ($ENV{ACE_ROOT})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui/gtk)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (camsource_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_camstream_common.h
     test_i_camstream_defines.h
     test_i_camstream_network.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_source.cpp
     test_i_source_common.h
     test_i_source_eventhandler.cpp
     test_i_source_eventhandler.h
     test_i_source_eventhandler.inl
     test_i_source_message.cpp
     test_i_source_message.h
     test_i_source_session_message.cpp
     test_i_source_session_message.h
     test_i_source_signalhandler.cpp
     test_i_source_signalhandler.h
     test_i_source_signalhandler.inl
     test_i_source_stream.cpp
     test_i_source_stream.h
     test_i_source_stream.inl
     test_i_stream_common.h
     test_i_target_message.cpp
     test_i_target_message.h
     test_i_target_session_message.cpp
     test_i_target_session_message.h
     test_i_target_stream.cpp
     test_i_target_stream.h)
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

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
set (EXECUTABLE_SOURCE camsource)
add_executable (${EXECUTABLE_SOURCE}
                EXCLUDE_FROM_ALL
                ${camsource_EXE_SRCS}
                etc/resources.rc
                etc/source.glade)
set_property (TARGET ${EXECUTABLE_SOURCE} PROPERTY FOLDER "test_i")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

#add_dependencies (${EXECUTABLE_SOURCE} ACE)
add_dependencies (${EXECUTABLE_SOURCE} Common)
add_dependencies (${EXECUTABLE_SOURCE} CommonUI)
add_dependencies (${EXECUTABLE_SOURCE} CommonUI_GTK)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_MediaFramework)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE_SOURCE} ACENetwork)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Network)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_SOURCE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} rt)
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_CAP_LIBRARIES})
 #target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_DBUS_LIBRARIES})
 #target_link_libraries (${EXECUTABLE_SOURCE} iw)
 target_link_libraries (${EXECUTABLE_SOURCE} X11)
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_V4L2_LIBRARIES})
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_FFMPEG_LIBRARIES})
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")

 target_link_libraries (${EXECUTABLE_SOURCE} evr.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} mfplat.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} strmiids.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} winmm.lib)
# *TODO*: repair WIN32 pkgconfig support
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk-3.0.lib)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gtk-3.0.lib)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/pango-1.0.lib)
 else ()
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgio-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libglib-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgobject-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgthread-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgdk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgdk_pixbuf-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgtk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libcairo.dll.a)
  target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libpango-1.0.dll.a)
 endif ()

# target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavcodec/avcodec.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavcodec/libavcodec.a)
endif ()
target_link_libraries (${EXECUTABLE_SOURCE} Common)
target_link_libraries (${EXECUTABLE_SOURCE} CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
#target_link_libraries (${EXECUTABLE_SOURCE} CommonUI)
target_link_libraries (${EXECUTABLE_SOURCE} CommonUI_GTK)

target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork)
#target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)

set (RESOURCE_FILES
     etc/resources.rc
     etc/source.glade
     etc/target.glade)
set_source_files_properties (${RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group (resources FILES ${RESOURCE_FILES})

install (TARGETS ${EXECUTABLE_SOURCE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT test_i)
if (WIN32)
 install (FILES etc/source.glade
          DESTINATION "data/${PROJECT_NAME}/camstream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/camstream" COMPONENT config)
elseif (UNIX)
 install (FILES etc/source.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/camstream" COMPONENT config)
endif ()

#guess_runtime_library_dirs (<outputvarname> [<extralibrary> ...])
if (UNIX)
 create_target_launcher (${EXECUTABLE_SOURCE}
#                         ARGS "-g./etc/source.glade -l -t"
                         ARGS "-g -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE_SOURCE}
#                         ARGS "-g.\\\\etc\\\\source.glade -l -t"
                         ARGS "-c -g -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavcodec
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavresample
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libswscale
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE_SOURCE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE_SOURCE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_SOURCE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\ffmpeg\libavcodec;$(ProjectDir)..\..\..\..\ffmpeg\libavresample;$(ProjectDir)..\..\..\..\ffmpeg\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\libACENetwork\cmake\src\Debug

############## next target ################

if (WIN32)
 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})

 set (camtarget_source_LIB_SRCS
      stdafx.cpp
      stdafx.h
      camtarget_source.cpp
      test_i_target_message.cpp
      test_i_target_message.h)

 set (LIBRARY_DS camtarget_source)
 add_library (${LIBRARY_DS} MODULE
              EXCLUDE_FROM_ALL
              ${camtarget_source_LIB_SRCS}
              etc/camtarget_source.def)
 set_property (TARGET ${LIBRARY_DS} PROPERTY FOLDER "test_i")

 add_dependencies (${LIBRARY_DS} Common)
 add_dependencies (${LIBRARY_DS} ${PROJECT_NAME})

 set (CMAKE_MODULE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")

 target_link_libraries (${LIBRARY_DS} ${ACE_LIBRARY})
 target_link_libraries (${LIBRARY_DS} strmiids.lib)
 target_link_libraries (${LIBRARY_DS} winmm.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${LIBRARY_DS} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Debug/strmbasd.lib)
 else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${LIBRARY_DS} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Release/strmbase.lib)
 endif ()
 target_link_libraries (${LIBRARY_DS} Common)

 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_MediaFramework)
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_Miscellaneous)

 set_target_properties (${LIBRARY_DS} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_DS} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

 # add PCH
 set_target_properties (${LIBRARY_DS} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_DS}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_DS}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 create_target_launcher (${LIBRARY_DS}
                         ARGS "${PROJECT_BINARY_DIR}/test_i/camstream/Debug/${LIBRARY_DS}.dll"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavcodec
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavresample
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
#                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
#                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)

#-----------------------------------------

 set (camtarget_source_mf_LIB_SRCS
      stdafx.cpp
      stdafx.h
      camtarget_source_mf.cpp)

 set (LIBRARY_MF camtarget_source_mf)
 add_library (${LIBRARY_MF} MODULE
              EXCLUDE_FROM_ALL
              ${camtarget_source_mf_LIB_SRCS}
              etc/camtarget_source_mf.def)
 set_property (TARGET ${LIBRARY_MF} PROPERTY FOLDER "test_i")

 add_dependencies (${LIBRARY_MF} Common)
 add_dependencies (${LIBRARY_MF} ${PROJECT_NAME})

 target_link_libraries (${LIBRARY_MF} ${ACE_LIBRARY})

 target_link_libraries (${LIBRARY_MF} Common)

 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_MediaFramework)
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Miscellaneous)

 set_target_properties (${LIBRARY_MF} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_MF} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

 # add PCH
 set_target_properties (${LIBRARY_MF} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_MF}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_MF}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)
endif ()

############## next target ################

#if (UNIX)
 set (camtarget_EXE_SRCS
      stdafx.cpp
      stdafx.h
      test_i_callbacks.cpp
      test_i_callbacks.h
      test_i_camstream_common.h
      test_i_camstream_defines.h
      test_i_camstream_network.h
      test_i_common_modules.h
      test_i_connection_manager_common.h
      test_i_module_eventhandler.cpp
      test_i_module_eventhandler.h
      test_i_module_eventhandler.inl
      test_i_source_stream.cpp
      test_i_source_stream.h
      test_i_source_stream.inl
      test_i_target.cpp
      test_i_target_common.h
      test_i_target_eventhandler.cpp
      test_i_target_eventhandler.h
      test_i_target_eventhandler.inl
      test_i_target_listener_common.h
      test_i_target_message.cpp
      test_i_target_message.h
      test_i_target_session_message.cpp
      test_i_target_session_message.h
      test_i_target_signalhandler.cpp
      test_i_target_signalhandler.h
      test_i_target_signalhandler.inl
      test_i_target_stream.cpp
      test_i_target_stream.h)
#  test_i_template_instantiations.cpp

set (EXECUTABLE_TARGET camtarget)
add_executable (${EXECUTABLE_TARGET}
                EXCLUDE_FROM_ALL
                ${camtarget_EXE_SRCS}
                etc/resources.rc
                etc/target.glade)
set_property (TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "test_i")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

#add_dependencies (${EXECUTABLE_TARGET} ACE)
add_dependencies (${EXECUTABLE_TARGET} Common)
add_dependencies (${EXECUTABLE_TARGET} CommonUI)
add_dependencies (${EXECUTABLE_TARGET} CommonUI_GTK)

add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Miscellaneous)

add_dependencies (${EXECUTABLE_TARGET} ACENetwork)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Network)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_TARGET} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} rt)
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_CAP_LIBRARIES})
 #target_link_libraries (${EXECUTABLE_TARGET} ${PKG_DBUS_LIBRARIES})
# target_link_libraries (${EXECUTABLE_TARGET} iw)
 target_link_libraries (${EXECUTABLE_TARGET} X11)
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_V4L2_LIBRARIES})
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_FFMPEG_LIBRARIES})
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_TARGET} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE_TARGET} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_TARGET} evr.lib)
 target_link_libraries (${EXECUTABLE_TARGET} strmiids.lib)
 target_link_libraries (${EXECUTABLE_TARGET} winmm.lib)
# *TODO*: repair WIN32 pkgconfig support
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk-3.0.lib)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gtk-3.0.lib)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/pango-1.0.lib)
 else ()
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgio-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libglib-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgobject-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgthread-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgdk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgdk_pixbuf-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libgtk-win32-2.0.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libcairo.dll.a)
  target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/libpango-1.0.dll.a)
 endif ()

# target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavcodec/avcodec.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavcodec/libavcodec.a)
endif ()

target_link_libraries (${EXECUTABLE_TARGET} Common)
target_link_libraries (${EXECUTABLE_TARGET} CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
#target_link_libraries (${EXECUTABLE_TARGET} CommonUI)
target_link_libraries (${EXECUTABLE_TARGET} CommonUI_GTK)

target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Device)
if (WIN32)
 target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_MediaFramework)
endif (WIN32)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_TARGET} ACENetwork)
#target_link_libraries (${EXECUTABLE_TARGET} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#cotire (${EXECUTABLE_SOURCE} ${LIBRARY_DS} ${LIBRARY_MF} ${EXECUTABLE_TARGET})
#cotire (${EXECUTABLE_TARGET})

install (TARGETS ${EXECUTABLE_TARGET}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (UNIX)
 install (FILES etc/target.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/camstream" COMPONENT config)
elseif (WIN32)
 install (FILES etc/target.glade
          DESTINATION "data/${PROJECT_NAME}/camstream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/camstream" COMPONENT config)
endif ()

if (UNIX)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-g./etc/target.glade -l -o -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-g.\\\\etc\\\\target.glade -l -o -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavcodec
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavresample
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
                                              ${PROJECT_SOURCE_DIR}/../ffmpeg/libswscale
#                                              D:/projects/libxml2-2.9.1/.libs
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dec/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE_TARGET}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_TARGET}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\ffmpeg\libavcodec;$(ProjectDir)..\..\..\..\ffmpeg\libavresample;$(ProjectDir)..\..\..\..\ffmpeg\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\libACENetwork\cmake\src\Debug
