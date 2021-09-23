include (FindPkgConfig)
include (ffmpeg)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev/scripts")
include (libcamera)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)

 pkg_check_modules (PKG_V4L REQUIRED libv4l2)
endif (UNIX)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

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
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE camerascreen)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.cpp
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
     test_u_camerascreen_session_message.cpp
     test_u_camerascreen_session_message.h
     test_u_camerascreen_session_message.inl
     test_u_camerascreen_stream.cpp
     test_u_camerascreen_stream.h)

if (UNIX)
elseif (WIN32)
 if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
  add_definitions (-DD3D_DEBUG_INFO)
  # *NOTE*: DirectShow uses 'DEBUG'
  add_definitions (-DDEBUG) # activate DirectShow debug logging
 endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
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
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

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
# target_link_libraries (${EXECUTABLE} PUBLIC dxguid.lib)
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
 if (GTK_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_GTK)
 elseif (WXWIDGETS_USE)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_wxWidgets)
 endif ()
endif (GUI_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonXML)
endif (UNIX)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_U)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

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
 install (FILES ${ACE_DLL_FQPATH}
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libatk-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/freetype6.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/intl.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libcairo-2.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libfontconfig-1.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgio-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libglib-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgmodule-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgobject-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgthread-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgdk_pixbuf-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgdk-win32-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libgtk-win32-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpango-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangocairo-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangoft2-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpangowin32-1.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/libpng14-14.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk/bin/zlib1.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libglade/bin/libglade-2.0-0.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libxml2-2.9.1/.libs/libxml2-2.dll
                G:/software/Development/dll/libgcc_s_dw2-1.dll
                G:/software/Development/dll/libexpat-1.dll
                G:/software/Development/dll/libiconv-2.dll
                ${Common_DIR}/src/Debug/Common.dll
                ${Common_DIR}/src/ui/Debug/CommonUI.dll
                ${ACENetwork_DIR}/src/Debug/ACENetwork.dll
                ${ACENetwork_DIR}/src/protocol/http/Debug/ACENetwork_Protocol_HTTP.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libmysql_2/cmake/libmysql/Debug/libmysql.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
 install (FILES etc/${EXECUTABLE}.gtk3 ${EXECUTABLE}.gtk2 ${EXECUTABLE}.fbp
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

set (ARGS "-l")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
if (UNIX)
# set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ImageMagick IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/freeglut/bin GLUT_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/assimp/cmake/code/${CMAKE_BUILD_TYPE} ASSIMP_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${GLUT_PATH}
                           ${ASSIMP_PATH}
                           ${GTK_LIB_PATH}
                           ${ACE_LIBRARY_PATH})
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${PROJECT_SOURCE_DIR}/../libpng/cmake/${CMAKE_BUILD_TYPE}
 #                          ${PROJECT_SOURCE_DIR}/../ffmpeg/bin
#                           ${PROJECT_SOURCE_DIR}/../ffmpeg/libavutil
#                           ${PROJECT_SOURCE_DIR}/../ffmpeg/libswresample
#                           ${PROJECT_SOURCE_DIR}/../ffmpeg/libswscale
                           ${PROJECT_SOURCE_DIR}/../assimp/cmake/code/${CMAKE_BUILD_TYPE})
endif ()
create_target_launcher (${EXECUTABLE}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/camerascreen
                        ENVIRONMENT ${ENVIRONMENT})

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
