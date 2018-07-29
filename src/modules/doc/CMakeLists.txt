include (FindPkgConfig)
if (UNIX)
# pkg_check_modules (PKG_DBUS REQUIRED dbus-1)
# pkg_check_modules (PKG_MYSQL REQUIRED libmysqlclient-dev)
endif ()

add_library (${PROJECT_NAME}_Document STATIC "")

if (UNIX)
# include_directories (${PKG_DBUS_INCLUDE_DIRS})
# include_directories (/usr/lib/libreoffice/sdk/include)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:/usr/include/libreoffice>)

 add_definitions (-DSAL_UNX) # libreoffice
 add_definitions (-DUNX)     # openoffice

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/ACE>)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/timer>)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src>)
elseif (WIN32)
 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_HOME})
   set (OO_SDK_DIRECTORY $ENV{UNO_PATH}\..\sdk)
 else ()
  message (WARNING "%OO_SDK_HOME%/%UNO_PATH% not set, falling back")
  set (OO_SDK_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../openoffice/sdk)
 endif ()
# *NOTE*: workaround
# set(_PF86 "ProgramFiles(x86)")
# set (OO_SDK_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/LibreOffice_5.1_SDK/sdk"
#       "C:/LibreOffice_5.1_SDK_installer/sdk")
# message (STATUS "Apache OpenOffice SDK include directory: \"${OO_SDK_DIRECTORY}/include\"")
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${OO_SDK_DIRECTORY}/include>)

 add_definitions (-DCPPU_ENV=msci)
 add_definitions (-DWNT)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src>)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../libACENetwork/src>)
endif ()
target_include_directories (${PROJECT_NAME}_Document PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Document PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../stat>)

############## next target ################
set (source_files
     stream_document_defines.h
     stream_document_exports.h
     stream_module_libreoffice_document_handler.cpp
     stream_module_libreoffice_document_handler.h
     stream_module_libreoffice_document_writer.cpp
     stream_module_libreoffice_document_writer.h
     stream_module_libreoffice_document_writer.inl)

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
target_sources (${PROJECT_NAME}_Document
               PRIVATE stdafx.cpp
               PRIVATE stdafx.h
               ${absolute_source_files})

if (CMAKE_CXX_COMPILER_ID STREQUAL "CLang")
 add_compile_options (-std=c++0x -fPIC)
endif ()

set_property (TARGET ${PROJECT_NAME}_Document PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Document Common)
add_dependencies (${PROJECT_NAME}_Document ${PROJECT_NAME})

if (WIN32)
 target_link_libraries (${PROJECT_NAME}_Document ${ACE_LIBRARY})

 target_link_libraries (${PROJECT_NAME}_Document ${OO_SDK_DIRECTORY}/lib/isal.lib)

 target_link_libraries (${PROJECT_NAME}_Document ${OO_SDK_DIRECTORY}/lib/icppu.lib)
 target_link_libraries (${PROJECT_NAME}_Document ${OO_SDK_DIRECTORY}/lib/icppuhelper.lib)
endif ()
#target_link_libraries (${PROJECT_NAME}_Document Common)

set_target_properties (${PROJECT_NAME}_Document PROPERTIES DEFINE_SYMBOL STREAM_DOCUMENT_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Document PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Document PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Document)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Document
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_document_defines.h
         stream_document_exports.h
         stream_module_libreoffice_document_handler.h
         stream_module_libreoffice_document_writer.h
         stream_module_libreoffice_document_writer.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
install (TARGETS ${PROJECT_NAME}_Document
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
