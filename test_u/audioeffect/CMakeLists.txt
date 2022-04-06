include (OpenGL)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl/scripts")
include (assimp)
include (png)

if (UNIX)
 include (ffmpeg) # AVI encoder
endif (UNIX)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec/scripts")
include (mpg123)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
include (sox)

if (UNIX)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)
endif (UNIX)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (MPG123_SUPPORT)
 include_directories (${MPG123_INCLUDE_DIRS})
endif (MPG123_SUPPORT)
if (SOX_SUPPORT)
 include_directories (${SOX_INCLUDE_DIRS})
endif (SOX_SUPPORT)
if (UNIX)
 include_directories (${PKG_ALSA_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)
 if (GUI_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui)
  if (GTK_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/gtk)
   if (GTKGL_SUPPORT)
    include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl)
   endif (GTKGL_SUPPORT)
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)
elseif (WIN32)
 # *NOTE*: workaround
# set (_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      $ENV{LIB_ROOT}/DShowBaseClasses)
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

# DirectShow
# include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})
 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/3rd_party)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 if (GUI_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
  if (GTK_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/gtk)
   if (GTKGL_SUPPORT)
    include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl)
   endif (GTKGL_SUPPORT)
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)
endif ()

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (EXECUTABLE audioeffect)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_u_audioeffect.cpp
     test_u_audioeffect_common.h
     test_u_audioeffect_common_modules.h
     test_u_audioeffect_defines.h
     test_u_audioeffect_eventhandler.cpp
     test_u_audioeffect_eventhandler.h
     test_u_audioeffect_message.cpp
     test_u_audioeffect_message.h
     test_u_audioeffect_module_eventhandler.cpp
     test_u_audioeffect_module_eventhandler.h
     test_u_audioeffect_session_message.cpp
     test_u_audioeffect_session_message.h
     test_u_audioeffect_signalhandler.cpp
     test_u_audioeffect_signalhandler.h
     test_u_audioeffect_stream.cpp
     test_u_audioeffect_stream.h)
if (WIN32)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_u_audioeffect_module_mediafoundation_target.cpp
      test_u_audioeffect_module_mediafoundation_target.h)
endif (WIN32)
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_u_audioeffect_callbacks.cpp
       test_u_audioeffect_callbacks.h)
  if (GTKGL_SUPPORT)
   set (${EXECUTABLE}_EXE_SRCS
        ${${EXECUTABLE}_EXE_SRCS}
        test_u_audioeffect_gl_callbacks.cpp
        test_u_audioeffect_gl_callbacks.h)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
#  set (${EXECUTABLE}_EXE_SRCS
#       ${${EXECUTABLE}_EXE_SRCS}
#       test_u_audioeffect_ui.cpp
#       test_u_audioeffect_ui.h
#       test_u_audioeffect_ui.inl
#       test_u_audioeffect_ui_base.cpp
#       test_u_audioeffect_ui_base.h)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                etc/${EXECUTABLE}.gtk2
                etc/${EXECUTABLE}.gtk3)
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_U)

#add_dependencies (${EXECUTABLE} ACE)
add_dependencies (${EXECUTABLE} Common)
if (WIN32)
 add_dependencies (${EXECUTABLE} CommonError)
endif (WIN32)
add_dependencies (${EXECUTABLE} CommonLog)
add_dependencies (${EXECUTABLE} CommonMath)
add_dependencies (${EXECUTABLE} CommonTimer)
if (GUI_SUPPORT)
 add_dependencies (${EXECUTABLE} CommonUI)
 if (GTK_SUPPORT)
  add_dependencies (${EXECUTABLE} CommonUI_GTK)
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if (MPG123_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${MPG123_LIBRARY})
endif (MPG123_SUPPORT)
if (SOX_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SOX_LIBRARIES})
endif (SOX_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_ALSA_LIBRARIES})
elseif (WIN32)
# set (DIRECTX_SDK_DIRECTORY $ENV{DXSDK_DIR})

 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfuuid.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Dmoguids.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Msdmo.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC strmiids.lib)

 #link_directories ("${DIRECTX_SDK_DIRECTORY}/Lib/x86")
 target_link_libraries (${EXECUTABLE} PUBLIC "$ENV{LIB_ROOT}/DXSDK/Lib/x86/dxguid.lib")

 if (GTKGL_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC OpenGL32.lib)
  target_link_libraries (${EXECUTABLE} PUBLIC GlU32.lib)
 endif (GTKGL_SUPPORT)

 # *WARNING*: linking BaseClasses(d) requires "/FORCE:MULTIPLE"
 target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
endif ()
if (GTKGL_SUPPORT)
 if (GLUT_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC ${GLUT_LIBRARIES})
 endif (GLUT_SUPPORT)
endif (GTKGL_SUPPORT)

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonMath)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTask)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  if (GTKGL_SUPPORT)
   target_link_libraries (${EXECUTABLE} PUBLIC CommonOpenGL)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

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
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/${EXECUTABLE}.gtk2 etc/${EXECUTABLE}.gtk3
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
 string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
 install (FILES ${ACE_DLL_FQPATH}
                ${LIB_ROOT}/gtk3/bin/libatk-1.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libfreetype-6.dll
                ${LIB_ROOT}/gtk3/bin/libintl-8.dll
                ${LIB_ROOT}/gtk3/bin/libcairo-2.dll
                ${LIB_ROOT}/gtk3/bin/libfontconfig-1.dll
                ${LIB_ROOT}/gtk3/bin/libgio-2.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libglib-2.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libgmodule-2.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libgobject-2.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libgthread-2.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libgdk_pixbuf-2.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libgdk-3-0.dll
                ${LIB_ROOT}/gtk3/bin/libgtk-3-0.dll
                ${LIB_ROOT}/gtk3/bin/libpango-1.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libpangocairo-1.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libpangoft2-1.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libpangowin32-1.0-0.dll
                ${LIB_ROOT}/gtk3/bin/libpng15-15.dll
                ${LIB_ROOT}/gtk3/bin/zlib1.dll
                ${LIB_ROOT}/gtk3/bin/libxml2-2.dll
                ${LIB_ROOT}/gtk3/bin/libffi-6.dll
                ${LIB_ROOT}/gtk3/bin/libcairo-gobject-2.dll
                ${LIB_ROOT}/gtk3/bin/libpixman-1-0.dll
                ${LIB_ROOT}/gtk3/bin/libwinpthread-1.dll
                ${LIB_ROOT}/sox/build/src/Debug/libsox.dll
                ${LIB_ROOT}/mpg123/src/libmpg123/.libs/libmpg123-0.dll
                ${LIB_ROOT}/assimp/cmake/bin/Debug/assimp-vc142-mtd.dll
                ${LIB_ROOT}/libpng/build/Debug/libpng16d.dll
                ${LIB_ROOT}/ffmpeg/libswresample/swresample-4.dll
                ${LIB_ROOT}/ffmpeg/libswscale/swscale-6.dll
                ${LIB_ROOT}/ffmpeg/libavcodec/avcodec-59.dll
                ${LIB_ROOT}/ffmpeg/libavutil/avutil-57.dll
                ${LIB_ROOT}/libepoxy/bin/epoxy-0.dll
                ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxbase316d_vc_x64_custom.dll
                ${LIB_ROOT}/zlib/build/Debug/zlibd.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 install (FILES etc/${EXECUTABLE}.gtk2 etc/${EXECUTABLE}.gtk3
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)

endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
    CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
 set (ARGS "-l -t")
else ()
 set (ARGS "")
endif ()
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE}" COMMAND)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTK3_USE OR GTK4_USE)
    set (ARGS "-g./etc/${EXECUTABLE}.gtk3 ${ARGS}")
   elseif (GTK2_USE)
    set (ARGS "-g./etc/${EXECUTABLE}.gtk2 ${ARGS}")
   endif ()
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)
 set (RUNTIME_LIBRARY_DIR ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 set (ARGS "-m ${ARGS}") # use mediafoundation
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTK3_USE OR GTK4_USE)
    set (ARGS "-g.\\\\etc\\\\${EXECUTABLE}.gtk3 ${ARGS}")
   elseif (GTK2_USE)
    set (ARGS "-g.\\\\etc\\\\${EXECUTABLE}.gtk2 ${ARGS}")
   endif ()
   file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
  endif (GTK_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)
 file (TO_CMAKE_PATH "${ASSIMP_LIB_DIR}" ASSIMP_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 if (MPG123_SUPPORT)
  file (TO_CMAKE_PATH "${MPG123_LIB_DIR}" MPG123_PATH)
 endif (MPG123_SUPPORT)
 if (SOX_SUPPORT)
  file (TO_CMAKE_PATH "${SOX_LIB_DIR}" SOX_PATH)
 endif (SOX_SUPPORT)
 file (TO_CMAKE_PATH "${PNG_LIB_DIR}" LIBPNG_PATH)
 file (TO_CMAKE_PATH "$ENV{LIB_ROOT}/zlib/build/${CMAKE_BUILD_TYPE}" ZLIB_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIR ${ASSIMP_PATH}
                          ${FFMPEG_PATH}
                          ${IMAGEMAGICK_PATH}
                          ${LIBPNG_PATH}
                          ${MPG123_PATH}
                          ${SOX_PATH}
                          ${ZLIB_PATH}
                          ${ACE_LIBRARY_PATH})
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   set (RUNTIME_LIBRARY_DIR ${RUNTIME_LIBRARY_DIR}
                            ${GTK_PATH})
  endif (GTK_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   set (RUNTIME_LIBRARY_DIR ${RUNTIME_LIBRARY_DIR}
                            ${WXWIDGETS_PATH})
  endif (WXWIDGETS_SUPPORT)
 endif (GUI_SUPPORT)
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIR}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_u/${EXECUTABLE}
                        ENVIRONMENT ${ENVIRONMENT})

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
