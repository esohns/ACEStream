# Install script for directory: /mnt/win_d/projects/ACEStream/src/modules/net

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xmod_includex" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/ACEStream/stream_net_common.h;/usr/local/include/ACEStream/stream_net_defines.h;/usr/local/include/ACEStream/stream_net_io.h;/usr/local/include/ACEStream/stream_net_io.inl;/usr/local/include/ACEStream/stream_net_io_stream.h;/usr/local/include/ACEStream/stream_net_io_stream.inl;/usr/local/include/ACEStream/stream_net_source.h;/usr/local/include/ACEStream/stream_net_source.inl;/usr/local/include/ACEStream/stream_net_target.h;/usr/local/include/ACEStream/stream_net_target.inl")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/ACEStream" TYPE FILE FILES
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_common.h"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_defines.h"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_io.h"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_io.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_io_stream.h"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_io_stream.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_source.h"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_source.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_target.h"
    "/mnt/win_d/projects/ACEStream/src/modules/net/stream_net_target.inl"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xmod_libx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libACEStream_Network.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/mnt/win_d/projects/ACEStream/clang/src/modules/net/libACEStream_Network.a")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/mnt/win_d/projects/ACEStream/clang/src/modules/net/protocols/cmake_install.cmake")

endif()

