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

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/src/protocol/pop)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/pop)
endif ()

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE POP_receive)
set (${EXECUTABLE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     ../test_i_common.h
     ../test_i_defines.h
     test_i_pop_receive.cpp
     test_i_pop_receive_common.h
     test_i_pop_receive_common_modules.h
     test_i_pop_receive_defines.h
     test_i_pop_receive_eventhandler.cpp
     test_i_pop_receive_eventhandler.h
     test_i_pop_receive_eventhandler.inl
     test_i_pop_receive_signalhandler.cpp
     test_i_pop_receive_signalhandler.h
     test_i_pop_receive_stream.cpp
     test_i_pop_receive_stream.h
     test_i_pop_receive_stream_common.h
     test_i_pop_receive_network.h)
if (GUI_SUPPORT)
 if (GTK_USE)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_i_pop_receive_gtk_callbacks.cpp
       test_i_pop_receive_gtk_callbacks.h)
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
 # *NOTE*: DirectShow uses 'DEBUG'
# add_definitions (-DDEBUG) # activate DirectShow debug logging
 add_compile_options ($<$<CONFIG:Debug>:-DDEBUG>)
 add_compile_options ($<$<CONFIG:Debug>:-DD3D_DEBUG_INFO>)
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
#   add_definitions(-DUNICODE -D_UNICODE)
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

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})
if (SSL_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_SSL_LIBRARY})
endif (SSL_SUPPORT)

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonEvent)
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
target_link_libraries (${EXECUTABLE} PUBLIC ACENetwork_Protocol_POP)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Test_I)

if (UNIX)
 if (GUI_SUPPORT)
  if (WXWIDGETS_USE)
   target_link_libraries (${EXECUTABLE} PUBLIC ${wxWidgets_LIBRARIES})
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
 set (ACE_SSL_DLL_FILE ACE_SSL${LIB_FILE_SUFFIX}.dll)
 set (ACE_SSL_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_SSL_DLL_FILE})
 file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
 string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
 install (FILES ${ACE_DLL_FQPATH}
                ${ACE_SSL_DLL_FQPATH}
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
                ${LIB_ROOT}/openssl/libcrypto-3-x64.dll
                ${LIB_ROOT}/openssl/libssl-3-x64.dll
          DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
 install (FILES ${ui_definition_file}
          DESTINATION "etc/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

set (ARGS "-h pop3.web.de -i 995 -l -r -u sohnse")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE}" COMMAND)
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
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
#  message (STATUS "ui_definition_file: ${ui_definition_file}")
  string (REGEX REPLACE "\\\\" "\\\\\\\\" ui_definition_file "${ui_definition_file}")
#  message (STATUS "ui_definition_file: ${ui_definition_file}")

  if (GTK_SUPPORT)
   file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             ${GTK_PATH})
   if (GTK3_USE)
    set (ARGS " ${ARGS} -g.\\\\\\\\${ui_definition_file}")
   elseif (GTK2_USE)
    set (ARGS " ${ARGS} -g.\\\\\\\\${ui_definition_file}")
   endif ()
  endif (GTK_SUPPORT)
  if (WXWIDGETS_SUPPORT)
   if (WXWIDGETS_USE)
    set (ARGS " ${ARGS} -g.\\\\\\\\${ui_definition_file}")
   endif (WXWIDGETS_USE)
   set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                             ${wxWidgets_LIB_DIR})
  endif (WXWIDGETS_SUPPORT)
 else ()
  set (ARGS "-g ${ARGS}")
 endif (GUI_SUPPORT)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/openssl OPENSSL_PATH)
# file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libepoxy/bin EPOXY_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ACE_TAO/ACE/lib ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${OPENSSL_PATH}
#                           ${EPOXY_PATH}
                           ${ACE_PATH})
endif ()
create_target_launcher (${EXECUTABLE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/pop_receive
                        ENVIRONMENT "${ENVIRONMENT}")

##########################################

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
