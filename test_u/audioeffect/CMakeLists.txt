if (GUI_SUPPORT)
 if (GTK_USE AND GTKGL_SUPPORT)
  option (GTKGL_USE "enable GTK OpenGL" ON)
  if(GTKGL_USE)
   add_definitions (-DGTKGL_USE)
  endif (GTKGL_USE)
 endif (GTK_USE AND GTKGL_SUPPORT)
endif (GUI_SUPPORT)

if (UNIX)
# *NOTE*: Common requires libcap
# pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_LIBALSA REQUIRED alsa)
 pkg_check_modules (PKG_SOX REQUIRED sox)
 pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libavutil libavformat libswscale)
endif ()

if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTKGL_SUPPORT)
# *TODO*: FindGLUT.cmake is currently broken
#   find_package (GLUT)
   if (NOT GLUT_FOUND)
    set (GLUT_LIB_FILE libglut.so.3)
    if (UNIX)
     find_library (GLUT_LIBRARY ${GLUT_LIB_FILE}
                   HINTS /usr/lib
                   PATH_SUFFIXES x86_64-linux-gnu
                   DOC "searching for ${GLUT_LIB_FILE} (system paths)")
    elseif (WIN32)
     unset (LIB_FILE_SUFFIX)
     set (GLUT_LIB_FILE freeglut)
#     if (CMAKE_BUILD_TYPE STREQUAL Debug)
#      set (LIB_FILE_SUFFIX d)
#     endif ()
     set (GLUT_LIB_FILE ${GLUT_LIB_FILE}${LIB_FILE_SUFFIX}.lib)
 # message (STATUS "${GLUT_LIB_FILE} search path: \"$ENV{LIB_ROOT}/freeglut/lib\"")
     find_library (GLUT_LIBRARY ${GLUT_LIB_FILE}
#                   PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../FreeGLUT/freeglut/freeglut/cmake/lib/${CMAKE_BUILD_TYPE}
                   PATHS $ENV{LIB_ROOT}/freeglut/lib
                   DOC "searching for ${GLUT_LIB_FILE}")
    endif ()
    if (GLUT_LIBRARY)
     message (STATUS "Found Glut library \"${GLUT_LIBRARY}\"")
    else ()
     message (FATAL_ERROR "could not find ${GLUT_LIB_FILE} (was: \"${GLUT_LIBRARY}\"), aborting")
    endif (GLUT_LIBRARY)
   endif (NOT GLUT_FOUND)
  endif (GTKGL_SUPPORT)
 endif (GTK_USE)
endif (GUI_SUPPORT)

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH DIRECTORY)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)
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
if (GUI_SUPPORT)
 add_definitions (-DCOMMON_UI_HAS_DLL)
 if (GTK_SUPPORT)
  add_definitions (-DCOMMON_UI_GTK_HAS_DLL)
  if (GTKGL_SUPPORT)
   add_definitions (-DCOMMON_GL_HAS_DLL)
  endif (GTKGL_SUPPORT)
 endif (GTK_SUPPORT)
endif (GUI_SUPPORT)

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES ${BUILD_PATH_SUFFIX}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()
add_definitions (-DSTREAM_HAS_DLL)

if (UNIX)
# include_directories (${PKG_CAP_INCLUDE_DIRS})

 include_directories (${PKG_ALSA_INCLUDE_DIRS})

 include_directories (${PKG_FFMPEG_INCLUDE_DIRS})

# include_directories (${PKG_LIBPNG_INCLUDE_DIRS})

 include_directories (/usr/local/src/ACE_wrappers)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
 if (GUI_SUPPORT)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
  if (GTK_USE)
   include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/gtk)
   if (GTKGL_SUPPORT)
    include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl)
   endif (GTKGL_SUPPORT)
  endif (GTK_USE)
 endif (GUI_SUPPORT)
elseif (WIN32)
# glm
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTKGL_SUPPORT)
    include_directories ($ENV{LIB_ROOT}/glm)
   endif (GTKGL_SUPPORT)
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)

 # *NOTE*: workaround
# set (_PF86 "ProgramFiles(x86)")
 set (DIRECTSHOW_BASECLASSES_DIRECTORY
      $ENV{LIB_ROOT}/DShowBaseClasses)
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

# DirectShow
# include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})
 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses)

 # assimp
 #include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../assimp/include)

# libav
 include_directories ($ENV{LIB_ROOT}/ffmpeg/include)

 if (DEFINED ENV{ACE_ROOT})
  include_directories ($ENV{ACE_ROOT})
 else ()
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ATCD/ACE)
 endif (DEFINED ENV{ACE_ROOT})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/3rd_party)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/cmake)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
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
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE audioeffect)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.cpp
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
if (GUI_SUPPORT)
 if (GTK_USE)
  set (${EXECUTABLE}_EXE_SRCS
       ${${EXECUTABLE}_EXE_SRCS}
       test_u_audioeffect_callbacks.cpp
       test_u_audioeffect_callbacks.h
       test_u_audioeffect_gl_callbacks.cpp
       test_u_audioeffect_gl_callbacks.h)
 elseif (WXWIDGETS_USE)
#  set (${EXECUTABLE}_EXE_SRCS
#       ${${EXECUTABLE}_EXE_SRCS}
#       test_u_audioeffect_ui.cpp
#       test_u_audioeffect_ui.h
#       test_u_audioeffect_ui.inl
#       test_u_audioeffect_ui_base.cpp
#       test_u_audioeffect_ui_base.h)
 endif ()
endif (GUI_SUPPORT)

if (UNIX)
 add_definitions (-D_GLIBCXX_INCLUDE_NEXT_C_HEADERS)
endif (UNIX)

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-std=c++11)
#add_compile_options (-fPIC)
 add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

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
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)
if (WIN32)
 add_dependencies (${EXECUTABLE} CommonError)
endif ()
add_dependencies (${EXECUTABLE} CommonLog)
add_dependencies (${EXECUTABLE} CommonMath)
add_dependencies (${EXECUTABLE} CommonTimer)
if (GUI_SUPPORT)
 add_dependencies (${EXECUTABLE} CommonUI)
 if (GTK_USE)
  add_dependencies (${EXECUTABLE} CommonUI_GTK)
 endif (GTK_USE)
endif (GUI_SUPPORT)

add_dependencies (${EXECUTABLE} ACEStream)
add_dependencies (${EXECUTABLE} ACEStream_Decoder)
add_dependencies (${EXECUTABLE} ACEStream_Device)
add_dependencies (${EXECUTABLE} ACEStream_Visualization)

if (UNIX)
 target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
# target_link_libraries (${EXECUTABLE} ${PKG_LIBALSA_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBAV_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_LIBPNG_LIBRARIES})
 target_link_libraries (${EXECUTABLE} ${PKG_SOX_LIBRARIES})
elseif (WIN32)
 if (MSVC)
#  target_link_libraries (${EXECUTABLE} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:-SAFESEH:NO>")
#  set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /SAFESEH:NO) 
#  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"libcmt.lib\"")
#  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"libcpmt.lib\"")
 endif (MSVC)
 set (DIRECTX_SDK_DIRECTORY
      $ENV{DXSDK_DIR})
# message (STATUS "DirectX SDK directory: \"${DIRECTX_SDK_DIRECTORY}\"")
 #link_directories ("${DIRECTX_SDK_DIRECTORY}/Lib/x86")
 target_link_libraries (${EXECUTABLE} "$ENV{LIB_ROOT}/DXSDK/Lib/x86/dxguid.lib")
 target_link_libraries (${EXECUTABLE} mfplat.lib)
 target_link_libraries (${EXECUTABLE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE} Dmoguids.lib)
 target_link_libraries (${EXECUTABLE} Msdmo.lib)
 target_link_libraries (${EXECUTABLE} strmiids.lib)
# OpenGL
 target_link_libraries (${EXECUTABLE} OpenGL32.lib)
 target_link_libraries (${EXECUTABLE} GlU32.lib)

# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavformat/avformat.lib)
# target_link_libraries (${EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg/libavformat/libavformat.a)

 target_link_libraries (${EXECUTABLE} $ENV{LIB_ROOT}/assimp/cmake/code/${CMAKE_BUILD_TYPE}/assimp-vc140-mt.lib)
endif ()
target_link_libraries (${EXECUTABLE} ${GLUT_LIBRARY})

target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonImage)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonMath)
target_link_libraries (${EXECUTABLE} CommonSignal)
target_link_libraries (${EXECUTABLE} CommonTimer)
if (GUI_SUPPORT)
 if (GTK_USE)
  if (GTKGL_SUPPORT)
   target_link_libraries (${EXECUTABLE} CommonOpenGL)
  endif (GTKGL_SUPPORT)
 endif (GTK_USE)
endif (GUI_SUPPORT)

if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif (WIN32)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
#target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_MediaFramework)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Statistic)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

#if (UNIX)
# target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
# target_link_libraries (${EXECUTABLE} ${PKG_LIBPNG_LIBRARIES})
#endif (UNIX)

# add PCH
set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${EXECUTABLE})

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR} COMPONENT test_u)
if (UNIX)
 install (FILES etc/${EXECUTABLE}.gtk2 etc/${EXECUTABLE}.gtk3
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
elseif (WIN32)
 install (FILES etc/${EXECUTABLE}.gtk2 etc/${EXECUTABLE}.gtk3
          DESTINATION "data/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/${EXECUTABLE}" COMPONENT config)
endif ()

#cmake_policy (SET CMP0053 OLD)
set (ENVIRONMENT "")
if (UNIX)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTK3_SUPPORT)
    set (ARGS "-g./etc/audioeffect.gtk3 -l -t")
   elseif (GTK2_SUPPORT)
    set (ARGS "-g./etc/audioeffect.gtk2 -l -t")
   endif ()
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)
 set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib)
 set (ENVIRONMENT "GDK_BACKEND=x11")
elseif (WIN32)
 if (GUI_SUPPORT)
  if (GTK_SUPPORT)
   if (GTK3_USE)
    set (ARGS "-g.\\\\etc\\\\audioeffect.gtk3 -l -t")
    set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                              G:/software/Development/dll)
    file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk3/bin GTK_PATH)
   elseif (GTK2_USE)
    set (ARGS "-g.\\\\etc\\\\audioeffect.gtk2 -l -t")
    set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                              G:/software/Development/dll)
    file (TO_CMAKE_PATH $ENV{LIB_ROOT}/gtk2/bin GTK_PATH)
   endif ()
  endif (GTK_SUPPORT)
 endif (GUI_SUPPORT)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/assimp/cmake/code/${CMAKE_BUILD_TYPE} ASSIMP_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ffmpeg/bin FFMPEG_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/libpng/cmake/${CMAKE_BUILD_TYPE} LIBPNG_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/zlib/cmake/${CMAKE_BUILD_TYPE} ZLIB_PATH)
 file (TO_CMAKE_PATH $ENV{LIB_ROOT}/ImageMagick IMAGEMAGICK_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${ASSIMP_PATH}
                           ${FFMPEG_PATH}
                           ${GTK_PATH}
                           ${LIBPNG_PATH}
                           ${ZLIB_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${PROJECT_SOURCE_DIR}/../ACE_TAO/ACE/lib)
 message (STATUS "RUNTIME_LIBRARY_DIRS: ${RUNTIME_LIBRARY_DIRS}")
endif ()
create_target_launcher (${EXECUTABLE}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/audioeffect
                        ENVIRONMENT ${ENVIRONMENT})

add_test (NAME ${EXECUTABLE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties (${EXECUTABLE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\Common\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\Common\cmake\src\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\Common\cmake\src\ui\Debug;$(ProjectDir)..\..\src\Debug;$(ProjectDir)..\..\src\modules\dev\Debug;$(ProjectDir)..\..\src\modules\vis\Debug
