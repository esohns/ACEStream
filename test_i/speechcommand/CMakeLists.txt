include (OpenGL)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl/scripts")
include (assimp)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/image/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image/scripts")
include (png)

if (UNIX)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)
endif (UNIX)
include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
include (sox)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec/scripts")
include (deepspeech)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)
#find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
#              REQUIRED
#              CONFIG
#              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
#              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
#              NO_DEFAULT_PATH)
#if (NOT EXISTS ${Common_DIR})
# message (FATAL_ERROR "could not find package \"Common\", aborting")
#endif ()
#add_definitions (-DCOMMON_HAS_DLL)

if (UNIX)
 include_directories (${PKG_ALSA_INCLUDE_DIRS})
 include_directories (${SOX_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/input)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
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

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/input)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
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

include_directories (${CMAKE_BINARY_DIR})

if (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)
endif (WIN32)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE speechcommand)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_i_common.h
     ../test_i_defines.h
     test_i.cpp
     test_i_speechcommand_common.h
     test_i_modules.h
     test_i_speechcommand_defines.h
     test_i_eventhandler.cpp
     test_i_eventhandler.h
     test_i_eventhandler.inl
     test_i_message.cpp
     test_i_message.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_session_message.inl
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i_stream.cpp
     test_i_stream.h
     test_i_stream_common.h)
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_i_gtk_callbacks.cpp
       test_i_gtk_callbacks.h)
  if (GTKGL_SUPPORT)
   set (${EXECUTABLE}_EXE_SRCS
        ${${EXECUTABLE}_EXE_SRCS}
        test_i_gl_callbacks.cpp
        test_i_gl_callbacks.h)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS})
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)

if (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions (-DD3D_DEBUG_INFO)
  # *NOTE*: DirectShow uses 'DEBUG'
  add_definitions (-DDEBUG) # activate DirectShow debug logging
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
# add_definitions (-DSWIG) # *WORKAROUND*
endif ()

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                ${ui_definition_file})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_I)

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
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_I)

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_ALSA_LIBRARIES})
 target_link_libraries (${EXECUTABLE} PUBLIC ${SOX_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfuuid.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Dmoguids.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Msdmo.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC strmiids.lib)

 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTKGL_SUPPORT)
    target_link_libraries (${EXECUTABLE} PUBLIC OpenGL32.lib)
    target_link_libraries (${EXECUTABLE} PUBLIC GlU32.lib)
   endif (GTKGL_SUPPORT)
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)

 if (FFMPEG_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC $ENV{LIB_ROOT}/ffmpeg/lib/avfilter.lib)
 endif (FFMPEG_SUPPORT)
endif ()

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE} PUBLIC CommonInput)
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

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

#if (UNIX)
# if (GUI_SUPPORT)
#  if (WXWIDGETS_USE)
#   target_link_libraries (${EXECUTABLE} PUBLIC ${wxWidgets_LIBRARIES})
#  endif (WXWIDGETS_USE)
# endif (GUI_SUPPORT)
#endif (UNIX)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

set (model_file "deepspeech-0.9.3-models.pbmm")
set (scorer_file "deepspeech-0.9.3-models.scorer")
set (model_file "etc/${model_file}")
set (scorer_file "etc/${scorer_file}")
set (ui_definition_file "")
if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTK3_USE)
   set (ui_definition_file etc/test_i.gtk3)
  elseif (GTK2_USE)
   set (ui_definition_file etc/test_i.gtk2)
  endif ()
 elseif (WXWIDGETS_USE)
  set (ui_definition_file etc/test_i.xrc)
 endif ()
endif (GUI_SUPPORT)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
#install (FILES ${model_file} ${scorer_file}
#         DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT data)
if (GUI_SUPPORT)
 install (FILES ${ui_definition_file}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
endif (GUI_SUPPORT)
if (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
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
                ${LIB_ROOT}/DeepSpeech/native_client/windows/bin/libdeepspeech.so
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 install (FILES ${ui_definition_file}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

if (UNIX)
 set (ARGS "-c ./${scorer_file} -f ./${model_file}")
endif (UNIX)
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
    CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
 set (ARGS "${ARGS} -l -t")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
       CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE}" COMMAND)
 if (GUI_SUPPORT)
  if (GTK_USE OR WXWIDGETS_USE)
   set (ARGS "${ARGS} -g./${ui_definition_file}")
  endif (GTK_USE OR WXWIDGETS_USE)
  if (GTK_USE)
   set (ENVIRONMENT "GTK_DEBUG=interactive")
  elseif (WXWIDGETS_USE)
   set (ENVIRONMENT "DISPLAY=:0")
  endif ()
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 file (TO_NATIVE_PATH ".\\${scorer_file}" scorer_file)
 string (REPLACE "\\" "\\\\" scorer_file ${scorer_file})
# message (STATUS "scorer_file: \"${scorer_file}\"")
 file (TO_NATIVE_PATH ".\\${model_file}" model_file)
 string (REPLACE "\\" "\\\\" model_file ${model_file})
 set (ARGS "${ARGS} -c ${scorer_file} -f ${model_file}")
 set (ARGS "-m ${ARGS}") # use mediafoundation
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  if ("${ui_definition_file}" STREQUAL "")
   set (ARGS " ${ARGS} -g")
  else ()
   file (TO_NATIVE_PATH ".\\${ui_definition_file}" ui_definition_file)
   string (REPLACE "\\" "\\\\" ui_definition_file "${ui_definition_file}")
#   message (STATUS "ui_definition_file: \"${ui_definition_file}\"")
   set (ARGS " ${ARGS} -g${ui_definition_file}")
  endif ("${ui_definition_file}" STREQUAL "")
  if (GTK_SUPPORT)
#   set (ENVIRONMENT "GTK_DEBUG=interactive$<COMMA>size-request")
   file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             ${GTK_PATH})
  endif (GTK_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             ${WXWIDGETS_PATH})
  endif (WXWIDGETS_SUPPORT)
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${ASSIMP_LIB_DIR}" ASSIMP_PATH)
 file (TO_CMAKE_PATH "${PNG_LIB_DIR}" LIBPNG_PATH)
 file (TO_CMAKE_PATH "${DEEPSPEECH_LIB_DIR}" DEEPSPEECH_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIBRARY_PATH}
                           ${ASSIMP_PATH}
                           ${LIBPNG_PATH}
                           ${DEEPSPEECH_LIBRARY_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH})
 if (SOX_SUPPORT)
  file (TO_CMAKE_PATH "${SOX_LIB_DIR}" SOX_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${SOX_PATH})
 endif (SOX_SUPPORT)
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE}
                        ENVIRONMENT ${ENVIRONMENT})

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
