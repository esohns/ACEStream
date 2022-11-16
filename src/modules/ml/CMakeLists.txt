include (tensorflow)

add_library (${PROJECT_NAME}_MachineLearning STATIC "")

if (TENSORFLOW_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${tensorflow_INCLUDE_DIRS}>)
endif (TENSORFLOW_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
endif ()
target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)

############## next target ################
set (source_files
     stream_module_ml_common.h
     stream_module_ml_defines.h
     stream_module_ml_tools.cpp
     stream_module_ml_tools.h)
if (TENSORFLOW_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_tensorflow.cpp
      stream_module_tensorflow.h
      stream_module_tensorflow.inl)
endif (TENSORFLOW_SUPPORT)

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
target_sources (${PROJECT_NAME}_MachineLearning
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
# add PCH
target_precompile_headers (${PROJECT_NAME}_MachineLearning
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_MachineLearning PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_MachineLearning Common)
add_dependencies (${PROJECT_NAME}_MachineLearning ${PROJECT_NAME})

#if (TENSORFLOW_SUPPORT)
# target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC ${tensorflow_LIBRARIES})
#endif (TENSORFLOW_SUPPORT)
if (WIN32)
 if (MSVC)
  if (MSVC_VERSION GREATER 1600)
#   target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC ucrt${LIB_FILE_SUFFIX}.lib)
#   target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC vcruntime${LIB_FILE_SUFFIX}.lib)
  endif (MSVC_VERSION GREATER 1600)
 endif (MSVC)
# target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_MachineLearning PROPERTIES DEFINE_SYMBOL STREAM_DB_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_MachineLearning PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_MachineLearning
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (source_files
#     stream_db_exports.h
     stream_module_ml_common.h
     stream_module_ml_defines.h
     stream_module_ml_tools.h)
if (TENSORFLOW_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_tensorflow.h
      stream_module_tensorflow.inl)
endif (TENSORFLOW_SUPPORT)
install (FILES
         ${source_files}
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_MachineLearning
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
