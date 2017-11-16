if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/math)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/math)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)
endif ()
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dec)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dev)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
if (WIN32)
 set (${PROJECT_NAME}_Statistic_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_stat_common.h
      stream_stat_defines.h
      stream_stat_exports.h
      stream_stat_statistic_analysis.cpp
      stream_stat_statistic_analysis.h
      stream_stat_statistic_analysis.inl
      stream_stat_statistic_handler.cpp
      stream_stat_statistic_handler.h
      stream_stat_statistic_handler.inl
      stream_stat_statistic_report.cpp
      stream_stat_statistic_report.h
      stream_stat_statistic_report.inl
      targetver.h)
elseif (UNIX)
 set (${PROJECT_NAME}_Statistic_LIB_SRCS
      stdafx.cpp
      stdafx.h
      stream_stat_common.h
      stream_stat_defines.h
      stream_stat_exports.h
      stream_stat_statistic_analysis.cpp
      stream_stat_statistic_analysis.h
      stream_stat_statistic_analysis.inl
      stream_stat_statistic_handler.cpp
      stream_stat_statistic_handler.h
      stream_stat_statistic_handler.inl
      stream_stat_statistic_report.cpp
      stream_stat_statistic_report.h
      stream_stat_statistic_report.inl)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "CLang")
 add_compile_options (-fPIC)
endif ()

add_library (${PROJECT_NAME}_Statistic SHARED
             ${ACEStream_Statistic_LIB_SRCS})
set_property (TARGET ${PROJECT_NAME}_Statistic PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Statistic Common)
add_dependencies (${PROJECT_NAME}_Statistic ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Statistic ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Statistic PROPERTIES DEFINE_SYMBOL STREAM_STAT_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Statistic PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Statistic PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Statistic)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Statistic
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
if (WIN32)
 install (FILES
          stream_stat_common.h
          stream_stat_defines.h
          stream_stat_exports.h
          stream_stat_statistic_analysis.h
          stream_stat_statistic_analysis.inl
          stream_stat_statistic_handler.h
          stream_stat_statistic_handler.inl
          stream_stat_statistic_report.h
          stream_stat_statistic_report.inl
          targetver.h
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
elseif (UNIX)
 install (FILES
          stream_stat_common.h
          stream_stat_defines.h
          stream_stat_exports.h
          stream_stat_statistic_analysis.h
          stream_stat_statistic_analysis.inl
          stream_stat_statistic_handler.h
          stream_stat_statistic_handler.inl
          stream_stat_statistic_report.h
          stream_stat_statistic_report.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif ()

install (TARGETS ${PROJECT_NAME}_Statistic
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
