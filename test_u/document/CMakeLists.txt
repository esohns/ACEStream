set (ACE_LIB_FILE libACE.so)
if (DEFINED ENV{ACE_ROOT})
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES ace build/linux/ace
               DOC "searching for ${ACE_LIB_FILE}")
else ()
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               DOC "searching for ${ACE_LIB_FILE} (system paths)")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED COMPONENTS lib
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

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

 add_definitions (-DSAL_UNX)

 include_directories (${OO_SDK_DIRECTORY}/include)
# include_directories (/usr/include/libreoffice)
elseif (WIN32)
 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_PATH})
   set (OO_SDK_DIRECTORY $ENV{UNO_PATH}/../sdk)
 else ()
  message (WARNING "%OO_SDK_HOME%/%UNO_PATH% not set, falling back")
  set (OO_SDK_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../openoffice/sdk)
 endif ()
# *NOTE*: workaround
# set(_PF86 "ProgramFiles(x86)")
# set (OO_SDK_DIRECTORY
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/LibreOffice_5.1_SDK/sdk"
#       "C:/LibreOffice_5.1_SDK_installer/sdk")
#message (STATUS "Apache OpenOffice SDK directory: \"${OO_SDK_DIRECTORY}\"")
 include_directories (${OO_SDK_DIRECTORY}/include)

 if (DEFINED ENV{UNO_PATH})
  string (REGEX REPLACE "\\\\" "/" OO_APP_DIRECTORY $ENV{UNO_PATH})
#  set (OO_APP_DIRECTORY $ENV{UNO_PATH})
 else ()
  message (WARNING "%UNO_PATH% not set, falling back")
  string (REGEX REPLACE "\\\\" "/" OO_APP_DIRECTORY ${OO_SDK_DIRECTORY}/program)
 endif ()
# message (STATUS "Apache OpenOffice UNO directory: \"${OO_APP_DIRECTORY}\"")
# *NOTE*: the OpenOffice SDK provides its own snprintf; however MSVC complains
#         about mutliple definitions (and inconsistent linkage)
#         --> disable that header and use the native function
 if (MSVC)
  add_definitions (-D_SNPRINTF_H)
 endif ()
endif ()

############## definitions ###############
add_definitions (-DACE_HAS_DLL)
add_definitions (-DHAVE_CONFIG_H)
set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)
#add_definitions (-DCOMMON_HAS_DLL)
#add_definitions (-DSTREAM_HAS_DLL)

############## includes ###############
include_directories ($ENV{LIB_ROOT}/ACE_TAO/ACE)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/tree)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/doc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ###############
set (document_EXE_SRCS
     document_defines.h
     test_u_common_modules.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_message.cpp
     test_u_message.h
     test_u_module_eventhandler.cpp
     test_u_module_eventhandler.h
     test_u_module_spreadsheetwriter.cpp
     test_u_module_spreadsheetwriter.h
     test_u_session_message.cpp
     test_u_session_message.h
     test_u_stream.cpp
     test_u_stream.h
     test_u_stream_common.h
     stdafx.cpp
     stdafx.h
#     ../test_u_common.h
#     ../test_u_defines.h
     test_u.cpp)

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
set (EXECUTABLE document)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${document_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")

target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})

if (UNIX)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/libuno_sal.so)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libgcc3_uno.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/libuno_cppu.so)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libstorelo.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libreglo.so)

 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libxmlreaderlo.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/../program/libunoidllo.so)
 target_link_libraries (${EXECUTABLE} ${OO_SDK_DIRECTORY}/lib/libuno_cppuhelpergcc3.so)
elseif (WIN32)
 # *WARNING*: linking Common requires "/FORCE:MULTIPLE"
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonLog)
#target_link_libraries (${EXECUTABLE} CommonParser)
target_link_libraries (${EXECUTABLE} CommonTimer)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Document)
#target_link_libraries (${EXECUTABLE} ACEStream_Statistic)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

############### installation #############
install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
# install (FILES etc/${EXECUTABLE}.glade
#          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)

################ launchers ###############
file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIB_PATH)
set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                          ${ACE_LIB_PATH})
create_target_launcher (${EXECUTABLE}
                        ARGS "-l -t"
#                         FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                                             ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                             \"${OO_APP_DIRECTORY}\"
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/document
                        ENVIRONMENT foo=bar)

################ tests ###################
add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE}
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
