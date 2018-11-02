add_library (${PROJECT_NAME}_HTML STATIC "")

include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_LIBXML2 REQUIRED libxml-2.0)

 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${PKG_LIBXML2_INCLUDE_DIRS}>)

 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src>)
 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/error>)
 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/Common/src/timer>)
elseif (WIN32)
# *TODO*: repair pkgconfig support
 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libiconv-1.9.2-1/include>)
 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libxml2/include>)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ATCD/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
endif ()
target_include_directories (${PROJECT_NAME}_HTML PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
set (source_files
     stream_module_htmlparser.cpp
     stream_module_htmlparser.h
     stream_module_htmlparser.inl
     stream_module_htmlwriter.cpp
     stream_module_htmlwriter.h
     stream_module_htmlwriter.inl
     stream_module_xpathquery.cpp
     stream_module_xpathquery.h
     stream_module_xpathquery.inl
     stream_html_exports.h
     stream_html_tools.cpp
     stream_html_tools.h)

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
target_sources (${PROJECT_NAME}_HTML
                PRIVATE stdafx.cpp
                PRIVATE stdafx.h
                ${absolute_source_files})

if (CMAKE_CXX_COMPILER_ID STREQUAL "CLang")
 add_compile_options (-fPIC)
endif ()

set_property (TARGET ${PROJECT_NAME}_HTML PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_HTML Common)
add_dependencies (${PROJECT_NAME}_HTML ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_HTML ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_HTML PROPERTIES DEFINE_SYMBOL STREAM_HTML_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_HTML PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_HTML PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_HTML)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_HTML
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_module_htmlparser.h
         stream_module_htmlparser.inl
         stream_module_htmlwriter.h
         stream_module_htmlwriter.inl
         stream_module_xpathquery.h
         stream_module_xpathquery.inl
         stream_html_exports.h
         stream_html_tools.h
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_HTML
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
