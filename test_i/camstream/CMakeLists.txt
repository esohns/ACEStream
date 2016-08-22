if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 find_package (Threads REQUIRED)
endif ()

set (ACE_LIB_FILE libACE.so)
if (UNIX)
 if (DEFINED ENV{ACE_ROOT})
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                PATHS ENV ACE_ROOT
                PATH_SUFFIXES ace build/linux/ace
                DOC "searching for ${ACE_LIB_FILE}")
 else ()
  find_library (ACE_LIBRARY ${ACE_LIB_FILE}
                DOC "searching for ${ACE_LIB_FILE} (system paths)")
 endif ()
elseif (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set (LIB_FILE_SUFFIX d)
 endif ()
 unset (ACE_LIB_FILE)
 set (ACE_LIB_FILE ACE${LIB_FILE_SUFFIX}.lib)
 find_library (ACE_LIBRARY ${ACE_LIB_FILE}
               PATHS ENV ACE_ROOT
               PATH_SUFFIXES lib lib\\${CMAKE_BUILD_TYPE}\\Win32
               DOC "searching for ${ACE_LIB_FILE}")
endif ()
#if (NOT EXISTS ACE_LIBRARY)
if (NOT ACE_LIBRARY)
 message (FATAL_ERROR "could not find ${ACE_LIB_FILE} (was: \"${ACE_LIBRARY}\"), aborting")
else ()
 message (STATUS "found ACE library \"${ACE_LIBRARY}\"")
endif ()

if (WIN32)
 set (DIRECTSHOW_BASECLASSES_DIRECTORY "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
endif ()

include (FindPkgConfig)
if (UNIX)
 #pkg_check_modules (PKG_ACE REQUIRED ACE)
# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)
 pkg_check_modules (PKG_V4L2 REQUIRED libv4l2)
 pkg_check_modules (PKG_AVCODEC REQUIRED libavutil libavcodec)
 pkg_check_modules (PKG_GTK REQUIRED libpng gmodule-2.0 gthread-2.0 gtk+-2.0)
endif ()

find_package (Common ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../modules ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES libCommon/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..
              PATH_SUFFIXES cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

find_package (ACENetwork ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES libACENetwork/cmake
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACE)
 include_directories (${PKG_CAP_INCLUDE_DIRS})
 include_directories (${PKG_V4L2_INCLUDE_DIRS})
 include_directories (${PKG_AVCODEC_INCLUDE_DIRS})
 include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/modules/libACENetwork/src/client_server)
else ()
 include_directories ($ENV{ACE_ROOT})
# *TODO*: repair pkgconfig support
# include_directories (${PKG_GTK_GLADE_INCLUDE_DIRS})
# gtk
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/glib-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/glib-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/gtk-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gtk-2.0/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/gdk-pixbuf-2.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/atk-1.0)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/include/pango-1.0)

# libav
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libav-11.7)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/msvc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libCommon/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../libACENetwork/src/client_server)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/misc)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/net)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

############## next target ################
set (camsource_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_camstream_common.h
     test_i_common_modules.h
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_defines.h
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

set (EXECUTABLE_SOURCE camsource)
add_executable (${EXECUTABLE_SOURCE} ${camsource_EXE_SRCS} etc/resources.rc etc/source.glade)

#add_dependencies (${EXECUTABLE_SOURCE} ACE)
add_dependencies (${EXECUTABLE_SOURCE} Common)
add_dependencies (${EXECUTABLE_SOURCE} CommonUI)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Device)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE_SOURCE} ACENetwork)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Network)
add_dependencies (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_SOURCE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_SOURCE} rt)
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_V4L2_LIBRARIES})
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_AVCODEC_LIBRARIES})
 target_link_libraries (${EXECUTABLE_SOURCE} ${PKG_GTK_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_SOURCE} dxguid.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} evr.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} mfplat.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} mfuuid.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} strmiids.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${EXECUTABLE_SOURCE} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${EXECUTABLE_SOURCE} strmbase.lib)
 endif ()
 target_link_libraries (${EXECUTABLE_SOURCE} winmm.lib)
# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/pango-1.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gdk_pixbuf-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gdk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gtk-win32-2.0.lib)

 target_link_libraries (${EXECUTABLE_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libav-11.7/libavcodec/libavcodec.dll.a)
endif ()
target_link_libraries (${EXECUTABLE_SOURCE} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE_SOURCE} CommonUI)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_SOURCE} ${PROJECT_NAME}_Visualization)
target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork)
#target_link_libraries (${EXECUTABLE_SOURCE} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_SOURCE} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)

set (RESOURCE_FILES
     etc/resources.rc
     etc/source.glade
     etc/target.glade)
set_source_files_properties (${RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group (resources FILES ${RESOURCE_FILES})

install (TARGETS ${EXECUTABLE_SOURCE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT test_i)
if (WIN32)
 install (FILES etc/source.glade
          DESTINATION "data/${PROJECT_NAME}/camstream" COMPONENT config)
#          DESTINATION "$ENV{APPDATA}/${PROJECT_NAME}/camstream" COMPONENT config)
elseif (UNIX)
 install (FILES etc/source.glade
          DESTINATION "${INSTALL_SHARE_DIR}/${PROJECT_NAME}/camstream" COMPONENT config)
endif ()

#guess_runtime_library_dirs (<outputvarname> [<extralibrary> ...])
if (UNIX)
 create_target_launcher (${EXECUTABLE_SOURCE}
#                         ARGS "-g./etc/source.glade -l -t"
                         ARGS "-g -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE_SOURCE}
#                         ARGS "-c -g.\\\\etc\\\\source.glade -l -t"
                         ARGS "-g -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk3/bin
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavcodec
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavresample
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavutil
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE_SOURCE}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE_SOURCE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_SOURCE}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\libACENetwork\cmake\src\Debug

############## next target ################

if (WIN32)
 include_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY})

 set (camtarget_source_LIB_SRCS
      stdafx.cpp
      stdafx.h
      camtarget_source.cpp)

 set (LIBRARY_DS camtarget_source)
 add_library (${LIBRARY_DS} MODULE
              ${camtarget_source_LIB_SRCS} etc/camtarget_source.def)

 add_dependencies (${LIBRARY_DS} Common)
 add_dependencies (${LIBRARY_DS} ${PROJECT_NAME})

 target_link_libraries (${LIBRARY_DS} ${ACE_LIBRARY})
 target_link_libraries (${LIBRARY_DS} strmiids.lib)
 target_link_libraries (${LIBRARY_DS} winmm.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${LIBRARY_DS} strmbasd.lib)
 else ()
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${LIBRARY_DS} strmbase.lib)
 endif ()
 target_link_libraries (${LIBRARY_DS} Common)
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_DS} ${PROJECT_NAME}_Miscellaneous)

 set_target_properties (${LIBRARY_DS} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_DS} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

 # add PCH
 set_target_properties (${LIBRARY_DS} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_DS}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_DS}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)


 create_target_launcher (${LIBRARY_DS}
#                         ARGS "-c -g.\\\\etc\\\\source.glade -l -t"
                         ARGS "-g -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk3/bin
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavcodec
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavresample
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavutil
#                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
#                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_i/camstream
                         ENVIRONMENT foo=bar)

#-----------------------------------------

 set (camtarget_source_mf_LIB_SRCS
      stdafx.cpp
      stdafx.h
      camtarget_source_mf.cpp)

 set (LIBRARY_MF camtarget_source_mf)
 add_library (${LIBRARY_MF} MODULE
              ${camtarget_source_mf_LIB_SRCS} etc/camtarget_source_mf.def)

 add_dependencies (${LIBRARY_MF} Common)
 add_dependencies (${LIBRARY_MF} ${PROJECT_NAME})

 target_link_libraries (${LIBRARY_MF} ${ACE_LIBRARY})
# target_link_libraries (${LIBRARY_MF} strmiids.lib)
# target_link_libraries (${LIBRARY_MF} winmm.lib)
# if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
#  target_link_libraries (${LIBRARY_MF} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
# else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
#  target_link_libraries (${LIBRARY_MF} strmbase.lib)
# endif ()
 target_link_libraries (${LIBRARY_MF} Common)
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME})
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Device)
 target_link_libraries (${LIBRARY_MF} ${PROJECT_NAME}_Miscellaneous)

 set_target_properties (${LIBRARY_MF} PROPERTIES DEFINE_SYMBOL STREAM_HAS_DLL)
 set_target_properties (${LIBRARY_MF} PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

 # add PCH
 set_target_properties (${LIBRARY_MF} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")

# add all targets to the build-tree export set
 export (TARGETS ${LIBRARY_MF}
         APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

 install (TARGETS ${LIBRARY_MF}
          EXPORT ${PROJECT_NAME}Targets
#          ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
          LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib
          RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT lib)
endif ()

############## next target ################

set (camtarget_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_i_callbacks.cpp
     test_i_callbacks.h
     test_i_camstream_common.h
     test_i_common_modules.h
     test_i_connection_common.h
     test_i_connection_manager_common.h
     test_i_defines.h
#     test_i_module_direct3d.cpp
#     test_i_module_direct3d.h
#     test_i_module_direct3d.inl
     test_i_module_eventhandler.cpp
     test_i_module_eventhandler.h
     test_i_module_eventhandler.inl
     test_i_module_splitter.cpp
     test_i_module_splitter.h
     test_i_source_stream.cpp
     test_i_source_stream.h
     test_i_source_stream.inl
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

set (EXECUTABLE_TARGET camtarget)
add_executable (${EXECUTABLE_TARGET} ${camtarget_EXE_SRCS} etc/resources.rc etc/target.glade)

#add_dependencies (${EXECUTABLE_TARGET} ACE)
add_dependencies (${EXECUTABLE_TARGET} Common)
add_dependencies (${EXECUTABLE_TARGET} CommonUI)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME})
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Miscellaneous)
add_dependencies (${EXECUTABLE_TARGET} ACENetwork)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Network)
add_dependencies (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Visualization)

target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE_TARGET} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE_TARGET} rt)
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_V4L2_LIBRARIES})
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_AVCODEC_LIBRARIES})
 target_link_libraries (${EXECUTABLE_TARGET} ${PKG_GTK_LIBRARIES})
elseif (WIN32)
 target_link_libraries (${EXECUTABLE_TARGET} strmiids.lib)
 target_link_libraries (${EXECUTABLE_TARGET} winmm.lib)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
  target_link_libraries (${EXECUTABLE_TARGET} ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
  target_link_libraries (${EXECUTABLE_TARGET} strmbase.lib)
 endif ()
# *TODO*: repair WIN32 pkgconfig support
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gio-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/glib-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gobject-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/pango-1.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gdk_pixbuf-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gdk-win32-2.0.lib)
 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../gtk3/lib/gtk-win32-2.0.lib)

 target_link_libraries (${EXECUTABLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/../../../libav-11.7/libavcodec/libavcodec.dll.a)
endif ()
target_link_libraries (${EXECUTABLE_TARGET} Common)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
 set_target_properties (${EXECUTABLE_TARGET} PROPERTIES LINK_FLAGS /FORCE:MULTIPLE)
endif ()
target_link_libraries (${EXECUTABLE_TARGET} CommonUI)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE_TARGET} ${PROJECT_NAME}_Visualization)
target_link_libraries (${EXECUTABLE_TARGET} ACENetwork)
#target_link_libraries (${EXECUTABLE_TARGET} ACENetwork_Client)

# add PCH
set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties (${EXECUTABLE_TARGET} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
#cotire (${EXECUTABLE_SOURCE} ${LIBRARY_DS} ${LIBRARY_MF} ${EXECUTABLE_TARGET})
#cotire (${EXECUTABLE_TARGET})

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

if (UNIX)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-g./../../../test_i/camstream/etc/target.glade -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk/bin
                                              ${PROJECT_SOURCE_DIR}/../libxml2-2.9.1/.libs
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/camstream
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 create_target_launcher (${EXECUTABLE_TARGET}
                         ARGS "-g.\\\\..\\\\..\\\\..\\\\test_i\\\\camstream\\\\etc\\\\target.glade -l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/../ATCD/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../gtk3/bin
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavcodec
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavresample
                                              ${PROJECT_SOURCE_DIR}/../libav-11.7/libavutil
#                                              D:/projects/libxml2-2.9.1/.libs
                                              G:/software/Development/dll
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/err/Debug
                                              ${PROJECT_SOURCE_DIR}/../libCommon/cmake/src/ui/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/dev/Debug
                                              ${PROJECT_SOURCE_DIR}/cmake/src/modules/vis/Debug
                                              ${PROJECT_SOURCE_DIR}/../libACENetwork/cmake/src/Debug
                         WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test_i/camstream
                         ENVIRONMENT foo=bar)
endif ()

add_test (NAME ${EXECUTABLE_TARGET}_test
          COMMAND ${PROJECT_BINARY_DIR}/${EXECUTABLE} -l -t
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_tests_properties (${EXECUTABLE_TARGET}_test
                      PROPERTIES
                      ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\..\\libCommon\\cmake\\src\\ui\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\Debug;${PROJECT_SOURCE_DIR}\\cmake\\src\\modules\\dev\\Debug;${PROJECT_SOURCE_DIR}\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%")
#PATH=%PATH%;$(ProjectDir)..\..\..\..\ATCD\ACE\lib;$(ProjectDir)..\..\..\..\gtk3\bin;$(ProjectDir)..\..\..\..\libav-11.7\libavcodec;$(ProjectDir)..\..\..\..\libav-11.7\libavresample;$(ProjectDir)..\..\..\..\libav-11.7\libavutil;$(ProjectDir)..\..\..\..\libCommon\cmake\src\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\err\Debug;$(ProjectDir)..\..\..\..\libCommon\cmake\src\ui\Debug;$(ProjectDir)..\..\..\cmake\src\Debug;$(ProjectDir)..\..\..\cmake\src\modules\dev\Debug;$(ProjectDir)..\..\..\cmake\src\modules\vis\Debug;$(ProjectDir)..\..\..\..\libACENetwork\cmake\src\Debug
