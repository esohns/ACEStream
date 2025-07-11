include (ffmpeg)
include (ImageMagick)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
include (sox)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec/scripts")
include (faad)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif (UNIX)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (UNIX)
  add_compile_options ("-rdynamic")
  add_link_options ("LINKER:--export-dynamic")
endif (UNIX)

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (FAAD_SUPPORT)
 include_directories (${faad_INCLUDE_DIRS})
endif (FAAD_SUPPORT)
if (SOX_SUPPORT)
 include_directories (${SOX_INCLUDE_DIRS})
endif (SOX_SUPPORT)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
elseif (WIN32)
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
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE extract_stream)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_i_common.h
     ../test_i_defines.h
     test_i.cpp
     test_i_common_modules.h
     test_i_eventhandler.cpp
     test_i_eventhandler.h
     test_i_eventhandler.inl
     test_i_extract_stream_common.h
     test_i_extract_stream_defines.h
     test_i_message.cpp
     test_i_message.h
     test_i_message.inl
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_session_message.inl
     test_i_signalhandler.cpp
     test_i_signalhandler.h
     test_i_stream.cpp
     test_i_stream.h)
if (GTK_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_i_gtk_callbacks.cpp
      test_i_gtk_callbacks.h)
endif (GTK_SUPPORT)

set (ui_definition_file "")
if (GTK_USE)
 if (GTK3_USE)
  set (ui_definition_file etc/test_i.gtk3)
 elseif (GTK2_USE)
  set (ui_definition_file etc/test_i.gtk2)
 endif ()
endif (GTK_USE)

add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS}
                ${ui_definition_file})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
#cmake_policy (SET CMP0012 NEW)
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
if (FAAD_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${faad_LIBRARIES})
endif (FAAD_SUPPORT)
if (SOX_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SOX_LIBRARIES})
endif (SOX_SUPPORT)
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonMath)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTask)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)
target_link_libraries (${EXECUTABLE} PUBLIC CommonUI)
if (GTK_USE)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_GTK)
endif (GTK_USE)
if (UNIX)
# target_link_libraries (${EXECUTABLE} PUBLIC CommonXML)
endif (UNIX)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (UNIX)
 install (FILES ${ui_definition_file}
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
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
                 ${LIB_ROOT}/ffmpeg/libavformat/avformat-59.dll
                 ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316d_core_vc_x64_custom.dll
                 ${LIB_ROOT}/wxWidgets/lib/vc_x64_dll/wxmsw316d_xrc_vc_x64_custom.dll
           DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 endif (DEFINED $ENV{LIB_ROOT})
 install (FILES ${ui_definition_file}
#                ${CMAKE_CURRENT_SOURCE_DIR}/etc/${EXECUTABLE}.fbp
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
 if (GTK_USE)
  set (ARGS "-g./${ui_definition_file} ${ARGS}")
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
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
 string (REGEX REPLACE "\\\\" "\\\\\\\\" ui_definition_file "${ui_definition_file}")

 if (GTK_SUPPORT)
  if (GTK_USE)
   set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")
  endif (GTK_USE)

  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS} ${GTK_PATH})
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  if (WXWIDGETS_USE)
   set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")
  endif (WXWIDGETS_USE)

  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS} ${wxWidgets_LIB_DIR})
 endif (WXWIDGETS_SUPPORT)
 if (GTK_USE OR WXWIDGETS_USE)
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GTK_USE OR WXWIDGETS_USE)
# *NOTE*: ffmpeg was built with (vcpkg-)"libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH "${faad_LIB_DIR}" FAAD_PATH)
 file (TO_CMAKE_PATH "${SOX_LIB_DIR}" SOX_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${VCPKG_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${FAAD_PATH}
                           ${SOX_PATH}
                           ${ACE_PATH})
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
