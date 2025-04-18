if (UNIX)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/ui/scripts")
 include (PreUIFramework)
 include (SetUIFramework)
 if (GTK_SUPPORT) # *NOTE*: _SUPPORT, not _USE !
  include (SetGTKVersion)
 endif (GTK_SUPPORT)
endif (UNIX)

add_library (${PROJECT_NAME}_Miscellaneous STATIC "")

if (UNIX)
 if (FFMPEG_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                              $<BUILD_INTERFACE:${ffmpeg_INCLUDE_DIRS}>)
 endif (FFMPEG_SUPPORT)
 if (GTK_SUPPORT)
  target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                              "$<BUILD_INTERFACE:${GTK_INCLUDE_DIRS}>")
  if (GTKGL_SUPPORT)
   if (GTKGLAREA_SUPPORT)
# *NOTE*: check out the 'master' branch for gtk3-based applications
# *NOTE*: check out the 'gtkglarea-2' branch for gtk2-based applications
# *TODO*: verify automatically
    target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                                $<BUILD_INTERFACE:$ENV{LIB_ROOT}/gtkglarea>)
   endif (GTKGLAREA_SUPPORT)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif (UNIX)

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/event>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/ui/gtk>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/event>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/math>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
endif ()
target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../file>)

############## next target ################
set (source_files
     stream_misc_aggregator.cpp
     stream_misc_aggregator.h
     stream_misc_aggregator.inl
     stream_misc_asynch.cpp
     stream_misc_asynch.h
     stream_misc_asynch.inl
     stream_misc_common.h
     stream_misc_defines.h
     stream_misc_defragment.cpp
     stream_misc_defragment.h
     stream_misc_defragment.inl
     stream_misc_delay.cpp
     stream_misc_delay.h
     stream_misc_delay.inl
     stream_misc_distributor.cpp
     stream_misc_distributor.h
     stream_misc_distributor.inl
     stream_misc_dump.cpp
     stream_misc_dump.h
     stream_misc_dump.inl
     stream_misc_injector.cpp
     stream_misc_injector.h
     stream_misc_injector.inl
     stream_misc_input_manager.cpp
     stream_misc_input_manager.h
     stream_misc_input_manager.inl
     stream_misc_input_stream.cpp
     stream_misc_input_stream.h
     stream_misc_input_stream.inl
     stream_misc_media_splitter.cpp
     stream_misc_media_splitter.h
     stream_misc_media_splitter.inl
     stream_misc_messagehandler.cpp
     stream_misc_messagehandler.h
     stream_misc_messagehandler.inl
     stream_misc_parser.cpp
     stream_misc_parser.h
     stream_misc_parser.inl
     stream_misc_queue_source.cpp
     stream_misc_queue_source.h
     stream_misc_queue_source.inl
     stream_misc_queue_target.cpp
     stream_misc_queue_target.h
     stream_misc_queue_target.inl
     stream_misc_splitter.cpp
     stream_misc_splitter.h
     stream_misc_splitter.inl
     stream_misc_window_source.cpp
     stream_misc_window_source.h
     stream_misc_window_source.inl)

# Include dir for installed targets
#set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
      )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Miscellaneous
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Miscellaneous
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Miscellaneous PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Miscellaneous Common)
add_dependencies (${PROJECT_NAME}_Miscellaneous ${PROJECT_NAME})

if (UNIX)
 if (FFMPEG_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Miscellaneous PUBLIC ${ffmpeg_LIBRARIES})
 endif (FFMPEG_SUPPORT)
endif (UNIX)

target_link_libraries (${PROJECT_NAME}_Miscellaneous PUBLIC ${ACE_LIBRARY})

target_link_libraries (${PROJECT_NAME}_Miscellaneous PUBLIC Common)

target_link_libraries (${PROJECT_NAME}_Miscellaneous PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES DEFINE_SYMBOL STREAM_MISC_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Miscellaneous
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_misc_aggregator.h
         stream_misc_aggregator.inl
         stream_misc_asynch.h
         stream_misc_asynch.inl
         stream_misc_common.h
         stream_misc_defines.h
         stream_misc_defragment.h
         stream_misc_defragment.inl
         stream_misc_delay.h
         stream_misc_delay.inl
         stream_misc_distributor.h
         stream_misc_distributor.inl
         stream_misc_dump.h
         stream_misc_dump.inl
         stream_misc_injector.h
         stream_misc_injector.inl
         stream_misc_input_manager.h
         stream_misc_input_manager.inl
         stream_misc_input_stream.h
         stream_misc_input_stream.inl
         stream_misc_media_splitter.h
         stream_misc_media_splitter.inl
         stream_misc_messagehandler.h
         stream_misc_messagehandler.inl
         stream_misc_parser.h
         stream_misc_parser.inl
         stream_misc_queue_source.h
         stream_misc_queue_source.inl
         stream_misc_queue_target.h
         stream_misc_queue_target.inl
         stream_misc_splitter.h
         stream_misc_splitter.inl
         stream_misc_window_source.h
         stream_misc_window_source.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)

install (TARGETS ${PROJECT_NAME}_Miscellaneous
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
