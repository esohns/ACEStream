include (tensorflow)
include (libtorch)
include (llama)

add_library (${PROJECT_NAME}_MachineLearning STATIC "")

if (WIN32)
# set (CMAKE_CXX_STANDARD 17)
 if (MSVC)
  target_compile_options (${PROJECT_NAME}_MachineLearning PUBLIC "/std:c++17")
 endif (MSVC)
endif (WIN32)

if (TENSORFLOW_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${tensorflow_INCLUDE_DIRS}>)
endif (TENSORFLOW_SUPPORT)
if (TENSORFLOW_CC_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             "$<BUILD_INTERFACE:${tensorflow_cc_INCLUDE_DIRS}>")
endif (TENSORFLOW_CC_SUPPORT)
if (LIBTORCH_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             "$<BUILD_INTERFACE:${LIBTORCH_INCLUDE_DIRS}>")
endif (LIBTORCH_SUPPORT)
if (LLAMACPP_SUPPORT)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             "$<BUILD_INTERFACE:${LLAMACPP_INCLUDE_DIRS}>")
endif (LLAMACPP_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_MachineLearning PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
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
if (LIBTORCH_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_libtorch.cpp
      stream_module_libtorch.h
      stream_module_libtorch.inl)
endif (LIBTORCH_SUPPORT)
if (LLAMACPP_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_llamacpp.cpp
      stream_module_llamacpp.h
      stream_module_llamacpp.inl)
endif (LLAMACPP_SUPPORT)

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
#if (LIBTORCH_SUPPORT)
# target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC ${LIBTORCH_LIBRARIES})
#endif (LIBTORCH_SUPPORT)
#if (LLAMACPP_SUPPORT)
# target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC ${LLAMACPP_LIBRARIES})
#endif (LLAMACPP_SUPPORT)
target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC Common)
target_link_libraries (${PROJECT_NAME}_MachineLearning PUBLIC ${PROJECT_NAME})

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_MachineLearning
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
set (source_files
     stream_module_ml_common.h
     stream_module_ml_defines.h
     stream_module_ml_tools.h)
if (TENSORFLOW_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_tensorflow.h
      stream_module_tensorflow.inl)
endif (TENSORFLOW_SUPPORT)
if (LIBTORCH_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_libtorch.h
      stream_module_libtorch.inl)
endif (LIBTORCH_SUPPORT)
if (LLAMACPP_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_llamacpp.h
      stream_module_llamacpp.inl)
endif (LLAMACPP_SUPPORT)
install (FILES
         ${source_files}
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_MachineLearning
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
