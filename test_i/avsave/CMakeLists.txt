include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev/scripts")
include (libcamera)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)

# *NOTE*: Common requires libcap
 include (FindPkgConfig)
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L2 REQUIRED libv4l2)
# pkg_check_modules (PKG_FFMPEG REQUIRED libswscale libavutil libavformat libavcodec)
endif (UNIX)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 include_directories (${ImageMagick_INCLUDE_DIRS})
endif (IMAGEMAGICK_SUPPORT)
if (UNIX)
 include_directories (${PKG_V4L2_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
 if (GUI_SUPPORT)
  if (QT_USE)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/qt)
  endif (QT_USE)
 endif (GUI_SUPPORT)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/test_i)
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

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/test_i)
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
set (EXECUTABLE avsave)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_i_common.h
     ../test_i_defines.h
     test_i_avsave.cpp
     test_i_avsave_common.h
     test_i_avsave_common_modules.h
     test_i_avsave_defines.h
     test_i_avsave_encoder.cpp
     test_i_avsave_encoder.h
     test_i_avsave_encoder.inl
     test_i_avsave_eventhandler.cpp
     test_i_avsave_eventhandler.h
     test_i_avsave_eventhandler.inl
     test_i_avsave_message.cpp
     test_i_avsave_message.h
     test_i_avsave_message.inl
     test_i_avsave_session_message.cpp
     test_i_avsave_session_message.h
     test_i_avsave_session_message.inl
     test_i_avsave_signalhandler.cpp
     test_i_avsave_signalhandler.h
     test_i_avsave_stream.cpp
     test_i_avsave_stream.h)
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_i_avsave_gtk_callbacks.cpp
       test_i_avsave_gtk_callbacks.h)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_i_avsave_ui.cpp
       test_i_avsave_ui.h
       test_i_avsave_ui.inl
       avsave_wxwidgets_ui.cpp
       avsave_wxwidgets_ui.h)
#       test_i_avsave_ui_base.cpp
#       test_i_avsave_ui_base.h)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)

if (UNIX)
elseif (WIN32)
 if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
  add_definitions (-DD3D_DEBUG_INFO)
  # *NOTE*: DirectShow uses 'DEBUG'
  add_definitions (-DDEBUG) # activate DirectShow debug logging
 endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
endif ()

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property

set (ui_definition_file "")
if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTK3_USE)
   set (ui_definition_file etc/${EXECUTABLE}.gtk3)
  elseif (GTK2_USE)
   set (ui_definition_file etc/${EXECUTABLE}.gtk2)
  endif ()
 elseif (WXWIDGETS_USE)
  if (WIN32)
   add_definitions(-DUNICODE -D_UNICODE)
  endif (WIN32)
  set (ui_definition_file etc/${EXECUTABLE}.xrc)
 endif ()
endif (GUI_SUPPORT)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                ${ui_definition_file})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_I)

add_dependencies (${EXECUTABLE} Common)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_I)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif (UNIX)

if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ImageMagick_LIBRARIES})
endif (IMAGEMAGICK_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_V4L2_LIBRARIES})
elseif (WIN32)
 if (MSVC)
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"libcmt.lib\"")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"libcpmt.lib\"")
 endif (MSVC)
# target_link_libraries (${EXECUTABLE} dxguid.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)

 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  target_link_libraries (${EXECUTABLE} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Debug/strmbasd.lib)
 else ()
#  target_link_libraries (${EXECUTABLE} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Release/strmbase.lib)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonMath)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTask)
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

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${EXECUTABLE})
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (UNIX)
 install (FILES ${ui_definition_file}
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
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
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 install (FILES ${ui_definition_file} ${EXECUTABLE}.fbp
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

set (ARGS "-l")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}" COMMAND)
 if (GUI_SUPPORT)
  if (GTK_USE)
   set (ARGS "-g./${ui_definition_file} ${ARGS}")
  elseif (WXWIDGETS_USE)
   set (ENVIRONMENT "DISPLAY=:0")
  endif ()
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)

 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
#  message (STATUS "ui_definition_file: ${ui_definition_file}")
  string (REGEX REPLACE "\\\\" "\\\\\\\\" ui_definition_file "${ui_definition_file}")
#  message (STATUS "ui_definition_file: ${ui_definition_file}")
  set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")

  if (GTK_SUPPORT)
   file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             G:/software/Development/dll
                             ${GTK_PATH})
  endif (GTK_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             ${wxWidgets_LIB_DIR})
  endif (WXWIDGETS_SUPPORT)
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)
 file (TO_CMAKE_PATH "${PNG_LIB_DIR}" LIBPNG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${LIBPNG_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${ACE_PATH})
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/avsave
                        ENVIRONMENT ${ENVIRONMENT})

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
