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
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
endif ()

#include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
pkg_check_modules (PKG_GTK_GLADE REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0 libglade-2.0)

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

# *TODO*: avoid circular dependency
#find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
#              PATH_SUFFIXES libNetwork/cmake
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${ACENetwork_DIR})
# message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
#endif ()
#string (TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
find_library (ACENETWORK_LIB
              ACENetwork
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork ${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork
#              PATH_SUFFIXES build/${BUILD_TYPE}/src/.libs
              PATH_SUFFIXES cmake/src
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENETWORK_LIB})
 message (FATAL_ERROR "could not find library \"ACENetwork\", aborting")
endif ()

#find_package (ACENetwork_Client ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
#              PATH_SUFFIXES libNetwork/cmake
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${ACENetwork_Client_DIR})
# message (FATAL_ERROR "could not find package \"ACENetwork_Client\", aborting")
#endif ()
find_library (ACENETWORK_CLIENT_LIB
              ACENetwork_Client
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork ${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork
#              PATH_SUFFIXES build/${BUILD_TYPE}/src/client_server/.libs
              PATH_SUFFIXES cmake/src/client_server
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENETWORK_CLIENT_LIB})
 message (FATAL_ERROR "could not find library \"ACENetwork_Client\", aborting")
endif ()

#find_package (ACENetwork_Server ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
#              PATH_SUFFIXES libNetwork/cmake
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${ACENetwork_Server_DIR})
# message (FATAL_ERROR "could not find package \"ACENetwork_Server\", aborting")
#endif ()
find_library (ACENETWORK_SERVER_LIB
              ACENetwork_Server
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork ${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork
#              PATH_SUFFIXES build/${BUILD_TYPE}/src/client_server/.libs
              PATH_SUFFIXES cmake/src/client_server
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENETWORK_SERVER_LIB})
 message (FATAL_ERROR "could not find library \"ACENetwork_Server\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ATCD/ACE)
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
else ()
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (filesource_EXE_SRCS
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

add_executable (filesource ${filesource_EXE_SRCS} etc/resources.rc etc/source.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (filesource ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (filesource ${ACE_LIBRARY})
target_link_libraries (filesource ${PKG_GTK_GLADE_LIBRARIES})
target_link_libraries (filesource Common)
target_link_libraries (filesource CommonUI)
target_link_libraries (filesource ACEStream)
#target_link_libraries (filesource ACENetwork)
#target_link_libraries (filesource ACENetwork_Client)
target_link_libraries (filesource ${ACENETWORK_LIB})
target_link_libraries (filesource ${ACENETWORK_CLIENT_LIB})

install (TARGETS filesource
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/source.glade etc/resources.rc
         DESTINATION etc/libACEStream/filestream COMPONENT config)

############## next target ################
set (filetarget_EXE_SRCS
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

add_executable (filetarget ${filetarget_EXE_SRCS} etc/resources.rc etc/target.glade)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (filetarget ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (filetarget ${ACE_LIBRARY})
target_link_libraries (filetarget ${PKG_GTK_GLADE_LIBRARIES})
target_link_libraries (filetarget Common)
target_link_libraries (filetarget CommonUI)
target_link_libraries (filetarget ACEStream)
#target_link_libraries (filetarget ACENetwork)
#target_link_libraries (filetarget ACENetwork_Client)
target_link_libraries (filetarget ${ACENETWORK_LIB})
target_link_libraries (filetarget ${ACENETWORK_SERVER_LIB})

install (TARGETS filetarget
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/target.glade etc/resources.rc
         DESTINATION etc/libACEStream/filestream COMPONENT config)
