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

include (FindPkgConfig)
if (UNIX)
 #pkg_check_modules (PKG_ACE REQUIRED ACE)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
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

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
else ()
# *TODO*: repair pkgconfig support
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE)
 include_directories ($ENV{ACE_ROOT})

# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/cairo)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/pango-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/include/gtk-3.0)
# libglade
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/include/libglade-2.0)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (filesource_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_common_modules.h
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_defines.h
     test_i_filestream_common.h
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

set (EXECUTABLE filesource)
add_executable (${EXECUTABLE} ${filesource_EXE_SRCS} etc/resources.rc etc/source.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_GTK_GLADE_LIBRARIES})
elseif (WIN32)
# *TODO*: repair WIN32 pkgconfig support
# gtk
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/pango-1.0.lib)
# libglade
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/lib/libglade-2.0.dll.a)
endif ()
target_link_libraries (${EXECUTABLE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACENetwork)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#cotire (${EXECUTABLE})

set (RESOURCE_FILES
     etc/resources.rc
     etc/source.glade
     etc/target.glade)
set_source_files_properties (${RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group (resources FILES ${RESOURCE_FILES})

install (TARGETS ${EXECUTABLE}
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
 create_target_launcher (${EXECUTABLE}
#                         ARGS "-g./../../../test_i/filestream/etc/source.glade -l -t"
                         ARGS "-f ~/Downloads/keyvalue-0.3.tgz -g -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk3/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-g.\\\\..\\\\..\\\\..\\\\test_i\\\\filestream\\\\etc\\\\source.glade -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk3/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME filestream_source
          COMMAND ${EXECUTABLE} -l -t
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
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_defines.h
     test_i_filestream_common.h
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

set (EXECUTABLE filetarget)
add_executable (${EXECUTABLE} ${filetarget_EXE_SRCS} etc/resources.rc etc/target.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_GTK_GLADE_LIBRARIES})
elseif (WIN32)
# *TODO*: repair WIN32 pkgconfig support
# gtk
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gthread-2.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gdk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/gtk-3.0.lib)
 target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk-win32/gtk/Win32/debug/lib/pango-1.0.lib)
# libglade
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade-2.6.4/lib/libglade-2.0.dll.a)
endif ()
target_link_libraries (${EXECUTABLE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE"
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} CommonUI)
target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACENetwork)
#target_link_libraries (${EXECUTABLE} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
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
 create_target_launcher (${EXECUTABLE}
                         ARGS "-g./../../../test_i/filestream/etc/target.glade -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk-win32/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE}
                         ARGS "-g.\\\\..\\\\..\\\\..\\\\test_i\\\\filestream\\\\etc\\\\target.glade -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk-win32/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
