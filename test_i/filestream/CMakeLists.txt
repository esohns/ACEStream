option (GTK3_SUPPORT "compile Gtk3 support"
        ON)

if (UNIX)
 include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
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
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 #message (STATUS "found ACE library \"${ACE_LIBRARY}\"")
endif ()
add_definitions (-DACE_HAS_DLL)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 if (GTK3_SUPPORT)
  pkg_check_modules (PKG_GTK3 REQUIRED gtk+-3.0)
 else ()
# pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)
  pkg_check_modules (PKG_GTK2 REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
 endif ()
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
#message (STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
#message (STATUS "BUILD_PATH: ${BUILD_PATH}")
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libCommon/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)
add_definitions (-DCOMMON_UI_HAS_DLL)

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
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 if (GTK3_SUPPORT)
  include_directories (${PKG_GTK3_INCLUDE_DIRS})
 else ()
  include_directories (${PKG_GTK2_INCLUDE_DIRS})
 endif ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui/gtk)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
else ()
# *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/include)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE)
 include_directories ($ENV{ACE_ROOT})

# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/include/gtk-3.0)
# libglade
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)

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
set (filesource_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_filestream_common.h
     test_i_filestream_defines.h
     test_i_filestream_network.h
     test_i_message.cpp
     test_i_message.h
     test_i_message.inl
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
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
     test_i_target_stream.h)
#  test_i_template_instantiations.cpp

set_source_files_properties (etc/resources.rc PROPERTIES HEADER_FILE_ONLY TRUE)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
set (EXECUTABLE_SOURCE filesource)
add_executable (${EXECUTABLE_SOURCE}
                EXCLUDE_FROM_ALL
                ${filesource_EXE_SRCS}
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

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_SOURCE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} rt)
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_CAP_LIBRARIES})
# target_link_libraries (${EXECUTABLE_SOURCE} iw)
 target_link_libraries (${EXECUTABLE_SOURCE} X11)
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_SOURCE} ntdll.lib)

# target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/lib/libeay32.lib)
# target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../openssl/lib/ssleay32.lib)

# *TODO*: repair WIN32 pkgconfig support
# gtk
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/pango-1.0.lib)
# libglade
# target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/lib/libglade-2.0.dll.a)
endif ()
target_link_libraries (${EXECUTABLE_SOURCE} Common)
target_link_libraries (${EXECUTABLE_SOURCE} CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
#target_link_libraries (${EXECUTABLE_SOURCE} CommonUI)
target_link_libraries (${EXECUTABLE_SOURCE} CommonUI_GTK)

target_link_libraries (${EXECUTABLE_SOURCE} ACEStream)
target_link_libraries (${EXECUTABLE_SOURCE} ACEStream_File)
target_link_libraries (${EXECUTABLE_SOURCE} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} ACEStream_Network)
target_link_libraries (${EXECUTABLE_SOURCE} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork)
#target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE_SOURCE})

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
          DESTINATION "data/${PROJECT_NAME}/filestream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/filestream" COMPONENT config)
elseif (UNIX)
 install (FILES etc/source.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/filestream" COMPONENT config)
endif ()

#guess_runtime_library_dirs (<outputvarname> [<extralibrary> ...])
if (UNIX)
 create_target_launcher (${EXECUTABLE_SOURCE}
#                         ARGS "-l -t"
                         ARGS "-f ~/Downloads/keyvalue-0.3.tgz -g -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
#                                              ${PROJECT_SOURCE_DIR}/../gtk3/bin
#                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/filestream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE_SOURCE}
                         ARGS "-f D:\\\\temp\\\\H264_720p.ts -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/filestream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME filestream_source
          COMMAND ${EXECUTABLE_SOURCE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (filestream_source
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")

############## next target ################
set (filetarget_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_filestream_common.h
     test_i_filestream_defines.h
     test_i_filestream_network.h
     test_i_message.cpp
     test_i_message.h
     test_i_message.inl
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
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
     test_i_target_stream.h)
#  test_i_template_instantiations.cpp

set (EXECUTABLE_TARGET filetarget)
add_executable (${EXECUTABLE_TARGET}
                EXCLUDE_FROM_ALL
                ${filetarget_EXE_SRCS}
                etc/resources.rc
                etc/target.glade)
set_property (TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "test_i")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_TARGET} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} rt)
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_CAP_LIBRARIES})
# target_link_libraries (${EXECUTABLE_TARGET} iw)
 target_link_libraries (${EXECUTABLE_TARGET} X11)
 if (GTK3_SUPPORT)
  target_link_libraries (${EXECUTABLE_TARGET} ${PKG_GTK3_LIBRARIES})
 else ()
  target_link_libraries (${EXECUTABLE_TARGET} ${PKG_GTK2_LIBRARIES})
 endif ()
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_TARGET} ntdll.lib)
# *TODO*: repair WIN32 pkgconfig support
# gtk
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/gtk/Win32/debug/lib/pango-1.0.lib)
# libglade
# target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/lib/libglade-2.0.dll.a)
endif ()
target_link_libraries (${EXECUTABLE_TARGET} Common)
target_link_libraries (${EXECUTABLE_TARGET} CommonTimer)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE"
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
#target_link_libraries (${EXECUTABLE_TARGET} CommonUI)
target_link_libraries (${EXECUTABLE_TARGET} CommonUI_GTK)

target_link_libraries (${EXECUTABLE_TARGET} ACEStream)
target_link_libraries (${EXECUTABLE_TARGET} ACEStream_File)
target_link_libraries (${EXECUTABLE_TARGET} ACEStream_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} ACEStream_Network)
target_link_libraries (${EXECUTABLE_TARGET} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE_TARGET} ACENetwork)
#target_link_libraries (${EXECUTABLE_TARGET} ACENetwork_Client)

# add PCH
#set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#get_target_property (_prefixHeader ${EXECUTABLE_SOURCE} COTIRE_CXX_PREFIX_HEADER)
#set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${_prefixHeader}")
set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE_TARGET})

install (TARGETS ${EXECUTABLE_TARGET}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT test_i)
if (UNIX)
 install (FILES etc/target.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/filestream" COMPONENT config)
elseif (WIN32)
 install (FILES etc/target.glade
          DESTINATION "data/${PROJECT_NAME}/filestream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/filestream" COMPONENT config)
endif ()
#guess_runtime_library_dirs (<outputvarname> [<extralibrary> ...])
if (UNIX)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
#                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
#                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/filestream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/filestream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE_TARGET}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE_TARGET} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE_TARGET}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
