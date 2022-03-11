set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/scripts")
include (SSLConfig)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)
find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES Common/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()
add_definitions (-DCOMMON_HAS_DLL)

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui)
 if (GUI_SUPPORT)
  if (QT_USE)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/qt)
  endif (QT_USE)
 endif (GUI_SUPPORT)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/src/protocol/smtp)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/smtp)
endif ()

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE SMTP_send)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_i_common.h
     ../test_i_defines.h
     test_i_smtp_send.cpp
     test_i_smtp_send_common.h
     test_i_smtp_send_common_modules.h
     test_i_smtp_send_defines.h
     test_i_smtp_send_eventhandler.cpp
     test_i_smtp_send_eventhandler.h
     test_i_smtp_send_eventhandler.inl
     test_i_smtp_send_signalhandler.cpp
     test_i_smtp_send_signalhandler.h
     test_i_smtp_send_stream.cpp
     test_i_smtp_send_stream.h
     test_i_smtp_send_stream_common.h
     test_i_smtp_send_network.h)
if (GUI_SUPPORT)
 if (GTK_USE)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_i_smtp_send_gtk_callbacks.cpp
       test_i_smtp_send_gtk_callbacks.h)
 elseif (WXWIDGETS_USE)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS})
 endif ()
endif (GUI_SUPPORT)

if (UNIX)
# add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
 if (GUI_SUPPORT)
  if (GTK_USE)
   if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions (-DG_ERRORCHECK_MUTEXES)
   endif (CMAKE_BUILD_TYPE STREQUAL Debug)
  endif (GTK_USE)
 endif (GUI_SUPPORT)
elseif (WIN32)
# if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
#  add_definitions (-DD3D_DEBUG_INFO)
  # *NOTE*: DirectShow uses 'DEBUG'
#  add_definitions (-DDEBUG) # activate DirectShow debug logging
# endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
endif ()

#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property

set (ui_definition_file "")
if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTK3_USE)
   set (ui_definition_file etc/test_i.gtk3)
  elseif (GTK2_USE)
   set (ui_definition_file etc/test_i.gtk2)
  endif ()
 elseif (WXWIDGETS_USE)
  if (WIN32)
   add_definitions(-DUNICODE -D_UNICODE)
  endif (WIN32)
  set (ui_definition_file etc/test_i.xrc)
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
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Network)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_I)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonSignal)
target_link_libraries (${EXECUTABLE} CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE} CommonUI)
 if (GTK_USE)
  target_link_libraries (${EXECUTABLE} CommonUI_GTK)
 elseif (WXWIDGETS_USE)
  target_link_libraries (${EXECUTABLE} CommonUI_wxWidgets)
 endif ()
endif (GUI_SUPPORT)
target_link_libraries (${EXECUTABLE} ACENetwork_Protocol_SMTP)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Test_I)

if (UNIX)
 if (GUI_SUPPORT)
  if (WXWIDGETS_USE)
   target_link_libraries (${EXECUTABLE} ${wxWidgets_LIBRARIES})
  endif (WXWIDGETS_USE)
 endif (GUI_SUPPORT)
endif (UNIX)

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
                ${Common_DIR}/src/${CMAKE_BUILD_TYPE}/Common.dll
                ${Common_DIR}/src/ui/${CMAKE_BUILD_TYPE}/CommonUI.dll
                ${ACENetwork_DIR}/src/${CMAKE_BUILD_TYPE}/ACENetwork.dll
                ${ACENetwork_DIR}/src/protocol/http/${CMAKE_BUILD_TYPE}/ACENetwork_Protocol_HTTP.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/../../../libmysql_2/${BUILD_PATH_SUFFIX}/libmysql/Debug/libmysql.dll
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
 if (GUI_SUPPORT)
  if (GTK_USE)
   set (ARGS "${ARGS} -g./${ui_definition_file}")
  elseif (WXWIDGETS_USE)
   set (ENVIRONMENT "DISPLAY=:0")
  endif ()
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)

 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 if (GUI_SUPPORT)
  file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
#  message (STATUS "ui_definition_file: ${ui_definition_file}")
  string (REGEX REPLACE "\\\\" "\\\\\\\\" ui_definition_file "${ui_definition_file}")
#  message (STATUS "ui_definition_file: ${ui_definition_file}")
  set (ARGS " ${ARGS} -g.\\\\${ui_definition_file}")

  if (GTK_USE)
   if (GTK3_USE)
    file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk3/bin GTK_LIB_PATH)
    set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                              G:/software/Development/dll
                              ${GTK_LIB_PATH})
   elseif (GTK2_USE)
    file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_LIB_PATH)
    set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                              G:/software/Development/dll
                              ${GTK_LIB_PATH})
   endif ()
  elseif (WXWIDGETS_USE)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             ${wxWidgets_LIB_DIR})
  endif ()
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl OPENSSL_PATH)
 #file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_PATH)
 #file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ImageMagick IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${OPENSSL_PATH}
                           ${ACE_PATH})
endif ()
create_target_launcher (${EXECUTABLE}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/smtp_send
                        ENVIRONMENT ${ENVIRONMENT})

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
