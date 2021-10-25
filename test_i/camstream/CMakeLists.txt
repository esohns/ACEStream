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
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      "$ENV{LIB_ROOT}/DShowBaseClasses")
#      "$ENV{PROGRAMFILES(X86)}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow")
endif ()

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACENetwork/${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
else ()
 message (STATUS "found package \"ACENetwork\": \"${ACENetwork_DIR}\"")
endif ()
add_definitions (-DACENETWORK_HAS_DLL)

if (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))
 # *NOTE*: DirectShow uses 'DEBUG'
 add_definitions (-DDEBUG) # activate DirectShow debug logging
endif (WIN32 AND (CMAKE_BUILD_TYPE STREQUAL Debug))

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (UNIX)
# include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_V4L2_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/${BUILD_PATH_SUFFIX})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../ACENetwork/src/client_server)
elseif (WIN32)
 if (OPENSSL_SUPPORT)
  include_directories (${SSL_INCLUDE_DIRS})
 endif (OPENSSL_SUPPORT)

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
     stdafx.cpp
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
 if (GTK_USE)
  set (${EXECUTABLE_SOURCE}_EXE_SRCS
       ${${EXECUTABLE_SOURCE}_EXE_SRCS}
       test_i_callbacks.cpp
       test_i_callbacks.h)
# elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE_TARGET}_EXE_SRCS
#       ${${EXECUTABLE_TARGET}_EXE_SRCS})
 endif (GTK_USE)
endif (GUI_SUPPORT)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE_SOURCE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_SOURCE}_EXE_SRCS}
                etc/resources.rc
                etc/source.glade)
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

target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_THREAD_LIBS_INIT})
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE_SOURCE} ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (UNIX)
# target_link_libraries (${EXECUTABLE_SOURCE} rt)
# target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_CAP_LIBRARIES})

 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_V4L2_LIBRARIES})
elseif (WIN32)
# set (CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")

 target_link_libraries (${EXECUTABLE_SOURCE} evr.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} mfplat.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} strmiids.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} winmm.lib)
endif ()
target_link_libraries (${EXECUTABLE_SOURCE} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_SOURCE} Common)
target_link_libraries (${EXECUTABLE_SOURCE} CommonImage)
target_link_libraries (${EXECUTABLE_SOURCE} CommonLog)
target_link_libraries (${EXECUTABLE_SOURCE} CommonSignal)
target_link_libraries (${EXECUTABLE_SOURCE} CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE_SOURCE} CommonUI)
 if (GTK_USE)
  target_link_libraries (${EXECUTABLE_SOURCE} CommonUI_GTK)
 elseif (WXWIDGETS_USE)
  target_link_libraries (${EXECUTABLE_SOURCE} CommonUI_wxWidgets)
 endif ()
endif (GUI_SUPPORT)

target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork)
#target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork_Client)

target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Test_I)

# add PCH
#set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#if (UNIX)
# cotire (${EXECUTABLE_SOURCE})
#endif (UNIX)
target_precompile_headers (${EXECUTABLE_SOURCE}
                           PUBLIC stdafx.h)

set (RESOURCE_FILES
     etc/resources.rc
     etc/source.glade
     etc/target.glade)
set_source_files_properties (${RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group (resources FILES ${RESOURCE_FILES})

install (TARGETS ${EXECUTABLE_SOURCE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
         COMPONENT test_i)
if (UNIX)
 install (FILES etc/source.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/camstream" COMPONENT config)
elseif (WIN32)
 install (FILES etc/source.glade
          DESTINATION "data/${PROJECT_NAME}/camstream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/camstream" COMPONENT config)
endif ()

#guess_runtime_library_dirs (<outputvarname> [<extralibrary> ...])
#cmake_policy (SET CMP0053 OLD)
set (COMMAND "$<TARGET_FILE:${EXECUTABLE_SOURCE}>")
set (ENVIRONMENT "")
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SOURCE}" COMMAND)
 if (GTK_SUPPORT)
  set (ARGS "-g./etc/source.glade -l -t")
 else ()
  set (ARGS "-g -l -t")
 endif (GTK_SUPPORT)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
 file (TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/../test_i/camstream WORKING_DIRECTORY)
elseif (WIN32)
# file (TO_NATIVE_PATH "${COMMAND}" COMMAND)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_SOURCE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GTK_SUPPORT)
  set (ARGS "-g.\\\\etc\\\\source.glade -l -t")
 else ()
  set (ARGS "-c -g -l -t")
 endif (GTK_SUPPORT)
 if (FFMPEG_SUPPORT)
  file (TO_CMAKE_PATH "$ENV{LIB_ROOT}/ffmpeg/bin" FFMPEG_PATH)
 endif (FFMPEG_SUPPORT)
 if (IMAGEMAGICK_SUPPORT)
  file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 endif (IMAGEMAGICK_SUPPORT)
 file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 file (TO_CMAKE_PATH "${SSL_LIB_DIR}" SSL_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${GTK_PATH}
                           ${SSL_PATH}
                           G:/software/Development/dll
                           ${ACE_PATH})
 string (REPLACE "/" "\\\\" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream) 
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
      stdafx.cpp
      stdafx.h
      camtarget_source.cpp
      test_i_target_message.cpp
      test_i_target_message.h)

 add_library (${LIBRARY_DS} MODULE
              EXCLUDE_FROM_ALL
              ${${LIBRARY_DS}_LIB_SRCS}
              etc/${LIBRARY_DS}.def)
 set_property (TARGET ${LIBRARY_DS} PROPERTY FOLDER "test_i")

 target_include_directories (${LIBRARY_DS} PUBLIC "${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses")

 add_dependencies (${LIBRARY_DS} Common)
 add_dependencies (${LIBRARY_DS} CommonLog)

 add_dependencies (${LIBRARY_DS} ${PROJECT_NAME})

 set (CMAKE_MODULE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\")
 set_target_properties (${LIBRARY_DS} PROPERTIES LINK_FLAGS /SAFESEH:NO)
 
 target_link_libraries (${LIBRARY_DS} strmiids.lib)
 target_link_libraries (${LIBRARY_DS} winmm.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${LIBRARY_DS} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Debug/BaseClassesd.lib)
#  target_link_libraries (${LIBRARY_DS} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${LIBRARY_DS} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/cmake/baseclasses/Release/BaseClasses.lib)
 endif (CMAKE_BUILD_TYPE STREQUAL Debug)
 target_link_libraries (${LIBRARY_DS} ${ACE_LIBRARY})

 target_link_libraries (${LIBRARY_DS} Common)
 target_link_libraries (${LIBRARY_DS} CommonLog)

 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_MediaFramework)
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_Miscellaneous)

 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_Test_I)

 set_target_properties (${LIBRARY_DS} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_DS} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
# set_target_properties (${LIBRARY_DS} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
# set_target_properties (${LIBRARY_DS} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
# cotire (${LIBRARY_DS})
 target_precompile_headers (${LIBRARY_DS}
                            PUBLIC stdafx.h)

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_DS}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_DS}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 file (TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/test_i/camstream/${CMAKE_BUILD_TYPE}/${LIBRARY_DS}.dll" TARGET_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" TARGET_PATH_NATIVE ${TARGET_PATH_NATIVE})
 file (TO_NATIVE_PATH "${ACE_LIB_DIR}" ACE_LIB_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" ACE_LIB_PATH_NATIVE ${ACE_LIB_PATH_NATIVE})
 create_generic_launcher (regsvr32_directshow
                          COMMAND regsvr32.exe ${TARGET_PATH_NATIVE}
                          RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH_NATIVE}
                          WORKING_DIRECTORY ${WORKING_DIRECTORY}
                          ENVIRONMENT ${ENVIRONMENT})

#-----------------------------------------

 set (LIBRARY_MF camtarget_source_mf)
 set (${LIBRARY_MF}_LIB_SRCS
      stdafx.cpp
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

 target_link_libraries (${LIBRARY_MF} ${ACE_LIBRARY})

 target_link_libraries (${LIBRARY_MF} Common)
 target_link_libraries (${LIBRARY_MF} CommonLog)

 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_MediaFramework)
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Miscellaneous)

 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Test_I)

 set_target_properties (${LIBRARY_MF} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_MF} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
# set_target_properties (${LIBRARY_MF} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
# set_target_properties (${LIBRARY_MF} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
# cotire (${LIBRARY_MF})
 target_precompile_headers (${LIBRARY_MF}
                            PUBLIC stdafx.h)

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_MF}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_MF}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)

 file (TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/test_i/camstream/${CMAKE_BUILD_TYPE}/${LIBRARY_MF}.dll" TARGET_PATH_NATIVE)
 string (REPLACE "\\" "\\\\" TARGET_PATH_NATIVE ${TARGET_PATH_NATIVE})
 create_generic_launcher (regsvr32_mediafoundation
                          COMMAND regsvr32.exe ${TARGET_PATH_NATIVE}
                          RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH_NATIVE}
                          WORKING_DIRECTORY ${WORKING_DIRECTORY}
                          ENVIRONMENT ${ENVIRONMENT})
endif (WIN32)

############## next target ################
set (EXECUTABLE_TARGET camtarget)
set (${EXECUTABLE_TARGET}_EXE_SRCS
     stdafx.cpp
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
 if (GTK_USE)
  set (${EXECUTABLE_TARGET}_EXE_SRCS
       ${${EXECUTABLE_TARGET}_EXE_SRCS}
       test_i_callbacks.cpp
       test_i_callbacks.h)
# elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE_TARGET}_EXE_SRCS
#       ${${EXECUTABLE_TARGET}_EXE_SRCS})
 endif (GTK_USE)
endif (GUI_SUPPORT)

add_executable (${EXECUTABLE_TARGET}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE_TARGET}_EXE_SRCS}
                etc/resources.rc
                etc/target.glade)
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

target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_THREAD_LIBS_INIT})
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET} ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_V4L2_LIBRARIES})

 target_link_libraries (${EXECUTABLE_TARGET} X11)
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_TARGET} evr.lib)
 target_link_libraries (${EXECUTABLE_TARGET} strmiids.lib)
 target_link_libraries (${EXECUTABLE_TARGET} winmm.lib)
endif ()
target_link_libraries (${EXECUTABLE_TARGET} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE_TARGET} Common)
target_link_libraries (${EXECUTABLE_TARGET} CommonImage)
target_link_libraries (${EXECUTABLE_TARGET} CommonLog)
target_link_libraries (${EXECUTABLE_TARGET} CommonSignal)
target_link_libraries (${EXECUTABLE_TARGET} CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE_TARGET} CommonUI)
 if (GTK_SUPPORT)
  target_link_libraries (${EXECUTABLE_TARGET} CommonUI_GTK)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_link_libraries (${EXECUTABLE_TARGET} CommonUI_wxWidgets)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)

target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Network)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_TARGET} ACENetwork)

target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Test_I)

# add PCH
#set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#cotire (${EXECUTABLE_TARGET})
target_precompile_headers (${EXECUTABLE_TARGET}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE_TARGET}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_i)
if (UNIX)
 install (FILES etc/target.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/camstream" COMPONENT config)
elseif (WIN32)
 install (FILES etc/target.glade
          DESTINATION "data/${PROJECT_NAME}/camstream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/camstream" COMPONENT config)
endif ()

#cmake_policy (SET CMP0053 OLD)
set (COMMAND "$<TARGET_FILE:${EXECUTABLE_TARGET}>")
set (ENVIRONMENT "")
if (UNIX)
 set (ARGS "-g./etc/target.glade -l -o -t")
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
 file (TO_NATIVE_PATH ${PROJECT_BINARY_DIR}/../test_i/camstream WORKING_DIRECTORY)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE_TARGET}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GTK_SUPPORT)
  set (ARGS "-g.\\\\etc\\\\target.glade -l -o -t")
 else ()
  set (ARGS "-c -g -l -o -t")
 endif (GTK_SUPPORT)
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
