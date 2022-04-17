include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl/scripts;;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl/scripts")
include (assimp)
include (png)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (IMAGEMAGICK_SUPPORT)
  add_definitions (${MAGICK_CXX_FLAGS})
  include_directories (${ImageMagick_INCLUDE_DIRS})
endif (IMAGEMAGICK_SUPPORT)
if (UNIX)
# include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_V4L2_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/test_u)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/test_u)
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
set (EXECUTABLE imagescreen)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_u_common.h
     ../test_u_defines.h
     test_u_imagescreen.cpp
     test_u_imagescreen_common.h
     test_u_imagescreen_common_modules.h
     test_u_imagescreen_defines.h
     test_u_imagescreen_eventhandler.cpp
     test_u_imagescreen_eventhandler.h
     test_u_imagescreen_eventhandler.inl
     test_u_imagescreen_message.cpp
     test_u_imagescreen_message.h
     test_u_imagescreen_message.inl
     test_u_imagescreen_session_message.cpp
     test_u_imagescreen_session_message.h
     test_u_imagescreen_session_message.inl
     test_u_imagescreen_signalhandler.cpp
     test_u_imagescreen_signalhandler.h
     test_u_imagescreen_stream.cpp
     test_u_imagescreen_stream.h)
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_u_imagescreen_gtk_callbacks.cpp
       test_u_imagescreen_gtk_callbacks.h)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_u_imagescreen_ui.cpp
       test_u_imagescreen_ui.h
       test_u_imagescreen_ui.inl
       imagescreen_wxwidgets_ui.cpp
       imagescreen_wxwidgets_ui.h)
#       test_u_imagescreen_ui_base.cpp
#       test_u_imagescreen_ui_base.h)
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
   set (ui_definition_file ${EXECUTABLE}.gtk3)
  elseif (GTK2_USE)
   set (ui_definition_file ${EXECUTABLE}.gtk2)
  endif ()
 elseif (WXWIDGETS_USE)
  if (WIN32)
   add_definitions(-DUNICODE -D_UNICODE)
  endif (WIN32)
  set (ui_definition_file ${EXECUTABLE}.xrc)
 endif ()
endif (GUI_SUPPORT)
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                etc/${ui_definition_file})
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
if (IMAGEMAGICK_SUPPORT)
 add_link_options (${MAGICK_LD_FLAGS})

 target_link_libraries (${EXECUTABLE} PUBLIC ${ImageMagick_LIBRARIES})
endif (IMAGEMAGICK_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_CAP_LIBRARIES})
# target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_V4L2_LIBRARIES})

# if (GUI_SUPPORT)
#  if (WXWIDGETS_USE)
#   target_link_libraries (${EXECUTABLE} PUBLIC ${wxWidgets_LIBRARIES})
#  endif (WXWIDGETS_USE)
# endif (GUI_SUPPORT)
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
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
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
 install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/${ui_definition_file} ${CMAKE_CURRENT_SOURCE_DIR}/etc/${EXECUTABLE}.fbp
          DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
 string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
 install (FILES ${ACE_DLL_FQPATH}
                ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxbase316d_xml_vc_x64_custom.dll
                ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316d_core_vc_x64_custom.dll
                ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316d_html_vc_x64_custom.dll
                ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316d_xrc_vc_x64_custom.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)

 install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/${ui_definition_file} ${CMAKE_CURRENT_SOURCE_DIR}/etc/${EXECUTABLE}.fbp
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
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
   set (ARGS "-g./etc/${ui_definition_file} ${ARGS}")
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
  set (ARGS "-g.\\\\etc\\\\${ui_definition_file} ${ARGS}")

  if (GTK_SUPPORT)
   file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
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
 file (TO_CMAKE_PATH "${ASSIMP_LIB_DIR}" ASSIMP_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${LIBPNG_PATH}
                           ${ASSIMP_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${ACE_PATH})
 file (TO_NATIVE_PATH "${IMAGEMAGICK_PATH}" IMAGEMAGICK_NATIVE_PATH)
 string (REPLACE "\\" "\\\\" IMAGEMAGICK_NATIVE_PATH ${IMAGEMAGICK_NATIVE_PATH})
 set (ENVIRONMENT "MAGICK_CODER_MODULE_PATH=${IMAGEMAGICK_NATIVE_PATH}\\\\modules\\\\coders")
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
