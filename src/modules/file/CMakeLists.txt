set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11)
 add_compile_options (-fPIC)
endif ()

add_library (${PROJECT_NAME}_File STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src>)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/timer>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_File PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src>)
endif ()
target_include_directories (${PROJECT_NAME}_File PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)

############## next target ################
set (source_files
     stream_file_defines.h
     stream_file_exports.h
     stream_file_source.cpp
     stream_file_source.h
     stream_file_source.inl
     stream_file_sink.cpp
     stream_file_sink.h
     stream_file_sink.inl)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
       PRIVATE ${abs_source_file}
      )
#       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_File
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

set_property (TARGET ${PROJECT_NAME}_File PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_File Common)
if (WIN32)
 add_dependencies (${PROJECT_NAME}_File CommonError)
endif ()
add_dependencies (${PROJECT_NAME}_File ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_File ${ACE_LIBRARY})
endif ()
#target_link_libraries (${PROJECT_NAME}_File Common)

set_target_properties (${PROJECT_NAME}_File PROPERTIES DEFINE_SYMBOL STREAM_FILE_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_File PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_File PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_File)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_File
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_file_defines.h
         stream_file_exports.h
         stream_file_source.h
         stream_file_source.inl
         stream_file_sink.h
         stream_file_sink.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_File
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
