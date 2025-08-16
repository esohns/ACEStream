if (UNIX)
 pkg_check_modules (PKG_ALSA REQUIRED alsa)
endif (UNIX)
include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
if (WIN32)
 include (DirectXBaseClasses)
endif (WIN32)
include (sox)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec/scripts")
include (festival)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (UNIX)
 include_directories (${PKG_ALSA_INCLUDE_DIRS})
 include_directories (${SOX_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/input)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui)
 if (GTK_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/gtk)
  if (GTKGL_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
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
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
 if (GTK_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/gtk)
  if (GTKGL_SUPPORT)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif ()

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (EXECUTABLE commandspeech)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_i_common.h
     ../test_i_defines.h
     test_i.cpp
     test_i_commandspeech_common.h
     test_i_commandspeech_defines.h
     test_i_eventhandler.cpp
     test_i_eventhandler.h
     test_i_eventhandler.inl
     test_i_input_handler.cpp
     test_i_input_handler.h
     test_i_input_handler.inl
     test_i_message.cpp
     test_i_message.h
     test_i_modules.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_session_message.inl
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i_stream.cpp
     test_i_stream.h
     test_i_stream_common.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_gtk_callbacks.cpp
      test_i_gtk_callbacks.h)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS})
endif (WXWIDGETS_SUPPORT)

if (WIN32)
 # *NOTE*: DirectShow uses 'DEBUG'
# add_definitions (-DDEBUG) # activate DirectShow debug logging
 add_compile_options ($<$<CONFIG:Debug>:-DDEBUG>)
 add_compile_options ($<$<CONFIG:Debug>:-DD3D_DEBUG_INFO>)
# add_definitions (-DSWIG) # *WORKAROUND*
endif ()

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                ${ui_definition_file})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_I)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif (WIN32 AND NOT BUILD_TEST_I)

if (WIN32)
 if (FESTIVAL_SUPPORT)
# at this point festival only supports static linking on win32 platforms
#  set_property (TARGET ${EXECUTABLE} PROPERTY
#                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
 endif (FESTIVAL_SUPPORT)
endif (WIN32)

#add_dependencies (${EXECUTABLE} ACE)

add_dependencies (${EXECUTABLE} Common)
if (WIN32)
 add_dependencies (${EXECUTABLE} CommonError)
endif (WIN32)
add_dependencies (${EXECUTABLE} CommonInput)
add_dependencies (${EXECUTABLE} CommonLog)
add_dependencies (${EXECUTABLE} CommonMath)
add_dependencies (${EXECUTABLE} CommonTimer)
add_dependencies (${EXECUTABLE} CommonUI)
if (GTK_SUPPORT)
 add_dependencies (${EXECUTABLE} CommonUI_GTK)
endif (GTK_SUPPORT)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_I)

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_ALSA_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfuuid.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Dmoguids.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC Msdmo.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC strmiids.lib)

 if (FESTIVAL_SUPPORT)
#  target_link_libraries (${EXECUTABLE} PUBLIC ${FESTIVAL_LIBRARIES})
# *WARNING*: linking libFestival.lib requires "/FORCE:MULTIPLE"
#  target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
 endif (FESTIVAL_SUPPORT)
 if (FLITE_SUPPORT)
# *WARNING*: linking libflite.lib requires "/FORCE:MULTIPLE"
  target_link_options (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-FORCE:MULTIPLE>")
 endif (FLITE_SUPPORT)
endif ()
if (FLITE_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${FLITE_LIBRARIES})
endif (FLITE_SUPPORT)
if (SOX_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SOX_LIBRARIES})
endif (SOX_SUPPORT)

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonInput)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonMath)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTask)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)
target_link_libraries (${EXECUTABLE} PUBLIC CommonUI)
if (GTK_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_GTK)
endif (GTK_SUPPORT)
if (WXWIDGETS_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_wxWidgets)
endif (WXWIDGETS_SUPPORT)

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
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

set (ui_definition_file "")
if (GTK_USE)
 if (GTK3_USE)
  set (ui_definition_file etc/test_i.gtk3)
 elseif (GTK2_USE)
  set (ui_definition_file etc/test_i.gtk2)
 endif ()
elseif (WXWIDGETS_USE)
 set (ui_definition_file etc/test_i.xrc)
endif ()

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (UNIX)
 install (FILES ${ui_definition_file}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 if (DEFINED $ENV{LIB_ROOT})
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
           DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 endif (DEFINED $ENV{LIB_ROOT})
 install (FILES ${ui_definition_file}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

set (VOICE_DIRECTORY "$ENV{LIB_ROOT}/flite/voices")
file (TO_NATIVE_PATH "${VOICE_DIRECTORY}" VOICE_DIRECTORY)
set (VOICE "cmu_us_slt")
if (DEFINED CMAKE_BUILD_TYPE)
 if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR
     ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  set (ARGS "${ARGS} -l -t")
 endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR
        ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
endif (DEFINED CMAKE_BUILD_TYPE)
if (UNIX)
 set (ARGS "${ARGS} -b ${VOICE} -c ${VOICE_DIRECTORY}")
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE}" COMMAND)
 if (GTK_USE OR WXWIDGETS_USE)
  set (ARGS "${ARGS} -g./${ui_definition_file}")
 endif (GTK_USE OR WXWIDGETS_USE)
 if (GTK_USE)
  set (ENVIRONMENT "GTK_DEBUG=interactive")
 elseif (WXWIDGETS_USE)
  set (ENVIRONMENT "DISPLAY=:0")
 endif ()
 if (GTK_USE OR WXWIDGETS_USE)
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GTK_USE OR WXWIDGETS_USE)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 string (REPLACE "\\" "\\\\" VOICE_DIRECTORY ${VOICE_DIRECTORY})
 set (ARGS "${ARGS} -b ${VOICE} -c ${VOICE_DIRECTORY}")
 set (ARGS "-m ${ARGS}") # use mediafoundation
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if ("${ui_definition_file}" STREQUAL "")
  set (ARGS " ${ARGS} -g")
 else ()
  file (TO_NATIVE_PATH ".\\${ui_definition_file}" ui_definition_file)
  string (REPLACE "\\" "\\\\" ui_definition_file "${ui_definition_file}")
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
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 file (TO_CMAKE_PATH "${ASSIMP_LIB_DIR}" ASSIMP_PATH)
 file (TO_CMAKE_PATH "${PNG_LIB_DIR}" LIBPNG_PATH)
 file (TO_CMAKE_PATH "${FLITE_LIB_DIR}" FLITE_LIBRARY_PATH)
  # *NOTE*: ffmpeg was built with (vcpkg-)"libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ACE_LIBRARY_PATH}
                           ${ASSIMP_PATH}
                           ${LIBPNG_PATH}
                           ${FLITE_LIBRARY_PATH}
                           ${VCPKG_PATH}
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
