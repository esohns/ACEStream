set (OPENOFFICE_DEFAULT OFF)
option (OPENOFFICE_SUPPORT "support OpenOffice/LibreOffice" ${OPENOFFICE_DEFAULT})
if (OPENOFFICE_SUPPORT)
# message (STATUS "OpenOffice support set, set OO_SDK_HOME and UNO_PATH and run make_oo_headers.bat from 'scripts' to generate the necessary #include headers")
 add_definitions (-DOPENOFFICE_SUPPORT)
endif (OPENOFFICE_SUPPORT)

##########################################

add_library (${PROJECT_NAME}_Document STATIC "")

if (UNIX)
 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_PATH})
   message (WARNING "OO_SDK_HOME not set, falling back to UNO_PATH (was: \"${UNO_PATH}\")")
   set (OO_SDK_DIRECTORY $ENV{UNO_PATH}/../sdk)
 else ()
  set (OO_SDK_DIRECTORY /usr/lib64/libreoffice/sdk)
  #set (OO_SDK_DIRECTORY /usr/lib/libreoffice/sdk)
  message (WARNING "OO_SDK_HOME|UNO_PATH not set, falling back to /usr/lib64/libreoffice/sdk")
 endif ()
 if (NOT EXISTS ${OO_SDK_DIRECTORY})
  message (FATAL_ERROR "OpenOffice SDK directory not found (was: \"${OO_SDK_DIRECTORY}\"), aborting")
 endif (NOT EXISTS ${OO_SDK_DIRECTORY})
elseif (WIN32)
# message (STATUS "ENV{OO_SDK_HOME}: \"$ENV{OO_SDK_HOME}\")")
 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_HOME})
  message (WARNING "OO_SDK_HOME not set, falling back to UNO_HOME (was: \"${UNO_HOME}\")")
  set (OO_SDK_DIRECTORY $ENV{UNO_HOME}\..\sdk)
 else ()
  message (WARNING "%OO_SDK_HOME%/%UNO_HOME% not set, falling back")
  set (OO_SDK_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../openoffice/sdk)
 endif ()
endif ()
target_include_directories (${PROJECT_NAME}_Document PUBLIC
                            $<BUILD_INTERFACE:${OO_SDK_DIRECTORY}/include>)

if (UNIX)
 add_definitions (-DSAL_UNX) # libreoffice
 add_definitions (-DUNX)     # openoffice

  target_include_directories (${PROJECT_NAME}_Document PUBLIC
                              $<BUILD_INTERFACE:$ENV{LIB_ROOT}/ACE_TAO/ACE>)
#                             $<BUILD_INTERFACE:/usr/local/src/ACE_wrappers>)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACENetwork/src>)
elseif (WIN32)
 add_definitions (-DCPPU_ENV=msci)
 add_definitions (-DWNT)

 if (DEFINED ENV{ACE_ROOT})
  target_include_directories (${PROJECT_NAME}_Document PUBLIC
                              $<BUILD_INTERFACE:$ENV{ACE_ROOT}>)
 else ()
  target_include_directories (${PROJECT_NAME}_Document PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACE_TAO/ACE>)
 endif (DEFINED ENV{ACE_ROOT})

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)

 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../ACENetwork/src>)
endif ()
target_include_directories (${PROJECT_NAME}_Document PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>)
target_include_directories (${PROJECT_NAME}_Document PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
target_include_directories (${PROJECT_NAME}_Document PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../stat>)

############## next target ################
set (source_files
     stream_document_defines.h)
if (OPENOFFICE_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_libreoffice_document_handler.cpp
      stream_module_libreoffice_document_handler.h
      stream_module_libreoffice_document_writer.cpp
      stream_module_libreoffice_document_writer.h
      stream_module_libreoffice_document_writer.inl)
endif (OPENOFFICE_SUPPORT)

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

 if (OPENOFFICE_SUPPORT)
  target_link_libraries (${PROJECT_NAME}_Document ${OO_SDK_DIRECTORY}/lib/isal.lib)

  target_link_libraries (${PROJECT_NAME}_Document ${OO_SDK_DIRECTORY}/lib/icppu.lib)
  target_link_libraries (${PROJECT_NAME}_Document ${OO_SDK_DIRECTORY}/lib/icppuhelper.lib)
 endif (OPENOFFICE_SUPPORT)
endif ()
target_link_libraries (${PROJECT_NAME}_Document Common)
target_link_libraries (${PROJECT_NAME}_Document ${PROJECT_NAME})

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
