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
                PATH_SUFFIXES build/linux/lib
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
               PATH_SUFFIXES lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}"
               NO_DEFAULT_PATH)
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
endif ()

include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)
pkg_check_modules (PKG_LIBXML2 REQUIRED libxml-2.0)

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

#string (TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
if (UNIX)
 find_library (ACENETWORK_LIB
               ACENetwork
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork ${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork
#               PATH_SUFFIXES build/${BUILD_TYPE}/src/.libs
               PATH_SUFFIXES cmake/src
               NO_DEFAULT_PATH)
 if (NOT EXISTS ${ACENETWORK_LIB})
  message (FATAL_ERROR "could not find library \"ACENetwork\", aborting")
 endif ()
elseif (WIN32)
# *TODO*: avoid circular dependency
 find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
               REQUIRED
               CONFIG
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
               PATH_SUFFIXES libACENetwork/cmake
               NO_DEFAULT_PATH)
 if (NOT EXISTS ${ACENetwork_DIR})
  message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
 endif ()
 set (ACENETWORK_LIB libACENetwork.lib)
endif ()

if (UNIX)
 find_library (ACENETWORK_CLIENT_LIB
               ACENetwork_Client
               PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork ${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork
#               PATH_SUFFIXES build/${BUILD_TYPE}/src/client_server/.libs
               PATH_SUFFIXES cmake/src/client_server
               NO_DEFAULT_PATH)
 if (NOT EXISTS ${ACENETWORK_CLIENT_LIB})
  message (FATAL_ERROR "could not find library \"ACENetwork_Client\", aborting")
 endif ()
elseif (WIN32)
#find_package (ACENetwork_Client ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
#              PATH_SUFFIXES libNetwork/cmake
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${ACENetwork_Client_DIR})
# message (FATAL_ERROR "could not find package \"ACENetwork_Client\", aborting")
#endif ()
 set (ACENETWORK_CLIENT_LIB libACENetwork_Client.lib)
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ATCD/ACE)
 include_directories (${PKG_LIBXML2_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
else ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
# include_directories (${PKG_LIBXML2_INCLUDE_DIRS})
# *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libiconv-1.9.1/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.1/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/include/libglade-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/html)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net/protocols)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (http_get_EXE_SRCS
     test_i_common.h
     test_i_common_modules.h
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_defines.h
     test_i_message.cpp
     test_i_message.h
     test_i_module_htmlparser.cpp
     test_i_module_htmlparser.h
     test_i_module_htmlwriter.cpp
     test_i_module_htmlwriter.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_source.cpp
     test_i_source_common.h
     test_i_source_signalhandler.cpp
     test_i_source_signalhandler.h
     test_i_source_stream.cpp
     test_i_source_stream.h
     test_i_source_stream.inl
#  test_i_template_instantiations.cpp
     stdafx.cpp
     stdafx.h)

add_executable (http_get ${http_get_EXE_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (http_get ${CMAKE_THREAD_LIBS_INIT})
# *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
set_target_properties (http_get PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
target_link_libraries (http_get ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (http_get ${PKG_GTK_GLADE_LIBRARIES})
elseif (WIN32)
# *TODO*: repair pkgconfig support
 #target_link_libraries (http_get ${LIBXML2_LIBRARY})
 target_link_libraries (http_get ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.1/.libs/libxml2.dll.a)
endif ()
target_link_libraries (http_get Common)
target_link_libraries (http_get ACEStream)
target_link_libraries (http_get ACEStream_HTML)
target_link_libraries (http_get ACENetwork)
#target_link_libraries (http_get ACENetwork_Client)

install (TARGETS http_get
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
