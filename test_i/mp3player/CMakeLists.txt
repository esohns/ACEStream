include (ffmpeg)
include (ImageMagick)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec/scripts")
include (faad)
include (mpg123)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev/scripts")
include (pipewire)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib/scripts")
include (sox)

string (REPLACE ${CMAKE_SOURCE_DIR} "" BUILD_PATH_SUFFIX ${CMAKE_BINARY_DIR})
string (SUBSTRING ${BUILD_PATH_SUFFIX} 1 -1 BUILD_PATH_SUFFIX)

if (FFMPEG_SUPPORT)
 include_directories (${ffmpeg_INCLUDE_DIRS})
endif (FFMPEG_SUPPORT)
if (FAAD_SUPPORT)
 include_directories (${faad_INCLUDE_DIRS})
endif (FAAD_SUPPORT)
if (MPG123_SUPPORT)
 include_directories (${MPG123_INCLUDE_DIRS})
endif (MPG123_SUPPORT)
if (LIBPIPEWIRE_SUPPORT)
 include_directories (${LIBPIPEWIRE_INCLUDE_DIRS})
endif (LIBPIPEWIRE_SUPPORT)
if (SOX_SUPPORT)
 include_directories (${SOX_INCLUDE_DIRS})
endif (SOX_SUPPORT)
if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/http)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/include)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/cmake)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)

# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/protocol/http)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/include)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../3rd_party/bison)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/vis)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE mp3player)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.h
     test_i_modules.h
     test_i_message.cpp
     test_i_message.h
     test_i_session_message.cpp
     test_i_session_message.h
     test_i_mp3player_common.h
     test_i_mp3player_defines.h
     test_i_mp3player_signalhandler.cpp
     test_i_mp3player_signalhandler.h
     test_i.cpp
     test_i_stream.cpp
     test_i_stream.h
     test_i_stream_common.h)

add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_i")
if (WIN32)
 if (NOT BUILD_TEST_I)
  set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
  set_target_properties (${EXECUTABLE} PROPERTIES COMPILE_FLAGS "/wd4503")
 endif ()
endif ()

target_link_libraries (${EXECUTABLE} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if (FFMPEG_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${ffmpeg_LIBRARIES})
endif (FFMPEG_SUPPORT)
if (FAAD_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${faad_LIBRARIES})
endif (FAAD_SUPPORT)
if (MPG123_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${MPG123_LIBRARIES})
endif (MPG123_SUPPORT)
if (LIBPIPEWIRE_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${LIBPIPEWIRE_LIBRARIES})
endif (LIBPIPEWIRE_SUPPORT)
if (SOX_SUPPORT)
 target_link_libraries (${EXECUTABLE} PUBLIC ${SOX_LIBRARIES})
endif (SOX_SUPPORT)

target_link_libraries (${EXECUTABLE} PUBLIC ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} PUBLIC Common)
target_link_libraries (${EXECUTABLE} PUBLIC CommonLog)
target_link_libraries (${EXECUTABLE} PUBLIC CommonSignal)
target_link_libraries (${EXECUTABLE} PUBLIC CommonTimer)

target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_Device)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} PUBLIC ${PROJECT_NAME}_MediaFramework)
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
 create_target_launcher (${EXECUTABLE}
                         ARGS "-f /mnt/win_d/test.mp3 -l -t"
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ACE/lib
                                              ${PROJECT_SOURCE_DIR}/../Common/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                                              ${PROJECT_SOURCE_DIR}/${BUILD_PATH_SUFFIX}/src/${CMAKE_BUILD_TYPE}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT foo=bar)
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 if (GTK_SUPPORT)
  file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_PATH)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS} ${GTK_PATH})
 endif (GTK_SUPPORT)
 if (WXWIDGETS_SUPPORT)
  set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS} ${wxWidgets_LIB_DIR})
 endif (WXWIDGETS_SUPPORT)
 file (TO_CMAKE_PATH "${faad_LIB_DIR}" FAAD_PATH)
 file (TO_CMAKE_PATH "${MPG123_LIB_DIR}" MPG123_PATH)
# *NOTE*: ffmpeg was built with (vcpkg-)"libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_PATH)
 file (TO_CMAKE_PATH "${SOX_LIB_DIR}" SOX_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_PATH)
 set (RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                           ${FAAD_PATH}
                           ${MPG123_PATH}
                           ${VCPKG_PATH}
                           ${FFMPEG_PATH}
                           ${IMAGEMAGICK_PATH}
                           ${SOX_PATH}
                           ${ACE_PATH})
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS "-f F:\\\\test.mp3 -l -t"
#                        FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test_i/${EXECUTABLE}
                         ENVIRONMENT foo=bar)
endif ()
