include (ffmpeg)
include (ImageMagick)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/scripts")
include (SSLConfig)

if (UNIX)
 include (FindPkgConfig)
# *NOTE*: Common requires libcap
# pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L2 REQUIRED libv4l2)
# pkg_check_modules (PKG_FFMPEG REQUIRED libswscale libavutil libavcodec)
elseif (WIN32)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
 include (DirectXBaseClasses)
endif ()

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
else ()
 message (STATUS "found package \"ACENetwork\": \"${ACENetwork_DIR}\"")
endif ()
#add_definitions (-DACENETWORK_HAS_DLL)

if (WIN32)
 # *NOTE*: DirectShow uses 'DEBUG'
# add_definitions (-DDEBUG) # activate DirectShow debug logging
 add_compile_options ($<$<CONFIG:Debug>:-DDEBUG>)
endif (WIN32)

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (SSL_SUPPORT)
 include_directories (${SSL_INCLUDE_DIRS})
endif (SSL_SUPPORT)
if (UNIX)
 include_directories (${PKG_V4L2_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/src/client_server)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE_SOURCE camsource)
set (${EXECUTABLE_SOURCE}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_camstream_common.h
     test_i_camstream_defines.h
     test_i_camstream_network.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_source.cpp
     test_i_source_common.h
     test_i_source_eventhandler.cpp
     test_i_source_eventhandler.h
     test_i_source_eventhandler.inl
     test_i_source_message.cpp
     test_i_source_message.h
     test_i_source_session_message.cpp
     test_i_source_session_message.h
     test_i_source_signalhandler.cpp
     test_i_source_signalhandler.h
     test_i_source_signalhandler.inl
     test_i_source_stream.cpp
     test_i_source_stream.h
     test_i_source_stream.inl
     test_i_stream_common.h
     test_i_target_message.cpp
     test_i_target_message.h
     test_i_target_session_message.cpp
     test_i_target_session_message.h
     test_i_target_stream.cpp
     test_i_target_stream.h)
#  test_i_template_instantiations.cpp
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE_SOURCE}_EXE_SRCS
       ${${EXECUTABLE_SOURCE}_EXE_SRCS}
       test_i_callbacks.cpp
       test_i_callbacks.h)
# elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE_TARGET}_EXE_SRCS
#       ${${EXECUTABLE_TARGET}_EXE_SRCS})
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

set (ui_definition_file "")
if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTK3_USE)
   set (ui_definition_file etc/source.gtk3)
   set (ui_definition_file_2 etc/target.gtk3)
  elseif (GTK2_USE)
   set (ui_definition_file etc/source.gtk2)
   set (ui_definition_file_2 etc/target.gtk2)
  endif ()
 elseif (WXWIDGETS_USE)
  if (WIN32)
#   add_definitions(-DUNICODE -D_UNICODE)
   endif (WIN32)
  set (ui_definition_file etc/source.xrc)
  set (ui_definition_file_2 etc/target.xrc)
 endif ()
endif (GUI_SUPPORT)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE_SOURCE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_SOURCE}_EXE_SRCS}
                etc/resources.rc
                ${ui_definition_file})
set_property (TARGET ${EXECUTABLE_SOURCE} PROPERTY FOLDER "test_i")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif (NOT BUILD_TEST_I)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
endif (WIN32)

#add_dependencies (${EXECUTABLE_SOURCE} ACE)

add_dependencies (${EXECUTABLE_SOURCE} Common)
add_dependencies (${EXECUTABLE_SOURCE} CommonImage)
add_dependencies (${EXECUTABLE_SOURCE} CommonLog)
add_dependencies (${EXECUTABLE_SOURCE} CommonTimer)

add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_MediaFramework)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Network)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE_SOURCE} ACENetwork)

if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif (UNIX)
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${ffmpeg_LIBRARIES})
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${ffmpeg_2_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PKG_V4L2_LIBRARIES})
elseif (WIN32)
# set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")

 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC evr.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC mfplat.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC mfuuid.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC strmiids.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC winmm.lib)
endif ()
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC Common)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonUI)
 if (GTK_USE)
  target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonUI_GTK)
 elseif (WXWIDGETS_USE)
  target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC CommonUI_wxWidgets)
 endif ()
endif (GUI_SUPPORT)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ACENetwork)

target_link_libraries (${EXECUTABLE_SOURCE} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE_SOURCE}
                           PUBLIC stdafx.h)

set (RESOURCE_FILES
     etc/resources.rc
     ${ui_definition_file}
     ${ui_definition_file_2})
set_source_files_properties (${RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group (resources FILES ${RESOURCE_FILES})

install (TARGETS ${EXECUTABLE_SOURCE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
         COMPONENT test_i)
if (UNIX)
 install (FILES etc/resources.rc ${ui_definition_file}
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/camstream" COMPONENT config)
elseif (WIN32)
 install (FILES etc/resources.rc ${ui_definition_file}
          DESTINATION "etc/camstream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/camstream" COMPONENT config)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY}
                         DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 set (ACE_SSL_DLL_FILE ACE_SSL${LIB_FILE_SUFFIX}.dll)
 set (ACE_SSL_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_SSL_DLL_FILE})
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
endif ()

#set (COMMAND "$<TARGET_FILE:${EXECUTABLE_SOURCE}>")
set (ENVIRONMENT "")
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SOURCE}" COMMAND)
 if (GTK_SUPPORT)
  set (ARGS "-g./${ui_definition_file} -l -t")
 else ()
  set (ARGS "-g -l -t")
 endif (GTK_SUPPORT)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
 file (TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/test_i/camstream WORKING_DIRECTORY)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SOURCE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  file (TO_NATIVE_PATH "${ui_definition_file}" ui_definition_file)
  string (REGEX REPLACE "\\\\" "/" ui_definition_file "${ui_definition_file}")
  set (ARGS "-g.\\\\${ui_definition_file} ${ARGS}")
  set (ARGS "${ARGS} -l -t")
 else ()
  set (ARGS "-c -g -l -t")
 endif (GUI_SUPPORT)
 # *NOTE*: ffmpeg was built with (vcpkg-)"libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_PATH)
 if (FFMPEG_SUPPORT)
  file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
  file (TO_CMAKE_PATH "${ffmpeg_2_LIB_DIR}" FFMPEG_2_PATH)
 endif (FFMPEG_SUPPORT)
 if (IMAGEMAGICK_SUPPORT)
  file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 endif (IMAGEMAGICK_SUPPORT)
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH "${SSL_LIB_DIR}" SSL_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${IMAGEMAGICK_PATH}
                           ${VCPKG_PATH}
                           ${FFMPEG_PATH}
                           ${FFMPEG_2_PATH}
                           ${GTK_PATH}
                           ${WXWIDGETS_PATH}
                           ${SSL_PATH}
                           ${ACE_PATH})
 string (REPLACE "/" "\\\\" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif ()
create_target_launcher (${EXECUTABLE_SOURCE}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                         FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${WORKING_DIRECTORY}
                        ENVIRONMENT ${ENVIRONMENT})

add_test (NAME ${EXECUTABLE_SOURCE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE_SOURCE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_SOURCE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\ffmpeg\libavcodec;$(ProjectDir)..\..\..\..\ffmpeg\libavresample;$(ProjectDir)..\..\..\..\ffmpeg\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\ACENetwork\cmake\src\Debug

############## next target ################

if (WIN32)
 set (LIBRARY_DS camtarget_source)
 set (${LIBRARY_DS}_LIB_SRCS
#      stdafx.cpp
      stdafx.h
      camtarget_source.cpp
      test_i_target_message.cpp
      test_i_target_message.h)

 add_library (${LIBRARY_DS} MODULE
              EXCLUDE_FROM_ALL
              ${${LIBRARY_DS}_LIB_SRCS}
              etc/${LIBRARY_DS}.def)
 set_property (TARGET ${LIBRARY_DS} PROPERTY FOLDER "test_i")

 target_include_directories (${LIBRARY_DS} PUBLIC ${DS_BASECLASSES_INCLUDE_DIRS})

 add_dependencies (${LIBRARY_DS} Common)
 add_dependencies (${LIBRARY_DS} CommonLog)

 add_dependencies (${LIBRARY_DS} ${PROJECT_NAME})

 set (CMAKE_MODULE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
 set_target_properties (${LIBRARY_DS} PROPERTIES LINK_FLAGS /SAFESEH:NO)
 
 target_link_libraries (${LIBRARY_DS} PUBLIC strmiids.lib)
 target_link_libraries (${LIBRARY_DS} PUBLIC winmm.lib)

 target_link_libraries (${LIBRARY_DS} PUBLIC ${DS_BASECLASSES_LIBRARIES})

 target_link_libraries (${LIBRARY_DS} PUBLIC ${ACE_LIBRARY})

 target_link_libraries (${LIBRARY_DS} PUBLIC Common)
 target_link_libraries (${LIBRARY_DS} PUBLIC CommonLog)

 target_link_libraries (${LIBRARY_DS} PUBLIC ${PROJECT_NAME})
 target_link_libraries (${LIBRARY_DS} PUBLIC ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_DS} PUBLIC ${PROJECT_NAME}_MediaFramework)
 target_link_libraries (${LIBRARY_DS} PUBLIC ${PROJECT_NAME}_Miscellaneous)

 target_link_libraries (${LIBRARY_DS} PUBLIC ${PROJECT_NAME}_Test_I)

 set_target_properties (${LIBRARY_DS} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_DS} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
 target_precompile_headers (${LIBRARY_DS}
                            REUSE_FROM ${EXECUTABLE_SOURCE})

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_DS}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_DS}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 set (PATH_SUFFIX "Release")
 if (DEFINED CMAKE_BUILD_TYPE)
  set (PATH_SUFFIX ${CMAKE_BUILD_TYPE})
 endif (DEFINED CMAKE_BUILD_TYPE)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${LIBRARY_DS}.dll" TARGET_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" TARGET_PATH_NATIVE ${TARGET_PATH_NATIVE})
 file (TO_NATIVE_PATH "${ACE_LIB_DIR}" ACE_LIB_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" ACE_LIB_PATH_NATIVE ${ACE_LIB_PATH_NATIVE})
 create_generic_launcher (regsvr32_directshow
                          COMMAND "regsvr32.exe ${TARGET_PATH_NATIVE}"
                          RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH_NATIVE}
                          WORKING_DIRECTORY ${WORKING_DIRECTORY}
                          ENVIRONMENT ${ENVIRONMENT})

#-----------------------------------------

 set (LIBRARY_MF camtarget_source_mf)
 set (${LIBRARY_MF}_LIB_SRCS
#      stdafx.cpp
      stdafx.h
      camtarget_source_mf.cpp)

 add_library (${LIBRARY_MF} MODULE
              EXCLUDE_FROM_ALL
              ${${LIBRARY_MF}_LIB_SRCS}
              etc/${LIBRARY_MF}.def)
 set_property (TARGET ${LIBRARY_MF} PROPERTY FOLDER "test_i")

 target_include_directories (${LIBRARY_MF} PUBLIC "${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses")

 add_dependencies (${LIBRARY_MF} Common)
 add_dependencies (${LIBRARY_MF} ${PROJECT_NAME})

 target_link_libraries (${LIBRARY_MF} PUBLIC Mf.lib)
 target_link_libraries (${LIBRARY_MF} PUBLIC Mfplat.lib)

 # *TODO*: get rid of this
if (FFMPEG_SUPPORT)
 target_link_libraries (${LIBRARY_MF} PUBLIC ${ffmpeg_LIBRARIES})
 target_link_libraries (${LIBRARY_MF} PUBLIC ${ffmpeg_2_LIBRARIES})
endif (FFMPEG_SUPPORT)

 target_link_libraries (${LIBRARY_MF} PUBLIC ${ACE_LIBRARY})

 target_link_libraries (${LIBRARY_MF} PUBLIC Common)
 target_link_libraries (${LIBRARY_MF} PUBLIC CommonLog)

 target_link_libraries (${LIBRARY_MF} PUBLIC ${PROJECT_NAME})
# target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_MF} PUBLIC ${PROJECT_NAME}_MediaFramework)
# target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Miscellaneous)

 target_link_libraries (${LIBRARY_MF} PUBLIC ${PROJECT_NAME}_Test_I)

 set_target_properties (${LIBRARY_MF} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_MF} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
 target_precompile_headers (${LIBRARY_MF}
                            REUSE_FROM ${EXECUTABLE_SOURCE})

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_MF}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_MF}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PATH_SUFFIX}/${LIBRARY_MF}.dll" TARGET_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" TARGET_PATH_NATIVE ${TARGET_PATH_NATIVE})
 create_generic_launcher (regsvr32_mediafoundation
                          COMMAND "regsvr32.exe ${TARGET_PATH_NATIVE}"
                          RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH_NATIVE}
                          WORKING_DIRECTORY ${WORKING_DIRECTORY}
                          ENVIRONMENT ${ENVIRONMENT})
endif (WIN32)

############## next target ################
set (EXECUTABLE_TARGET camtarget)
set (${EXECUTABLE_TARGET}_EXE_SRCS
#     stdafx.cpp
     stdafx.h
     test_i_camstream_common.h
     test_i_camstream_defines.h
     test_i_camstream_network.h
     test_i_common_modules.h
     test_i_connection_manager_common.h
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_target.cpp
     test_i_target_common.h
     test_i_target_eventhandler.cpp
     test_i_target_eventhandler.h
     test_i_target_eventhandler.inl
     test_i_target_listener_common.h
     test_i_target_message.cpp
     test_i_target_message.h
     test_i_target_session_message.cpp
     test_i_target_session_message.h
     test_i_target_signalhandler.cpp
     test_i_target_signalhandler.h
     test_i_target_signalhandler.inl
     test_i_target_stream.cpp
     test_i_target_stream.h)
#  test_i_template_instantiations.cpp
if (GUI_SUPPORT)
 if (GTK_SUPPORT)
  set (${EXECUTABLE_TARGET}_EXE_SRCS
       ${${EXECUTABLE_TARGET}_EXE_SRCS}
       test_i_callbacks.cpp
       test_i_callbacks.h)
# elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE_TARGET}_EXE_SRCS
#       ${${EXECUTABLE_TARGET}_EXE_SRCS})
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

add_executable (${EXECUTABLE_TARGET}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_TARGET}_EXE_SRCS}
                etc/resources.rc
                ${ui_definition_file_2})
set_property (TARGET ${EXECUTABLE_TARGET} PROPERTY FOLDER "test_i")
if (WIN32)
 cmake_policy (SET CMP0012 NEW)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

#add_dependencies (${EXECUTABLE_TARGET} ACE)
add_dependencies (${EXECUTABLE_TARGET} Common)
add_dependencies (${EXECUTABLE_TARGET} CommonImage)
add_dependencies (${EXECUTABLE_TARGET} CommonLog)
add_dependencies (${EXECUTABLE_TARGET} CommonSignal)
add_dependencies (${EXECUTABLE_TARGET} CommonTimer)

add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_MediaFramework)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Network)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE_TARGET} ACENetwork)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PKG_V4L2_LIBRARIES})

# target_link_libraries (${EXECUTABLE_TARGET} PUBLIC X11)
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC evr.lib)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC strmiids.lib)
 target_link_libraries (${EXECUTABLE_TARGET} PUBLIC winmm.lib)
endif ()
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC Common)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonEvent)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ACENetwork)

target_link_libraries (${EXECUTABLE_TARGET} PUBLIC ${PROJECT_NAME}_Test_I)

# add PCH
target_precompile_headers (${EXECUTABLE_TARGET}
                           REUSE_FROM ${EXECUTABLE_SOURCE})

install (TARGETS ${EXECUTABLE_TARGET}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (UNIX)
 install (FILES etc/resources.rc ${ui_definition_file_2}
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/camstream" COMPONENT config)
elseif (WIN32)
 install (FILES etc/resources.rc ${ui_definition_file_2}
          DESTINATION "etc/camstream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/camstream" COMPONENT config)
endif ()

#cmake_policy (SET CMP0053 OLD)
set (ARGS "")
#set (COMMAND "$<TARGET_FILE:${EXECUTABLE_TARGET}>")
file (TO_NATIVE_PATH ${PROJECT_SOURCE_DIR}/test_i/camstream WORKING_DIRECTORY)
set (ENVIRONMENT "")
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_TARGET}" COMMAND)
 set (ARGS "-g./${ui_definition_file_2} -l -o -t")
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_TARGET}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GUI_SUPPORT)
  file (TO_NATIVE_PATH "${ui_definition_file_2}" ui_definition_file_2)
  string (REGEX REPLACE "\\\\" "/" ui_definition_file_2 "${ui_definition_file_2}")
  set (ARGS "-g.\\\\${ui_definition_file_2} ${ARGS}")
  set (ARGS "${ARGS} -l -o -t")
 else ()
  set (ARGS "-c -g -l -o -t")
 endif (GUI_SUPPORT)
 string (REPLACE "\\" "\\\\" WORKING_DIRECTORY ${WORKING_DIRECTORY})
endif ()
create_target_launcher (${EXECUTABLE_TARGET}
                        COMMAND ${COMMAND}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${WORKING_DIRECTORY}
                        ENVIRONMENT ${ENVIRONMENT})

add_test (NAME ${EXECUTABLE_TARGET}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_TARGET}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\ffmpeg\libavcodec;$(ProjectDir)..\..\..\..\ffmpeg\libavresample;$(ProjectDir)..\..\..\..\ffmpeg\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\ACENetwork\cmake\src\Debug
