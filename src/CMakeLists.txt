include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/scripts/Threads.cmake)

include (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/scripts/ACE.cmake)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

# ........................................

add_library (${PROJECT_NAME} STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${PKG_FFMPEG_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)

 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/cmake>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
elseif (WIN32)
 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME} PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME} PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/cmake>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/error>)
 target_include_directories (${PROJECT_NAME} PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../Common/src/timer>)
endif ()
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/tree>)
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>)
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules/lib>)
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules/misc>)
target_include_directories (${PROJECT_NAME} PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules/stat>)

############## next target ################
set (source_files
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
     stream_head_task.cpp
     stream_head_task.h
     stream_head_task.inl
     stream_headmoduletask_base.cpp
     stream_headmoduletask_base.h
     stream_headmoduletask_base.inl
     stream_iallocator.h
     stream_ilayout.h
     stream_ilink.h
     stream_ilock.h
     stream_imessage.h
     stream_imessagequeue.h
     stream_imodule.h
     stream_inotify.h
     stream_isessionnotify.h
     stream_istreamcontrol.h
     stream_itask.h
     stream_layout.cpp
     stream_layout.h
     stream_layout.inl
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
     stream_statistic.h
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
if (WIN32)
 set (source_files
      ../include/${PROJECT_NAME}_config.h
      ${source_files})
endif (WIN32)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

foreach (source_file ${source_files})
get_filename_component (abs_source_file ${source_file} ABSOLUTE)
list (APPEND absolute_source_files
      PRIVATE ${abs_source_file}
     )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

if (UNIX)
 if(CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")
#  set (CMAKE_STATIC_LINKER_FLAGS "-Wl,--as-needed")
 endif (CMAKE_COMPILER_IS_GNUCXX)
endif (UNIX)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${PROJECT_NAME} Common)

set_target_properties (${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL STREAM_BUILD_DLL)
set_target_properties (${PROJECT_NAME} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

if (WIN32)
 if (MSVC)
#  message (STATUS "MSVC_VERSION: ${MSVC_VERSION}")
  if (MSVC_VERSION GREATER 1600)
#  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
   target_link_libraries (${PROJECT_NAME} vcruntime${LIB_FILE_SUFFIX}.lib ucrt${LIB_FILE_SUFFIX}.lib)
  endif ()
 endif (MSVC)
# endif ()

 target_link_libraries (${PROJECT_NAME} ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME} Common)
target_link_libraries (${PROJECT_NAME} ${PROJECT_NAME}_Miscellaneous)

# add PCH
set_target_properties (${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME})

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
         stream_head_task.h
         stream_head_task.inl
         stream_headmoduletask_base.h
         stream_headmoduletask_base.inl
         stream_iallocator.h
         stream_ilayout.h
         stream_ilink.h
         stream_ilock.h
         stream_imessage.h
         stream_imessagequeue.h
         stream_imodule.h
         stream_inotify.h
         stream_isessionnotify.h
         stream_istreamcontrol.h
         stream_itask.h
         stream_layout.h
         stream_layout.inl
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
         stream_session_base.h
         stream_session_base.inl
         stream_session_data.h
         stream_session_data.inl
         stream_session_message_base.h
         stream_session_message_base.inl
         stream_statistic.h
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
if (WIN32)
 install (FILES
          ../include/lib${PROJECT_NAME}_config.h
         DESTINATION ${INSTALL_INCLUDE_DIR})
endif (WIN32)
install (TARGETS ${PROJECT_NAME}
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT lib
         LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT lib
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT lib)

##########################################

add_subdirectory (modules)
