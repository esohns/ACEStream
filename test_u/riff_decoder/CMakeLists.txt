#include (FindPkgConfig)

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 set (CMAKE_THREAD_PREFER_PTHREAD ON)
 find_package (Threads)
endif (UNIX)

##pkg_check_modules (PKG_ACE REQUIRED ACE)
if (UNIX)
## *NOTE*: Common requires libcap
# pkg_check_modules (PKG_CAP REQUIRED libcap)
# pkg_check_modules (PKG_FFMPEG REQUIRED libavcodec libswscale libavutil)
endif ()
# *TODO*: repair WIN32 pkgconfig support

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

get_filename_component (BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR}/../.. REALPATH)
get_filename_component (BUILD_PATH_SUFFIX ${BUILD_PATH} NAME)

if (UNIX)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/timer)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/include)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/log)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/parser)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/timer)
endif ()
include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/stat)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)

############## next target ################
set (EXECUTABLE RIFF_decoder)
set (${EXECUTABLE}_EXE_SRCS
     stdafx.cpp
     stdafx.h
     test_u_riffdecoder.cpp
     test_u_riffdecoder_common.h
     test_u_riffdecoder_common_modules.h
     test_u_riffdecoder_defines.h
     test_u_riffdecoder_message.cpp
     test_u_riffdecoder_message.h
     test_u_riffdecoder_session_message.cpp
     test_u_riffdecoder_session_message.h
     test_u_riffdecoder_stream.cpp
     test_u_riffdecoder_stream.h)

#cmake_policy (SET CMP0060 OLD) # link libraries by full path even in implicit directories
#cmake_policy (SET CMP0065 OLD) # do not add flags to export symbols from executables without the ENABLE_EXPORTS target property

add_executable (${EXECUTABLE}
                EXCLUDE_FROM_ALL
                ${${EXECUTABLE}_EXE_SRCS})
set_property (TARGET ${EXECUTABLE} PROPERTY FOLDER "test_u")
cmake_policy (SET CMP0012 NEW)
if (WIN32 AND NOT BUILD_TEST_U)
 set_target_properties (${EXECUTABLE} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
add_dependencies (${EXECUTABLE} Common)

add_dependencies (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
#if (UNIX)
# target_link_libraries (${EXECUTABLE} rt)
# target_link_libraries (${EXECUTABLE} ${AIO_LIBRARY})
# target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
# target_link_libraries (${EXECUTABLE} ${PKG_FFMPEG_LIBRARIES})
#endif (UNIX)
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonTimer)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME})
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Decoder)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_File)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Miscellaneous)
target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Statistic)

target_link_libraries (${EXECUTABLE} ${PROJECT_NAME}_Test_U)

#if (UNIX)
# target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
#endif (UNIX)

# add PCH
#set_target_properties (${EXECUTABLE} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire (${EXECUTABLE})
target_precompile_headers (${EXECUTABLE}
                           PUBLIC stdafx.h)

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION ${INSTALL_BIN_DIR}
         COMPONENT test_u)

set (ARGS "-d -f servers.ini -l -t")
if (UNIX)
# set (RUNTIME_LIBRARY_DIRS ${PROJECT_SOURCE_DIR}/modules/ATCD/ACE/lib)
elseif (WIN32)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIB_PATH)
 set (RUNTIME_LIBRARY_DIRS ${ACE_LIB_PATH})
endif ()
create_target_launcher (${EXECUTABLE}
                        ARGS ${ARGS}
#                        FORWARD_ARGS
                        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBRARY_DIRS}
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/../test_u/riff_decoder
                        ENVIRONMENT ${ENVIRONMENT})
