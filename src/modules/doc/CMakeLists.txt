include (Office)

##########################################

add_library (${PROJECT_NAME}_Document STATIC "")

if (LIBREOFFICE_SUPPORT)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${LIBREOFFICE_INCLUDE_DIRS}>)
endif (LIBREOFFICE_SUPPORT)
if (UNIX)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
elseif (WIN32)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/error>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/parser>)
 target_include_directories (${PROJECT_NAME}_Document PUBLIC
                             $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../../Common/src/timer>)
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
if (LIBREOFFICE_SUPPORT)
 set (source_files
      ${source_files}
      stream_module_libreoffice_document_handler.cpp
      stream_module_libreoffice_document_handler.h
      stream_module_libreoffice_document_writer.cpp
      stream_module_libreoffice_document_writer.h
      stream_module_libreoffice_document_writer.inl)
endif (LIBREOFFICE_SUPPORT)

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
target_sources (${PROJECT_NAME}_Document
               PRIVATE stdafx.cpp
               PRIVATE stdafx.h
               ${absolute_source_files})

set_property (TARGET ${PROJECT_NAME}_Document PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Document Common)
add_dependencies (${PROJECT_NAME}_Document ${PROJECT_NAME})

if (LIBREOFFICE_SUPPORT)
 target_link_libraries (${PROJECT_NAME}_Document PUBLIC ${LIBREOFFICE_LIBRARIES})
endif (LIBREOFFICE_SUPPORT)

target_link_libraries (${PROJECT_NAME}_Document PUBLIC ${ACE_LIBRARY})

target_link_libraries (${PROJECT_NAME}_Document PUBLIC Common)

target_link_libraries (${PROJECT_NAME}_Document PUBLIC ${PROJECT_NAME})

#set_target_properties (${PROJECT_NAME}_Document PROPERTIES DEFINE_SYMBOL STREAM_DOCUMENT_BUILD_DLL)
#set_target_properties (${PROJECT_NAME}_Document PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Document PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Document)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Document
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_document_defines.h
#         stream_document_exports.h
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)
if (LIBREOFFICE_SUPPORT)
 install (FILES
          stream_module_libreoffice_document_handler.h
          stream_module_libreoffice_document_writer.h
          stream_module_libreoffice_document_writer.inl
          DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
          COMPONENT mod_include)
endif (LIBREOFFICE_SUPPORT)
install (TARGETS ${PROJECT_NAME}_Document
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
