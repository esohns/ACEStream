set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/scripts")
include (SSLConfig)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (UNIX)
 add_compile_options ("-rdynamic")
 add_link_options ("LINKER:--export-dynamic")
endif (UNIX)

find_package (ACENetwork
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif (NOT EXISTS ${ACENetwork_DIR})

if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/src/client_server)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/error)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE_SOURCE filesource)
set (${EXECUTABLE_SOURCE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_filestream_common.h
     test_i_filestream_defines.h
     test_i_filestream_network.h
     test_i_message.cpp
     test_i_message.h
     test_i_message.inl
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_source.cpp
     test_i_source_common.h
     test_i_source_eventhandler.cpp
     test_i_source_eventhandler.h
     test_i_source_signalhandler.cpp
     test_i_source_signalhandler.h
     test_i_source_stream.cpp
     test_i_source_stream.h
     test_i_source_stream.inl
     test_i_target_stream.cpp
     test_i_target_stream.h)
#  test_i_template_instantiations.cpp
if (GTK_SUPPORT)
 set (${EXECUTABLE_SOURCE}_EXE_SRCS
      ${${EXECUTABLE_SOURCE}_EXE_SRCS}
      test_i_callbacks.cpp
      test_i_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
#  set (${EXECUTABLE_SOURCE}_EXE_SRCS
#       ${${EXECUTABLE_SOURCE}_EXE_SRCS})
endif (WXWIDGETS_SUPPORT)

if (GTK_SUPPORT)
 set (SOURCE_RESOURCE_FILES
      etc/resources.rc)
 set (TARGET_RESOURCE_FILES
      etc/resources.rc)
 if (GTK3_SUPPORT)
  list (APPEND
        SOURCE_RESOURCE_FILES
        etc/source.gtk3)
  list (APPEND
        TARGET_RESOURCE_FILES
        etc/target.gtk3)
 else ()
  list (APPEND
        SOURCE_RESOURCE_FILES
        etc/source.glade)
  list (APPEND
        TARGET_RESOURCE_FILES
        etc/target.glade)
 endif (GTK3_SUPPORT)
 set_source_files_properties (${SOURCE_RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
 set_source_files_properties (${TARGET_RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
 source_group (resources FILES ${SOURCE_RESOURCE_FILES})
 source_group (resources FILES ${TARGET_RESOURCE_FILES})
endif (GTK_SUPPORT)

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE_SOURCE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_SOURCE}_EXE_SRCS}
                ${SOURCE_RESOURCE_FILES})
set_property (TARGET ${EXECUTABLE_SOURCE} PROPERTY FOLDER "test_i")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} X11)
endif (UNIX)

target_link_libraries (${EXECUTABLE_SOURCE} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_SOURCE} Common)
target_link_libraries (${EXECUTABLE_SOURCE} CommonEvent)
target_link_libraries (${EXECUTABLE_SOURCE} CommonLog)
target_link_libraries (${EXECUTABLE_SOURCE} CommonSignal)
target_link_libraries (${EXECUTABLE_SOURCE} CommonTimer)

target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Test_I)

target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork)
target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork_Server)

target_precompile_headers (${EXECUTABLE_SOURCE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE_SOURCE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT test_i)

if (GTK_USE)
 if (WIN32)
  install (FILES ${SOURCE_RESOURCE_FILES}
           DESTINATION "etc/filestream" COMPONENT config)
  install (FILES ${TARGET_RESOURCE_FILES}
           DESTINATION "etc/filestream" COMPONENT config)
 elseif (UNIX)
  install (FILES ${SOURCE_RESOURCE_FILES}
           DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/filestream" COMPONENT config)
  install (FILES ${TARGET_RESOURCE_FILES}
           DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/filestream" COMPONENT config)
 endif ()
endif (GTK_USE)

if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SOURCE}" COMMAND)
 create_target_launcher (${EXECUTABLE_SOURCE}
                         COMMAND ${COMMAND}
#                         ARGS "-l -t"
                         ARGS "-f /mnt/win_e/video/Akira.mkv -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SOURCE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl SSL_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_PATH)
 create_target_launcher (${EXECUTABLE_SOURCE}
                         COMMAND ${COMMAND}
                         ARGS "-f E:\\\\video\\\\Akira.mkv -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${GTK_PATH}
                                              ${WXWIDGETS_PATH}
                                              ${SSL_PATH}
                                              ${ACE_PATH}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME filestream_source
          COMMAND ${EXECUTABLE_SOURCE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (filestream_source
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")

############## next target ################
set (EXECUTABLE_TARGET filetarget)
set (${EXECUTABLE_TARGET}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_filestream_common.h
     test_i_filestream_defines.h
     test_i_filestream_network.h
     test_i_message.cpp
     test_i_message.h
     test_i_message.inl
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_target.cpp
     test_i_target_common.h
     test_i_target_eventhandler.cpp
     test_i_target_eventhandler.h
     test_i_target_listener_common.h
     test_i_target_signalhandler.cpp
     test_i_target_signalhandler.h
     test_i_target_stream.cpp
     test_i_target_stream.h)
#  test_i_template_instantiations.cpp
if (GTK_SUPPORT)
 set (${EXECUTABLE_TARGET}_EXE_SRCS
      ${${EXECUTABLE_TARGET}_EXE_SRCS}
      test_i_callbacks.cpp
      test_i_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
#  set (${EXECUTABLE_TARGET}_EXE_SRCS
#       ${${EXECUTABLE_TARGET}_EXE_SRCS})
endif (WXWIDGETS_SUPPORT)

add_executable (${EXECUTABLE_TARGET}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_TARGET}_EXE_SRCS}
                ${TARGET_RESOURCE_FILES})
set_property (TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "test_i")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)

target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_TARGET} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} X11)
elseif (WIN32)
endif ()
target_link_libraries (${EXECUTABLE_TARGET} Common)
target_link_libraries (${EXECUTABLE_TARGET} CommonEvent)
target_link_libraries (${EXECUTABLE_TARGET} CommonLog)
target_link_libraries (${EXECUTABLE_TARGET} CommonSignal)
target_link_libraries (${EXECUTABLE_TARGET} CommonTimer)

target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Test_I)

target_link_libraries (${EXECUTABLE_TARGET} ACENetwork)
target_link_libraries (${EXECUTABLE_TARGET} ACENetwork_Server)

target_precompile_headers (${EXECUTABLE_TARGET}
                           REUSE_FROM ${EXECUTABLE_SOURCE})

##########################################

install (TARGETS ${EXECUTABLE_TARGET}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT test_i)

if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_TARGET}" COMMAND)
 create_target_launcher (${EXECUTABLE_TARGET}
                         COMMAND ${COMMAND}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_TARGET}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 create_target_launcher (${EXECUTABLE_TARGET}
                         COMMAND ${COMMAND}
                         ARGS "-l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${GTK_PATH}
                                              ${WXWIDGETS_PATH}
                                              ${SSL_PATH}
                                              ${ACE_PATH}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/filestream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE_TARGET}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE_TARGET} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE_TARGET}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
