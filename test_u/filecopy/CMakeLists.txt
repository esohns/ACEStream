if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif ()

include (FindPkgConfig)
#pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
# *NOTE*: Common requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
elseif (WIN32)
endif ()
# *TODO*: repair WIN32 pkgconfig support

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (UNIX)
 include_directories (${PKG_CAP_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
elseif (WIN32)
# *TODO*: repair pkgconfig support
# if (DEFINED ENV{ACE_ROOT})
#  include_directories ($ENV{ACE_ROOT})
# else ()
#  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
# endif (DEFINED ENV{ACE_ROOT})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
endif ()
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE filecopy)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_u_common.h
     ../test_u_defines.h
     test_u_filecopy.cpp
     test_u_filecopy_common.h
     test_u_filecopy_common_modules.h
     test_u_filecopy_defines.h
     test_u_filecopy_eventhandler.cpp
     test_u_filecopy_eventhandler.h
     test_u_filecopy_message.cpp
     test_u_filecopy_message.h
     test_u_filecopy_module_eventhandler.cpp
     test_u_filecopy_module_eventhandler.h
     test_u_filecopy_session_message.cpp
     test_u_filecopy_session_message.h
     test_u_filecopy_signalhandler.cpp
     test_u_filecopy_signalhandler.h
     test_u_filecopy_stream.cpp
     test_u_filecopy_stream.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_u_filecopy_callbacks.cpp
      test_u_filecopy_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
#       test_u_filecopy_ui.cpp
#       test_u_filecopy_ui.h
#       test_u_filecopy_ui.inl
#       test_u_filecopy_ui_base.cpp
#       test_u_filecopy_ui_base.h)
      )
endif (WXWIDGETS_SUPPORT)

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
#set (GLADE_FILE etc/${EXECUTABLE}.glade)
if (GTK2_USE)
 set (GLADE_FILE etc/${EXECUTABLE}.gtk2)
elseif (GTK3_USE)
 set (GLADE_FILE etc/${EXECUTABLE}.gtk3)
endif ()
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${filecopy_EXE_SRCS}
                ${GLADE_FILE})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
 target_link_options (${EXECUTABLE} PUBLIC "-rdynamic")

 target_link_libraries (${EXECUTABLE} PUBLIC rt)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} PUBLIC X11)
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC Advapi32.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Ole32.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Quartz.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Shell32.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC User32.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Userenv.lib)
# *TODO*: repair WIN32 pkgconfig support
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)
if (GTK_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_GTK)
 if (GTKGL_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonOpenGL)
 endif (GTKGL_SUPPORT)
endif (GTK_SUPPORT)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_U)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_CAP_LIBRARIES})
endif (UNIX)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES ${GLADE_FILE}
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 install (FILES ${GLADE_FILE}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}" COMMAND)
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-g./etc/filecopy.gtk3 -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/gtk/Win32/debug/bin
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../Common/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/filecopy
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIB_PATH)
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
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIB_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-g.\\\\etc\\\\filecopy.gtk3 -l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/filecopy
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
