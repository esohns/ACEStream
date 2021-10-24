get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)

############## definitions ###############
#add_definitions (-DACE_HAS_DLL)
add_definitions (-DHAVE_CONFIG_H)
set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)
#add_definitions (-DCOMMON_HAS_DLL)
#add_definitions (-DSTREAM_HAS_DLL)

############## includes ###############
#include_directories (${PKG_FLEX_BISON_INCLUDE_DIRS})

#include_directories ($ENV{LIB_ROOT}/ACE_TAO/ACE)

#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/3rd_party/bison)
#if (WIN32)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/3rd_party)
#endif (WIN32)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)

#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/tree)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ###############
set (branch_EXE_SRCS
     branch_common_modules.h
     branch_eventhandler.cpp
     branch_eventhandler.h
     branch_message.cpp
     branch_message.h
     branch_module_eventhandler.cpp
     branch_module_eventhandler.h
     branch_session_message.cpp
     branch_session_message.h
     branch_stream.cpp
     branch_stream.h
     branch_stream_common.h
     stdafx.cpp
     stdafx.h
#     ../test_i_common.h
#     ../test_i_defines.h
     test_u.cpp)

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
set (EXECUTABLE branch)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${branch_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")

#target_link_libraries (${EXECUTABLE} ${PKG_FLEX_BISON_LIBRARIES})

target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonLog)
#target_link_libraries (${EXECUTABLE} CommonParser)
target_link_libraries (${EXECUTABLE} CommonTimer)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Statistic)

# add PCH
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${EXECUTABLE})
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

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
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/branch
                        ENVIRONMENT foo=bar)

################ tests ###################
add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE}
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
