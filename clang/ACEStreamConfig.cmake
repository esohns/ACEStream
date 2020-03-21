# ACEStreamConfig.cmake.in
#  LIBACESTEAM_INCLUDE_DIRS - include directories for FooBar
#  LIBACESTEAM_LIBRARIES    - libraries to link against

# Compute paths
get_filename_component (LIBACESTREAM_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set (LIBACESTREAM_INCLUDE_DIRS "/mnt/win_d/projects/ACEStream;/mnt/win_d/projects/ACEStream/clang")

# library dependencies (contains definitions for IMPORTED targets)
if (NOT TARGET libACEStream AND NOT LIBACESTREAM_BINARY_DIR)
 cmake_policy (SET CMP0024 OLD)
 include ("${LIBACESTREAM_CMAKE_DIR}/ACEStreamTargets.cmake")
endif ()

# These are IMPORTED targets created by libACEStreamTargets.cmake
set (LIBACESTREAM_LIBRARIES ACEStream)