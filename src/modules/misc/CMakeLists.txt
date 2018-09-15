if (UNIX)
# add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
endif ()

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11)
 #add_compile_options (-fPIC)
endif ()

add_library (${PROJECT_NAME}_Miscellaneous STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/error>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/timer>)
elseif (WIN32)
 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/error>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/math>)
 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/timer>)

 target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
endif ()
target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison>)

target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)

target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dec>)
target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dev>)
target_include_directories (${PROJECT_NAME}_Miscellaneous PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../file>)

############## next target ################
set (source_files
     stream_misc_aggregator.cpp
     stream_misc_aggregator.h
     stream_misc_aggregator.inl
     stream_misc_common.h
     stream_misc_defines.h
     stream_misc_dump.cpp
     stream_misc_dump.h
     stream_misc_dump.inl
     stream_misc_exports.h
     stream_misc_messagehandler.cpp
     stream_misc_messagehandler.h
     stream_misc_messagehandler.inl
     stream_misc_parser.cpp
     stream_misc_parser.h
     stream_misc_parser.inl
     stream_misc_queue_source.cpp
     stream_misc_queue_source.h
     stream_misc_queue_source.inl
     stream_misc_splitter.cpp
     stream_misc_splitter.h
     stream_misc_splitter.inl)

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
target_sources (${PROJECT_NAME}_Miscellaneous
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

set_property (TARGET ${PROJECT_NAME}_Miscellaneous PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Miscellaneous Common)
add_dependencies (${PROJECT_NAME}_Miscellaneous ${PROJECT_NAME})

if (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous strmbase.lib)
 endif ()
# target_link_libraries (${PROJECT_NAME}_Miscellaneous winmm.lib)
 target_link_libraries (${PROJECT_NAME}_Miscellaneous ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES DEFINE_SYMBOL STREAM_MISC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Miscellaneous)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Miscellaneous
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_misc_aggregator.h
         stream_misc_aggregator.inl
         stream_misc_common.h
         stream_misc_defines.h
         stream_misc_dump.h
         stream_misc_dump.inl
         stream_misc_exports.h
         stream_misc_messagehandler.h
         stream_misc_messagehandler.inl
         stream_misc_parser.h
         stream_misc_parser.inl
         stream_misc_queue_source.h
         stream_misc_queue_source.inl
         stream_misc_splitter.h
         stream_misc_splitter.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)

install (TARGETS ${PROJECT_NAME}_Miscellaneous
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
