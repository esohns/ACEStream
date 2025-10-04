############## definitions ###############

############## includes ###############
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ###############
set (tree_EXE_SRCS
#     stdafx.cpp
     stdafx.h
#     ../test_u_common.h
#     ../test_u_defines.h
     test_u.cpp)

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
set (EXECUTABLE tree)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${tree_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_U)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif ()

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

#target_link_libraries (${EXECUTABLE} PUBLIC ACEStream)
#target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ACEStream_Test_U)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

############### installation #############
install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)

################ launchers ###############
if (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH ${ACE_LIB_DIR} ACE_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIB_PATH})

 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${GTK_PATH})
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${WXWIDGETS_PATH})
 endif (WXWIDGETS_SUPPORT)
endif (WIN32)
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS "-l -m 0 -t"
#                         FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/tree
                        ENVIRONMENT foo=bar)

################ tests ###################
add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE}
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
