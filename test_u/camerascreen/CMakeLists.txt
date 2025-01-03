#set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/scripts")
#include (PreUIFramework)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl/scripts")
include (assimp)
include (glew)
include (glut)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/image/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image/scripts")
include (png)
include (ffmpeg)
include (ImageMagick)

if (UNIX)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev/scripts")
 include (libcamera)

 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis/scripts")
 include (wayland)
endif (UNIX)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)

 include (FindPkgConfig)
 pkg_check_modules (PKG_V4L REQUIRED libv4l2)
endif (UNIX)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (GLUT_SUPPORT)
 include_directories (${GLUT_INCLUDE_DIRS})
endif (GLUT_SUPPORT)
if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (LIBCAMERA_SUPPORT)
 include_directories (${LIBCAMERA_INCLUDE_DIRS})
endif (LIBCAMERA_SUPPORT)
if (UNIX)
 include_directories (${PKG_V4L_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
elseif (WIN32)
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "D:/projects/DShowBaseClasses")
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

# *TODO*: repair pkgconfig support
 include_directories ("${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses")

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE camerascreen)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.h
     ../test_u_common.h
     ../test_u_defines.h
     test_u_camerascreen.cpp
     test_u_camerascreen_common.h
     test_u_camerascreen_common_modules.h
     test_u_camerascreen_defines.h
     test_u_camerascreen_eventhandler.cpp
     test_u_camerascreen_eventhandler.h
     test_u_camerascreen_eventhandler.inl
     test_u_camerascreen_message.cpp
     test_u_camerascreen_message.h
     test_u_camerascreen_message.inl
     test_u_camerascreen_module_video_wall.cpp
     test_u_camerascreen_module_video_wall.h
     test_u_camerascreen_session_message.cpp
     test_u_camerascreen_session_message.h
     test_u_camerascreen_session_message.inl
     test_u_camerascreen_signalhandler.cpp
     test_u_camerascreen_signalhandler.h
     test_u_camerascreen_stream.cpp
     test_u_camerascreen_stream.h)
if (CURSES_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_u_camerascreen_curses.cpp
      test_u_camerascreen_curses.h
      test_u_camerascreen_curses_window.cpp
      test_u_camerascreen_curses_window.h)
endif (CURSES_SUPPORT)

if (UNIX)
 if (CURSES_SUPPORT)
  add_definitions (-D_XOPEN_SOURCE_EXTENDED) # activate wide character support in ncurses
  add_definitions (-DNCURSES_WIDECHAR) # activate wide character support in ncurses
 endif (CURSES_SUPPORT)
elseif (WIN32)
 # *NOTE*: DirectShow uses 'DEBUG'
# add_definitions (-DDEBUG) # activate DirectShow debug logging
 add_compile_options ($<$<CONFIG:Debug>:-DDEBUG>)
 add_compile_options ($<$<CONFIG:Debug>:-DD3D_DEBUG_INFO>)

 if (CURSES_SUPPORT)
  add_definitions (-DPDC_WIDE) # activate wide character support in PDCurses
 endif (CURSES_SUPPORT)
endif ()

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_U)

#add_dependencies (${EXECUTABLE} ACE)
add_dependencies (${EXECUTABLE} Common)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
#add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

if (CURSES_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CURSES_LIBRARIES})
 if (UNIX)
  target_link_libraries (${EXECUTABLE} PUBLIC ncursesw)
 endif (UNIX)
endif (CURSES_SUPPORT)
if (GLUT_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${GLUT_LIBRARIES})
endif (GLUT_SUPPORT)
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (LIBCAMERA_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBCAMERA_LIBRARIES})
endif (LIBCAMERA_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_V4L_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC d3d12.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC dxgi.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)

 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  target_link_libraries (${EXECUTABLE} PUBLIC ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Debug/strmbasd.lib)
 else ()
#  target_link_libraries (${EXECUTABLE} PUBLIC ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Release/strmbase.lib)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonUI)
 if (CURSES_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_Curses)
 endif (CURSES_SUPPORT)
 if (GTK_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_GTK)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_wxWidgets)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonXML)
endif (UNIX)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
#target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_U)

# add PCH
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${EXECUTABLE})
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
# install (FILES etc/${ui_definition_file}
#          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 if (DEFINED $ENV{LIB_ROOT})
  file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
  string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
  install (FILES ${ACE_DLL_FQPATH}
                 ${LIB_ROOT}/freeglut/bin/x64/freeglut.dll
           DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
 endif (DEFINED $ENV{LIB_ROOT})
# install (FILES etc/${EXECUTABLE}.gtk3 ${EXECUTABLE}.gtk2 ${EXECUTABLE}.fbp
#          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

set (ARGS "-l")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}" COMMAND)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${OPENGL_LIB_DIR}" OPENGL_PATH)
 file (TO_CMAKE_PATH "${GLEW_LIB_DIR}" GLEW_PATH)
 file (TO_CMAKE_PATH "${GLUT_LIB_DIR}" GLUT_PATH)
 file (TO_CMAKE_PATH "${ASSIMP_LIB_DIR}" ASSIMP_PATH)
 file (TO_CMAKE_PATH "${PNG_LIB_DIR}" LIBPNG_PATH)
 # *NOTE*: ffmpeg was built with "vcpkg-libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${OPENGL_PATH}
                           ${GLEW_PATH}
                           ${GLUT_PATH}
                           ${VCPKG_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${ASSIMP_PATH}
                           ${LIBPNG_PATH}
                           ${ACE_LIBRARY_PATH})
 if (GTK_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${GTK_PATH})
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${WXWIDGETS_PATH})
 endif (WXWIDGETS_SUPPORT)
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/${EXECUTABLE}
                        ENVIRONMENT ${ENVIRONMENT})

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
