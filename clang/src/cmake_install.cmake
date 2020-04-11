# Install script for directory: /mnt/win_d/projects/ACEStream/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xincludex" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/ACEStream/stream_allocatorbase.h;/usr/local/include/ACEStream/stream_allocatorbase.inl;/usr/local/include/ACEStream/stream_allocatorheap.h;/usr/local/include/ACEStream/stream_allocatorheap.inl;/usr/local/include/ACEStream/stream_base.h;/usr/local/include/ACEStream/stream_base.inl;/usr/local/include/ACEStream/stream_cachedmessageallocatorheap_base.h;/usr/local/include/ACEStream/stream_cachedmessageallocatorheap_base.inl;/usr/local/include/ACEStream/stream_cachedmessageallocatorheap.h;/usr/local/include/ACEStream/stream_cachedmessageallocatorheap.inl;/usr/local/include/ACEStream/stream_cacheddatablockallocatorheap.h;/usr/local/include/ACEStream/stream_cacheddatablockallocatorheap.inl;/usr/local/include/ACEStream/stream_cachedallocatorheap.h;/usr/local/include/ACEStream/stream_cachedallocatorheap.inl;/usr/local/include/ACEStream/stream_common.h;/usr/local/include/ACEStream/stream_configuration.h;/usr/local/include/ACEStream/stream_configuration.inl;/usr/local/include/ACEStream/stream_control_message.h;/usr/local/include/ACEStream/stream_control_message.inl;/usr/local/include/ACEStream/stream_data_base.h;/usr/local/include/ACEStream/stream_data_base.inl;/usr/local/include/ACEStream/stream_data_message_base.h;/usr/local/include/ACEStream/stream_data_message_base.inl;/usr/local/include/ACEStream/stream_datablockallocatorheap.h;/usr/local/include/ACEStream/stream_datablockallocatorheap.inl;/usr/local/include/ACEStream/stream_defines.h;/usr/local/include/ACEStream/stream_head_task.h;/usr/local/include/ACEStream/stream_head_task.inl;/usr/local/include/ACEStream/stream_headmoduletask_base.h;/usr/local/include/ACEStream/stream_headmoduletask_base.inl;/usr/local/include/ACEStream/stream_iallocator.h;/usr/local/include/ACEStream/stream_ilayout.h;/usr/local/include/ACEStream/stream_ilink.h;/usr/local/include/ACEStream/stream_ilock.h;/usr/local/include/ACEStream/stream_imessage.h;/usr/local/include/ACEStream/stream_imessagequeue.h;/usr/local/include/ACEStream/stream_imodule.h;/usr/local/include/ACEStream/stream_inotify.h;/usr/local/include/ACEStream/stream_isessionnotify.h;/usr/local/include/ACEStream/stream_istreamcontrol.h;/usr/local/include/ACEStream/stream_itask.h;/usr/local/include/ACEStream/stream_layout.h;/usr/local/include/ACEStream/stream_layout.inl;/usr/local/include/ACEStream/stream_macros.h;/usr/local/include/ACEStream/stream_message_base.h;/usr/local/include/ACEStream/stream_message_base.inl;/usr/local/include/ACEStream/stream_messageallocatorheap_base.h;/usr/local/include/ACEStream/stream_messageallocatorheap_base.inl;/usr/local/include/ACEStream/stream_messagequeue.h;/usr/local/include/ACEStream/stream_messagequeue.inl;/usr/local/include/ACEStream/stream_messagequeue_base.h;/usr/local/include/ACEStream/stream_messagequeue_base.inl;/usr/local/include/ACEStream/stream_module_base.h;/usr/local/include/ACEStream/stream_module_base.inl;/usr/local/include/ACEStream/stream_session_base.h;/usr/local/include/ACEStream/stream_session_base.inl;/usr/local/include/ACEStream/stream_session_data.h;/usr/local/include/ACEStream/stream_session_data.inl;/usr/local/include/ACEStream/stream_session_message_base.h;/usr/local/include/ACEStream/stream_session_message_base.inl;/usr/local/include/ACEStream/stream_statistic.h;/usr/local/include/ACEStream/stream_statemachine_control.h;/usr/local/include/ACEStream/stream_statemachine_control.inl;/usr/local/include/ACEStream/stream_streammodule_base.h;/usr/local/include/ACEStream/stream_streammodule_base.inl;/usr/local/include/ACEStream/stream_task_base.h;/usr/local/include/ACEStream/stream_task_base.inl;/usr/local/include/ACEStream/stream_task_base_asynch.h;/usr/local/include/ACEStream/stream_task_base_asynch.inl;/usr/local/include/ACEStream/stream_task_base_synch.h;/usr/local/include/ACEStream/stream_task_base_synch.inl;/usr/local/include/ACEStream/stream_tools.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/ACEStream" TYPE FILE FILES
    "/mnt/win_d/projects/ACEStream/src/stream_allocatorbase.h"
    "/mnt/win_d/projects/ACEStream/src/stream_allocatorbase.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_allocatorheap.h"
    "/mnt/win_d/projects/ACEStream/src/stream_allocatorheap.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_cachedmessageallocatorheap_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_cachedmessageallocatorheap_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_cachedmessageallocatorheap.h"
    "/mnt/win_d/projects/ACEStream/src/stream_cachedmessageallocatorheap.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_cacheddatablockallocatorheap.h"
    "/mnt/win_d/projects/ACEStream/src/stream_cacheddatablockallocatorheap.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_cachedallocatorheap.h"
    "/mnt/win_d/projects/ACEStream/src/stream_cachedallocatorheap.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_common.h"
    "/mnt/win_d/projects/ACEStream/src/stream_configuration.h"
    "/mnt/win_d/projects/ACEStream/src/stream_configuration.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_control_message.h"
    "/mnt/win_d/projects/ACEStream/src/stream_control_message.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_data_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_data_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_data_message_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_data_message_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_datablockallocatorheap.h"
    "/mnt/win_d/projects/ACEStream/src/stream_datablockallocatorheap.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_defines.h"
    "/mnt/win_d/projects/ACEStream/src/stream_head_task.h"
    "/mnt/win_d/projects/ACEStream/src/stream_head_task.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_headmoduletask_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_headmoduletask_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_iallocator.h"
    "/mnt/win_d/projects/ACEStream/src/stream_ilayout.h"
    "/mnt/win_d/projects/ACEStream/src/stream_ilink.h"
    "/mnt/win_d/projects/ACEStream/src/stream_ilock.h"
    "/mnt/win_d/projects/ACEStream/src/stream_imessage.h"
    "/mnt/win_d/projects/ACEStream/src/stream_imessagequeue.h"
    "/mnt/win_d/projects/ACEStream/src/stream_imodule.h"
    "/mnt/win_d/projects/ACEStream/src/stream_inotify.h"
    "/mnt/win_d/projects/ACEStream/src/stream_isessionnotify.h"
    "/mnt/win_d/projects/ACEStream/src/stream_istreamcontrol.h"
    "/mnt/win_d/projects/ACEStream/src/stream_itask.h"
    "/mnt/win_d/projects/ACEStream/src/stream_layout.h"
    "/mnt/win_d/projects/ACEStream/src/stream_layout.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_macros.h"
    "/mnt/win_d/projects/ACEStream/src/stream_message_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_message_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_messageallocatorheap_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_messageallocatorheap_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_messagequeue.h"
    "/mnt/win_d/projects/ACEStream/src/stream_messagequeue.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_messagequeue_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_messagequeue_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_module_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_module_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_session_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_session_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_session_data.h"
    "/mnt/win_d/projects/ACEStream/src/stream_session_data.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_session_message_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_session_message_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_statistic.h"
    "/mnt/win_d/projects/ACEStream/src/stream_statemachine_control.h"
    "/mnt/win_d/projects/ACEStream/src/stream_statemachine_control.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_streammodule_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_streammodule_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_task_base.h"
    "/mnt/win_d/projects/ACEStream/src/stream_task_base.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_task_base_asynch.h"
    "/mnt/win_d/projects/ACEStream/src/stream_task_base_asynch.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_task_base_synch.h"
    "/mnt/win_d/projects/ACEStream/src/stream_task_base_synch.inl"
    "/mnt/win_d/projects/ACEStream/src/stream_tools.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libACEStream.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/mnt/win_d/projects/ACEStream/clang/src/libACEStream.a")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/win_d/projects/ACEStream/clang/src/modules/cmake_install.cmake")

endif()

