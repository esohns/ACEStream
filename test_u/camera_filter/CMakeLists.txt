set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/scripts")
include (libnoise)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl/scripts")
include (assimp)
include (glew)
include (glm)
include (glut)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/image/scripts;${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image/scripts")
include (png)
include (ffmpeg)
include (ImageMagick)

if (WIN32)
 set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
 include (DirectXBaseClasses)
endif (WIN32)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev/scripts")
include (libcamera)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis/scripts")
include (wayland)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../scripts")
include (pge)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (voronoi)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)

 include (FindPkgConfig)
 pkg_check_modules (PKG_V4L REQUIRED libv4l2)
endif (UNIX)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (LIBNOISE_SUPPORT)
 include_directories (${LIBNOISE_INCLUDE_DIRS})
endif (LIBNOISE_SUPPORT)

if (GLEW_SUPPORT)
 include_directories (${GLEW_INCLUDE_DIRS})
endif (GLEW_SUPPORT)
if (GLM_SUPPORT)
 include_directories (${GLM_INCLUDE_DIRS})
endif (GLM_SUPPORT)
if (GLUT_SUPPORT)
 include_directories (${GLUT_INCLUDE_DIRS})
endif (GLUT_SUPPORT)
if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)

if (LIBCAMERA_SUPPORT)
 include_directories (${LIBCAMERA_INCLUDE_DIRS})
endif (LIBCAMERA_SUPPORT)

if (OLC_PGE_SUPPORT)
 include_directories (${PGE_INCLUDE_DIR})
endif (OLC_PGE_SUPPORT)

if (JC_VORONOI_SUPPORT)
 include_directories (${VORONOI_INCLUDE_DIR})
endif (JC_VORONOI_SUPPORT)

if (UNIX)
 include_directories (${PKG_V4L_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
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
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE camera_filter)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.h
     ../test_u_common.h
     ../test_u_defines.h
     test_u.cpp
     test_u_camera_filter_common.h
     test_u_camera_filter_defines.h
     test_u_common_modules.h
     test_u_eventhandler.cpp
     test_u_eventhandler.h
     test_u_eventhandler.inl
     test_u_message.cpp
     test_u_message.h
     test_u_message.inl
     test_u_module_sobel_filter.cpp
     test_u_module_sobel_filter.h
     test_u_session_message.cpp
     test_u_session_message.h
     test_u_session_message.inl
     test_u_signalhandler.cpp
     test_u_signalhandler.h
     test_u_stream.cpp
     test_u_stream.h)

if (OLC_PGE_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_u_module_marchingsquares_filter.cpp
      test_u_module_marchingsquares_filter.h)
endif (OLC_PGE_SUPPORT)

if (LIBNOISE_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_u_module_perlin_noise_filter.cpp
      test_u_module_perlin_noise_filter.h)
endif (LIBNOISE_SUPPORT)

if (GLUT_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_u_module_opengl_glut.cpp
      test_u_module_opengl_glut.h
      test_u_module_opengl_glut_2.cpp
      test_u_module_opengl_glut_2.h
      test_u_module_opengl_glut_3.cpp
      test_u_module_opengl_glut_3.h
      test_u_module_opengl_glut_4.cpp
      test_u_module_opengl_glut_4.h
      test_u_module_opengl_glut_5.cpp
      test_u_module_opengl_glut_5.h
      test_u_module_opengl_glut_6.cpp
      test_u_module_opengl_glut_6.h
      test_u_module_opengl_glut_7.cpp
      test_u_module_opengl_glut_7.h
      test_u_module_opengl_glut_8.cpp
      test_u_module_opengl_glut_8.h
      test_u_module_opengl_glut_9.cpp
      test_u_module_opengl_glut_9.h
      test_u_module_opengl_glut_10.cpp
      test_u_module_opengl_glut_10.h
      test_u_module_opengl_glut_11.cpp
      test_u_module_opengl_glut_11.h
      test_u_module_opengl_glut_12.cpp
      test_u_module_opengl_glut_12.h
      test_u_module_opengl_glut_13.cpp
      test_u_module_opengl_glut_13.h
      test_u_module_opengl_glut_14.cpp
      test_u_module_opengl_glut_14.h
      test_u_module_opengl_glut_15.cpp
      test_u_module_opengl_glut_15.h
      test_u_module_opengl_glut_16.cpp
      test_u_module_opengl_glut_16.h
      test_u_module_opengl_glut_17.cpp
      test_u_module_opengl_glut_17.h
      test_u_module_opengl_glut_18.cpp
      test_u_module_opengl_glut_18.h
      test_u_module_opengl_glut_19.cpp
      test_u_module_opengl_glut_19.h
      test_u_module_opengl_glut_20.cpp
      test_u_module_opengl_glut_20.h
      test_u_module_opengl_glut_21.cpp
      test_u_module_opengl_glut_21.h
      test_u_module_opengl_glut_22.cpp
      test_u_module_opengl_glut_22.h
      test_u_module_opengl_glut_23.cpp
      test_u_module_opengl_glut_23.h
      test_u_module_opengl_glut_24.cpp
      test_u_module_opengl_glut_24.h
      test_u_module_opengl_glut_25.cpp
      test_u_module_opengl_glut_25.h
      test_u_module_opengl_glut_26.cpp
      test_u_module_opengl_glut_26.h
      test_u_module_opengl_glut_27.cpp
      test_u_module_opengl_glut_27.h
      test_u_module_opengl_glut_28.cpp
      test_u_module_opengl_glut_28.h
      test_u_module_opengl_glut_29.cpp
      test_u_module_opengl_glut_29.h
      test_u_module_opengl_glut_30.cpp
      test_u_module_opengl_glut_30.h
      test_u_module_opengl_glut_31.cpp
      test_u_module_opengl_glut_31.h
      test_u_module_opengl_glut_32.cpp
      test_u_module_opengl_glut_32.h
      test_u_module_opengl_glut_33.cpp
      test_u_module_opengl_glut_33.h
      test_u_module_opengl_glut_34.cpp
      test_u_module_opengl_glut_34.h
      test_u_module_opengl_glut_35.cpp
      test_u_module_opengl_glut_35.h
      test_u_module_opengl_glut_36.cpp
      test_u_module_opengl_glut_36.h
      test_u_module_opengl_glut_37.cpp
      test_u_module_opengl_glut_37.h
      test_u_module_opengl_glut_38.cpp
      test_u_module_opengl_glut_38.h
      test_u_module_opengl_glut_39.cpp
      test_u_module_opengl_glut_39.h
      test_u_module_opengl_glut_40.cpp
      test_u_module_opengl_glut_40.h
      test_u_module_opengl_glut_41.cpp
      test_u_module_opengl_glut_41.h
      test_u_module_opengl_glut_42.cpp
      test_u_module_opengl_glut_42.h
      test_u_module_opengl_glut_43.cpp
      test_u_module_opengl_glut_43.h
      test_u_module_opengl_glut_44.cpp
      test_u_module_opengl_glut_44.h
      test_u_module_opengl_glut_45.cpp
      test_u_module_opengl_glut_45.h
      test_u_module_opengl_glut_46.cpp
      test_u_module_opengl_glut_46.h)
endif (GLUT_SUPPORT)

if (JC_VORONOI_SUPPORT AND OLC_PGE_SUPPORT)
 set (${EXECUTABLE}_EXE_SRCS
      ${${EXECUTABLE}_EXE_SRCS}
      test_u_module_weightedvoronoistipple_filter.cpp
      test_u_module_weightedvoronoistipple_filter.h)
endif (JC_VORONOI_SUPPORT AND OLC_PGE_SUPPORT)

if (WIN32)
 # *NOTE*: DirectShow uses 'DEBUG'
# add_definitions (-DDEBUG) # activate DirectShow debug logging
 add_compile_options ($<$<CONFIG:Debug>:-DDEBUG>)
 add_compile_options ($<$<CONFIG:Debug>:-DD3D_DEBUG_INFO>)
endif (WIN32)

cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property
add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS})
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
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_MediaFramework)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Statistic)
add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

if (LIBNOISE_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBNOISE_LIBRARIES})
endif (LIBNOISE_SUPPORT)

if (GLEW_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${GLEW_LIBRARIES})
endif (GLEW_SUPPORT)
if (GLUT_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${GLUT_LIBRARIES})
endif (GLUT_SUPPORT)
if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)

if (LIBCAMERA_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBCAMERA_LIBRARIES})
endif (LIBCAMERA_SUPPORT)

if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
 target_link_libraries (${EXECUTABLE} PUBLIC ${PKG_V4L_LIBRARIES})
elseif (WIN32)
# target_link_libraries (${EXECUTABLE} PUBLIC dxguid.lib)
 target_link_libraries (${EXECUTABLE} PUBLIC mfplat.lib)
endif ()
target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonImage)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)
if (GUI_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonUI)
 if (GTK_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_GTK)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  target_link_libraries (${EXECUTABLE} PUBLIC CommonUI_wxWidgets)
 endif (WXWIDGETS_SUPPORT)
endif (GUI_SUPPORT)
if (UNIX)
 target_link_libraries (${EXECUTABLE} PUBLIC CommonXML)
endif (UNIX)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
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
install (FILES
         "shader.frag"
         "shader.vert"
         "shader_2.frag"
         "shader_2.vert"
         "shader_3.frag"
         "shader_3.vert"
         "shader_4.frag"
         "shader_4.vert"
         "shader_5.frag"
         "shader_5.vert"
         "shader_6.frag"
         "shader_6.frag_2"
         "shader_6.frag_3"
         "shader_6.frag_4"
         "shader_6.frag_5"
         "shader_6.vert"
         "shader_6_common.glsl"
         "shader_7.frag"
         "shader_7.vert"
         "shader_7_channel1.png"
         "shader_7_channel2.png"
         "shader_8.frag"
         "shader_8.vert"
         "shader_9.frag"
         "shader_9.vert"
         "shader_10.frag"
         "shader_10.vert"
         "shader_11.frag"
         "shader_11.vert"
         "shader_12.frag"
         "shader_12.vert"
         "shader_13.frag"
         "shader_13.vert"
         "shader_14.frag"
         "shader_14.frag_2"
         "shader_14.vert"
         "shader_15.frag"
         "shader_15.vert"
         "shader_16.frag"
         "shader_16.vert"
         "shader_16_channel1.png"
         "shader_17.frag"
         "shader_17.vert"
         "shader_18.frag"
         "shader_18.frag_2"
         "shader_18.frag_3"
         "shader_18.vert"
         "shader_18_channel1.png"
         "shader_19.frag"
         "shader_19.vert"
         "shader_20.frag"
         "shader_20.vert"
         "shader_21.frag"
         "shader_21.vert"
         "shader_22.frag"
         "shader_22.vert"
         "shader_23.frag"
         "shader_23.vert"
         "shader_24.frag"
         "shader_24.vert"
         "shader_25.frag"
         "shader_25.vert"
         "shader_26.frag"
         "shader_26.frag_2"
         "shader_26.vert"
         "shader_27.frag"
         "shader_27.vert"
         "shader_28.frag"
         "shader_28.vert"
         "shader_29.frag"
         "shader_29.vert"
         "shader_30.frag"
         "shader_30.vert"
         "shader_31.frag"
         "shader_31.vert"
         "shader_32.frag"
         "shader_32.frag_2"
         "shader_32.frag_3"
         "shader_32.frag_4"
         "shader_32.frag_5"
         "shader_32.vert"
         "shader_33.frag"
         "shader_33.vert"
         "shader_34.frag"
         "shader_34.vert"
         "shader_35.frag"
         "shader_35.vert"
         "shader_36.frag"
         "shader_36.frag_2"
         "shader_36.vert"
         "shader_37.frag"
         "shader_37.frag_2"
         "shader_37.frag_3"
         "shader_37.frag_4"
         "shader_37.vert"
         "shader_38.frag"
         "shader_38.vert"
         "shader_39.frag"
         "shader_39.frag_2"
         "shader_39.frag_3"
         "shader_39.vert"
         "shader_40.frag"
         "shader_40.vert"
         "shader_41.frag"
         "shader_41.vert"
         "shader_42.frag"
         "shader_42.frag_2"
         "shader_42.vert"
         "shader_43.frag"
         "shader_43.frag_2"
         "shader_43.frag_3"
         "shader_43.vert"
         "shader_44.frag"
         "shader_44.frag_2"
         "shader_44.frag_3"
         "shader_44.frag_4"
         "shader_44.frag_5"
         "shader_44.vert"
         "shader_45.frag"
         "shader_45.vert"
         "shader_46.frag"
         "shader_46.vert"
         DESTINATION ${INSTALL_BIN_DIR}
         COMPONENT config)
if (UNIX)
# install (FILES etc/${ui_definition_file}
#          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 get_filename_component (ACE_LIB_PATH ${ACE_LIBRARY} DIRECTORY)
 set (ACE_DLL_FILE ACE${LIB_FILE_SUFFIX}.dll)
 set (ACE_DLL_FQPATH ${ACE_LIB_PATH}/${ACE_DLL_FILE})
 if (DEFINED $ENV{LIB_ROOT})
  file (TO_NATIVE_PATH $ENV{LIB_ROOT} LIB_ROOT)
  string (REPLACE "\\" "\\\\" LIB_ROOT ${LIB_ROOT})
  install (FILES ${ACE_DLL_FQPATH}
                 ${LIB_ROOT}/freeglut/bin/x64/freeglut.dll
           DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
 endif (DEFINED $ENV{LIB_ROOT})
# install (FILES etc/${EXECUTABLE}.gtk3 ${EXECUTABLE}.gtk2 ${EXECUTABLE}.fbp
#          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

##########################################

set (ARGS "-l")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}" COMMAND)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib)
# set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${LIBNOISE_LIB_DIR}" LIBNOISE_PATH)
 file (TO_CMAKE_PATH "${OPENGL_LIB_DIR}" OPENGL_PATH)
 file (TO_CMAKE_PATH "${GLEW_LIB_DIR}" GLEW_PATH)
 file (TO_CMAKE_PATH "${GLUT_LIB_DIR}" GLUT_PATH)
 file (TO_CMAKE_PATH "${ASSIMP_LIB_DIR}" ASSIMP_PATH)
 file (TO_CMAKE_PATH "${PNG_LIB_DIR}" LIBPNG_PATH)
 # *NOTE*: ffmpeg was built with "vcpkg-libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  file (TO_CMAKE_PATH "${wxWidgets_LIB_DIR}" WXWIDGETS_PATH)
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIBRARY_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${LIBNOISE_PATH}
                           ${OPENGL_PATH}
                           ${GLEW_PATH}
                           ${GLUT_PATH}
                           ${VCPKG_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${ASSIMP_PATH}
                           ${LIBPNG_PATH}
                           ${ACE_LIBRARY_PATH})
 if (GTK_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${GTK_PATH})
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                            ${WXWIDGETS_PATH})
 endif (WXWIDGETS_SUPPORT)
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
