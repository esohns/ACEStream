add_library (${PROJECT_NAME}_Statistic STATIC "")

if (UNIX)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/math>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/image>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/log>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/math>)
 target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
endif ()
target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../..>)
target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dev>)
target_include_directories (${PROJECT_NAME}_Statistic PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)

############## next target ################
set (source_files
     stream_stat_common.h
     stream_stat_defines.h
     stream_stat_statistic_analysis.cpp
     stream_stat_statistic_analysis.h
     stream_stat_statistic_analysis.inl
     stream_stat_statistic_report.cpp
     stream_stat_statistic_report.h
     stream_stat_statistic_report.inl)
#if (WIN32)
# set (source_files
#      ${source_files})
#endif (WIN32)

# Include dir for installed targets
set (INCLUDE_DIR include/${PROJECT_NAME}-${upstream_version})

unset (absolute_source_files)
foreach (source_file ${source_files})
 get_filename_component (abs_source_file ${source_file} ABSOLUTE)
 list (APPEND absolute_source_files
   PRIVATE ${abs_source_file}
   )
 #       $<INSTALL_INTERFACE:${INCLUDE_DIR}/${source_file}>)
endforeach ()
target_sources (${PROJECT_NAME}_Statistic
#                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                PRIVATE ${absolute_source_files})
target_precompile_headers (${PROJECT_NAME}_Statistic
                           PUBLIC stdafx.h)

set_property (TARGET ${PROJECT_NAME}_Statistic PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Statistic Common)
add_dependencies (${PROJECT_NAME}_Statistic ${PROJECT_NAME})

target_link_libraries (${PROJECT_NAME}_Statistic PUBLIC ${ACE_LIBRARY})

target_link_libraries (${PROJECT_NAME}_Statistic PUBLIC Common)

target_link_libraries (${PROJECT_NAME}_Statistic PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Statistic PROPERTIES DEFINE_SYMBOL STREAM_STAT_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Statistic PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
#set_target_properties (${PROJECT_NAME}_Statistic PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${PROJECT_NAME}_Statistic)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Statistic
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_stat_common.h
         stream_stat_defines.h
         stream_stat_exports.h
         stream_stat_statistic_analysis.h
         stream_stat_statistic_analysis.inl
         stream_stat_statistic_report.h
         stream_stat_statistic_report.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Statistic
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
