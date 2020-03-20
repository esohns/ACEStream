# Install script for directory: /mnt/win_d/projects/ACEStream/src/modules/misc

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
   "/usr/local/include/ACEStream/stream_misc_aggregator.h;/usr/local/include/ACEStream/stream_misc_aggregator.inl;/usr/local/include/ACEStream/stream_misc_common.h;/usr/local/include/ACEStream/stream_misc_defines.h;/usr/local/include/ACEStream/stream_misc_delay.h;/usr/local/include/ACEStream/stream_misc_delay.inl;/usr/local/include/ACEStream/stream_misc_distributor.h;/usr/local/include/ACEStream/stream_misc_distributor.inl;/usr/local/include/ACEStream/stream_misc_dump.h;/usr/local/include/ACEStream/stream_misc_dump.inl;/usr/local/include/ACEStream/stream_misc_messagehandler.h;/usr/local/include/ACEStream/stream_misc_messagehandler.inl;/usr/local/include/ACEStream/stream_misc_parser.h;/usr/local/include/ACEStream/stream_misc_parser.inl;/usr/local/include/ACEStream/stream_misc_queue_source.h;/usr/local/include/ACEStream/stream_misc_queue_source.inl;/usr/local/include/ACEStream/stream_misc_splitter.h;/usr/local/include/ACEStream/stream_misc_splitter.inl")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/ACEStream" TYPE FILE FILES
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_aggregator.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_aggregator.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_common.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_defines.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_delay.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_delay.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_distributor.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_distributor.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_dump.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_dump.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_messagehandler.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_messagehandler.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_parser.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_parser.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_queue_source.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_queue_source.inl"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_splitter.h"
    "/mnt/win_d/projects/ACEStream/src/modules/misc/stream_misc_splitter.inl"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xmod_libx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libACEStream_Miscellaneous.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/mnt/win_d/projects/ACEStream/clang/src/modules/misc/libACEStream_Miscellaneous.a")
endif()

