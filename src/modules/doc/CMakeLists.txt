#cmake_minimum_required (VERSION 2.8)

if (UNIX)
 include_directories (/usr/lib/libreoffice/sdk/include)

 add_definitions (-DSAL_UNX)
elseif (WIN32)
 # *NOTE*: workaround
 set(_PF86 "ProgramFiles(x86)")
 set (LIBREOFFICE_SDK_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/LibreOffice_5.1_SDK/sdk"
       "C:/LibreOffice_5.1_SDK_installer/sdk")
# message (STATUS "LibreOffice SDK directory: \"${LIBREOFFICE_SDK_DIRECTORY}\"")
 include_directories (${LIBREOFFICE_SDK_DIRECTORY}/include)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)

############## next target ################
set (${PROJECT_NAME}_Document_LIB_SRCS
     stream_document_defines.h
     stream_document_exports.h
     stream_module_libreoffice_spreadsheet_writer.cpp
     stream_module_libreoffice_spreadsheet_writer.h
     stream_module_libreoffice_spreadsheet_writer.inl
     stdafx.cpp
     stdafx.h)

add_library (${PROJECT_NAME}_Document SHARED ${ACEStream_Document_LIB_SRCS})

add_dependencies (${PROJECT_NAME}_Document Common)
add_dependencies (${PROJECT_NAME}_Document ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Document ${ACE_LIBRARY})
endif ()
target_link_libraries (${PROJECT_NAME}_Document Common)

set_target_properties (${PROJECT_NAME}_Document PROPERTIES DEFINE_SYMBOL STREAM_DOCUMENT_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Document PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Document
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_document_defines.h
         stream_document_exports.h
         stream_module_libreoffice_spreadsheet_writer.h
         stream_module_libreoffice_spreadsheet_writer.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Document
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
