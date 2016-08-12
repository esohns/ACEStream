cmake_minimum_required (VERSION 2.8)

include (FindPkgConfig)
if (UNIX)
 pkg_check_modules (PKG_LIBXML2 REQUIRED libxml-2.0)

 include_directories (${PKG_LIBXML2_INCLUDE_DIRS})
elseif (WIN32)
# *TODO*: repair pkgconfig support
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libiconv-1.9.2/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libxml2-2.9.4/win32/include/libxml2)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
set (${PROJECT_NAME}_HTML_LIB_SRCS
     stdafx.cpp
     stdafx.h
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

add_library (${PROJECT_NAME}_HTML SHARED ${ACEStream_HTML_LIB_SRCS})

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
