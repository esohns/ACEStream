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
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 message (STATUS "found ACE library \"${ACE_LIBRARY}\"")
endif ()

if (WIN32)
 set (DIRECTSHOW_BASECLASSES_DIRECTORY "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
endif ()

include (FindPkgConfig)
if (UNIX)
 #pkg_check_modules (PKG_ACE REQUIRED ACE)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L2 REQUIRED libv4l2)
 pkg_check_modules (PKG_AVCODEC REQUIRED libavutil libavcodec)
 pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
endif ()

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
if (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
endif ()

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_V4L2_INCLUDE_DIRS})
 include_directories (${PKG_AVCODEC_INCLUDE_DIRS})
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
else ()
 include_directories ($ENV{ACE_ROOT})
# *TODO*: repair pkgconfig support
# include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
# gtk
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/glib-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/glib-2.0/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/gtk-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/lib/gtk-2.0/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/gdk-pixbuf-2.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/atk-1.0)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/cairo)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk2/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gtk-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/pango-1.0)
# libglade
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/include/libglade-2.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (camsource_EXE_SRCS
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_common.h
     test_i_common_modules.h
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_defines.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_source.cpp
     test_i_source_common.h
     test_i_source_eventhandler.cpp
     test_i_source_eventhandler.h
     test_i_source_signalhandler.cpp
     test_i_source_signalhandler.h
     test_i_source_stream.cpp
     test_i_source_stream.h
     test_i_source_stream.inl
     test_i_target_stream.cpp
     test_i_target_stream.h
#  test_i_template_instantiations.cpp
     stdafx.cpp
     stdafx.h)

set_source_files_properties (etc/resources.rc PROPERTIES HEADER_FILE_ONLY TRUE)

set (EXECUTABLE camsource)
add_executable (${EXECUTABLE} ${camsource_EXE_SRCS} etc/resources.rc etc/source.glade)

#add_dependencies (${EXECUTABLE} ACE)
add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACENetwork)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_V4L2_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_AVCODEC_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_GTK_GLADE_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} dxguid.lib)
 target_link_libraries (${EXECUTABLE} evr.lib)
 target_link_libraries (${EXECUTABLE} mfplat.lib)
 target_link_libraries (${EXECUTABLE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE} strmiids.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${EXECUTABLE} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${EXECUTABLE} strmbase.lib)
 endif ()
 target_link_libraries (${EXECUTABLE} winmm.lib)
# *TODO*: repair WIN32 pkgconfig support
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gio-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/glib-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gobject-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/pango-1.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gdk-win32-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gtk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/pango-1.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gdk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gtk-win32-2.0.lib)
endif ()
target_link_libraries (${EXECUTABLE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Decoder)
target_link_libraries (${EXECUTABLE} ACEStream_Device)
target_link_libraries (${EXECUTABLE} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE} ACENetwork)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

install (TARGETS ${EXECUTABLE}
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

add_test (NAME camstream_source
          COMMAND ${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (camstream_source
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")

#guess_runtime_library_dirs (<outputvarname> [<extralibrary> ...])
create_target_launcher (${EXECUTABLE}
                        ARGS "-g.\\\\test_i\\\\camstream\\\\etc\\\\source.glade -l -t"
                        #FORWARD_ARGS 
                        RUNTIME_LIBRARY_DIRS D:/projects/ATCD/ACE/lib
                                             D:/projects/gtk/bin
                                             D:/projects/libglade/bin
                                             D:/projects/libxml2-2.9.1/.libs
                                             G:/software/Development/dll
                                             ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                             ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                             ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                             ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                             ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        ENVIRONMENT foo=bar)

############## next target ################

if (WIN32)
 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})

 set (camtarget_source_LIB_SRCS
      camtarget_source.cpp
      stdafx.cpp
      stdafx.h)

 set (LIBRARY camtarget_source)
 add_library (${LIBRARY} MODULE ${camtarget_source_LIB_SRCS} scripts/camstream_source.def)

 add_dependencies (${LIBRARY} Common)
 add_dependencies (${LIBRARY} ${PROJECT_NAME})

 target_link_libraries (${LIBRARY} ${ACE_LIBRARY})
 target_link_libraries (${LIBRARY} strmiids.lib)
 target_link_libraries (${LIBRARY} winmm.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${LIBRARY} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${LIBRARY} strmbase.lib)
 endif ()
 target_link_libraries (${LIBRARY} Common)
 target_link_libraries (${LIBRARY} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY} ${PROJECT_NAME}_Miscellaneous)

 set_target_properties (${LIBRARY} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 set (camtarget_source_mf_LIB_SRCS
      camtarget_source_mf.cpp
      stdafx.cpp
      stdafx.h)

 set (LIBRARY camtarget_source_mf)
 add_library (${LIBRARY} MODULE ${camtarget_source_mf_LIB_SRCS} scripts/camstream_source_mf.def)

 add_dependencies (${LIBRARY} Common)
 add_dependencies (${LIBRARY} ${PROJECT_NAME})

 target_link_libraries (${LIBRARY} ${ACE_LIBRARY})
# target_link_libraries (${LIBRARY} strmiids.lib)
# target_link_libraries (${LIBRARY} winmm.lib)
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
#  target_link_libraries (${LIBRARY} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
# else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
#  target_link_libraries (${LIBRARY} strmbase.lib)
# endif ()
 target_link_libraries (${LIBRARY} Common)
 target_link_libraries (${LIBRARY} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY} ${PROJECT_NAME}_Miscellaneous)

 set_target_properties (${LIBRARY} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)
endif ()

############## next target ################

set (camtarget_EXE_SRCS
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_common.h
     test_i_common_modules.h
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_defines.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_source_stream.cpp
     test_i_source_stream.h
     test_i_source_stream.inl
     test_i_target.cpp
     test_i_target_common.h
     test_i_target_eventhandler.cpp
     test_i_target_eventhandler.h
     test_i_target_listener_common.h
     test_i_target_signalhandler.cpp
     test_i_target_signalhandler.h
     test_i_target_stream.cpp
     test_i_target_stream.h
#  test_i_template_instantiations.cpp
     stdafx.cpp
     stdafx.h)

set (EXECUTABLE camtarget)
add_executable (${EXECUTABLE} ${camtarget_EXE_SRCS} etc/resources.rc etc/target.glade)

#add_dependencies (${EXECUTABLE} ACE)
add_dependencies (${EXECUTABLE} Common)
add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACENetwork)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_V4L2_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_AVCODEC_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_GTK_GLADE_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} strmiids.lib)
 target_link_libraries (${EXECUTABLE} winmm.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${EXECUTABLE} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${EXECUTABLE} strmbase.lib)
 endif ()
# *TODO*: repair WIN32 pkgconfig support
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gio-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/glib-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gobject-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/pango-1.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gdk-win32-2.0.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/lib/gtk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/pango-1.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gdk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gtk-win32-2.0.lib)
endif ()
target_link_libraries (${EXECUTABLE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} ACENetwork)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

install (TARGETS ${EXECUTABLE}
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

create_target_launcher (${EXECUTABLE}
                        ARGS "-g.\\\\test_i\\\\camstream\\\\etc\\\\target.glade -l -t"
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS D:/projects/ATCD/ACE/lib
                                             D:/projects/gtk/bin
                                             D:/projects/libglade/bin
                                             D:/projects/libxml2-2.9.1/.libs
                                             G:/software/Development/dll
                                             ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                             ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                             ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                             ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                             ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        ENVIRONMENT foo=bar)

add_test (NAME camstream_target
          COMMAND ${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (camstream_target
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
