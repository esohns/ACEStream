# target system introspection (see configure.ac)
include (CheckFunctionExists)
if (UNIX)
 check_function_exists (gettimeofday HAVE_FUNC_GETTIMEOFDAY)
endif ()
if (UNIX)
 check_function_exists (localtime_r HAVE_FUNC_LOCALTIME_R)
endif ()
check_function_exists (strerror HAVE_FUNC_STRERROR)
check_function_exists (tzset HAVE_FUNC_TZSET)

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
# *TODO*: this does not work
 if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
     CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_LIB_FILE)
 set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
#               PATH_SUFFIXES lib\\${CMAKE_CFG_INTDIR}\\Win32
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

#include (FindPkgConfig)
#pkg_check_modules (PKGACE REQUIRED ACE)

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

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/ACE)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../modules/libCommon/src)
else ()
 include_directories ($ENV{ACE_ROOT})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/stat)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/..)

############## next target ################
set (${PROJECT_NAME}_LIB_SRCS
     stdafx.cpp
     stdafx.h
     stream_allocatorbase.cpp
     stream_allocatorbase.h
     stream_allocatorbase.inl
     stream_allocatorheap.cpp
     stream_allocatorheap.h
     stream_allocatorheap.inl
     stream_base.cpp
     stream_base.h
     stream_base.inl
     stream_cachedmessageallocator.cpp
     stream_cachedmessageallocator.h
     stream_cachedmessageallocator.inl
     stream_cachedmessageallocatorheap_base.cpp
     stream_cachedmessageallocatorheap_base.h
     stream_cachedmessageallocatorheap_base.inl
#     stream_cachedmessageallocatorheap.cpp
#     stream_cachedmessageallocatorheap.h
#     stream_cachedmessageallocatorheap.inl
     stream_cacheddatablockallocatorheap.cpp
     stream_cacheddatablockallocatorheap.h
     stream_cacheddatablockallocatorheap.inl
     stream_cachedallocatorheap.cpp
     stream_cachedallocatorheap.h
     stream_cachedallocatorheap.inl
     stream_common.h
     stream_configuration.cpp
     stream_configuration.h
     stream_configuration.inl
     stream_control_message.cpp
     stream_control_message.h
     stream_control_message.inl
     stream_data_base.cpp
     stream_data_base.h
     stream_data_base.inl
     stream_data_message_base.cpp
     stream_data_message_base.h
     stream_data_message_base.inl
     stream_datablockallocatorheap.cpp
     stream_datablockallocatorheap.h
     stream_datablockallocatorheap.inl
     stream_defines.h
     stream_exports.h
     stream_head_task.cpp
     stream_head_task.h
     stream_head_task.inl
     stream_headmoduletask_base.cpp
     stream_headmoduletask_base.h
     stream_headmoduletask_base.inl
     stream_iallocator.h
     stream_ilink.h
     stream_ilock.h
     stream_imessage.h
     stream_imessagequeue.h
     stream_imodule.h
     stream_inotify.h
     stream_iparser.h
     stream_isessionnotify.h
     stream_istreamcontrol.h
     stream_itask.h
     stream_macros.h
     stream_message_base.cpp
     stream_message_base.h
     stream_message_base.inl
     stream_messageallocatorheap_base.cpp
     stream_messageallocatorheap_base.h
     stream_messageallocatorheap_base.inl
     stream_messagequeue.cpp
     stream_messagequeue.h
     stream_messagequeue.inl
     stream_messagequeue_base.cpp
     stream_messagequeue_base.h
     stream_messagequeue_base.inl
     stream_module_base.cpp
     stream_module_base.inl
     stream_module_base.h
     stream_resetcounterhandler.cpp
     stream_resetcounterhandler.h
     stream_session_base.cpp
     stream_session_base.h
     stream_session_base.inl
     stream_session_data.cpp
     stream_session_data.h
     stream_session_data.inl
     stream_session_message_base.cpp
     stream_session_message_base.h
     stream_session_message_base.inl
     stream_statemachine_control.cpp
     stream_statemachine_control.h
     stream_statemachine_control.inl
     stream_streammodule_base.cpp
     stream_streammodule_base.h
     stream_streammodule_base.inl
     stream_task_base.cpp
     stream_task_base.h
     stream_task_base.inl
     stream_task_base_asynch.cpp
     stream_task_base_asynch.h
     stream_task_base_asynch.inl
     stream_task_base_synch.cpp
     stream_task_base_synch.h
     stream_task_base_synch.inl
     stream_tools.cpp
     stream_tools.h)

add_library (${PROJECT_NAME} STATIC ${ACEStream_LIB_SRCS})

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME} Common)

set_target_properties (${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL STREAM_BUILD_DLL)
set_target_properties (${PROJECT_NAME} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

if (WIN32)
# if (MSVC)
#  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_link_libraries (${PROJECT_NAME} vcruntime${LIB_FILE_SUFFIX}.lib ucrt${LIB_FILE_SUFFIX}.lib)
#  endif ()
# endif ()
 target_link_libraries (${PROJECT_NAME} ${ACE_LIBRARY})
 target_link_libraries (${PROJECT_NAME} Common)
endif ()

# add PCH
set_target_properties (${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${PROJECT_NAME})

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}
        FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_allocatorbase.h
         stream_allocatorbase.inl
         stream_allocatorheap.h
         stream_allocatorheap.inl
         stream_base.h
         stream_base.inl
#         stream_cachedmessageallocator.h
#         stream_cachedmessageallocator.inl
         stream_cachedmessageallocatorheap_base.h
         stream_cachedmessageallocatorheap_base.inl
         stream_cachedmessageallocatorheap.h
         stream_cachedmessageallocatorheap.inl
         stream_cacheddatablockallocatorheap.h
         stream_cacheddatablockallocatorheap.inl
         stream_cachedallocatorheap.h
         stream_cachedallocatorheap.inl
         stream_common.h
         stream_configuration.h
         stream_configuration.inl
         stream_control_message.h
         stream_control_message.inl
         stream_data_base.h
         stream_data_base.inl
         stream_data_message_base.h
         stream_data_message_base.inl
         stream_datablockallocatorheap.h
         stream_datablockallocatorheap.inl
         stream_defines.h
         stream_exports.h
         stream_head_task.h
         stream_head_task.inl
         stream_headmoduletask_base.h
         stream_headmoduletask_base.inl
         stream_iallocator.h
         stream_ilink.h
         stream_ilock.h
         stream_imessage.h
         stream_imessagequeue.h
         stream_imodule.h
         stream_inotify.h
         stream_iparser.h
         stream_isessionnotify.h
         stream_istreamcontrol.h
         stream_itask.h
         stream_macros.h
         stream_message_base.h
         stream_message_base.inl
         stream_messageallocatorheap_base.h
         stream_messageallocatorheap_base.inl
         stream_messagequeue.h
         stream_messagequeue.inl
         stream_messagequeue_base.h
         stream_messagequeue_base.inl
         stream_module_base.h
         stream_module_base.inl
         stream_resetcounterhandler.h
         stream_session_base.h
         stream_session_base.inl
         stream_session_data.h
         stream_session_data.inl
         stream_session_message_base.h
         stream_session_message_base.inl
         stream_statemachine_control.h
         stream_statemachine_control.inl
         stream_streammodule_base.h
         stream_streammodule_base.inl
         stream_task_base.h
         stream_task_base.inl
         stream_task_base_asynch.h
         stream_task_base_asynch.inl
         stream_task_base_synch.h
         stream_task_base_synch.inl
         stream_tools.h
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT include)
install (TARGETS ${PROJECT_NAME}
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT lib
         LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT lib
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT lib)

add_subdirectory (modules)
