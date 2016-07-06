cmake_minimum_required (VERSION 2.8)

# options
#set (CMAKE_VERBOSE_MAKEFILE ON)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_MICRO 1)
set (VERSION_DEVEL devel)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
if (VERSION_DEVEL)
 unset (VERSION)
 set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}-${VERSION_DEVEL})
endif ()

set (PACKAGE_DESCRIPTION "\
user-space wrapper library for the pipes-and-filters pattern, based on the ACE \
framework (see: http://www.cs.wustl.edu/~schmidt/ACE.html). In particular, the \
library lightly encapsulates the ACE_Stream and ACE_Module classes, introducing\
 a new set of (control) interfaces to support asynchronous operation, and \
additional concepts, such as 'session' data and messages. In conjunction with \
additional, modular data processing functionality (see e.g.: \
https://github.com/esohns/libACENetwork), this approach facilitates the \
separation of data processing from application-specific logic and therefore \
enables portable approaches to (distributed) application design")
set (PACKAGE_DESCRIPTION_SUMMARY "(wrapper) library for streams functionality, based on the ACE framework")

# Package section (see http://packages.debian.org/stable/)
set (PACKAGE_SECTION "devel") # Debian
#set (PACKAGE_SECTION "Development/Libraries")

#variable_watch (CMAKE_BUILD_TYPE)
# *NOTE*: if the user specifies -DCMAKE_BUILD_TYPE on the command line, take
#         their definition and dump it in the cache along with proper
#         documentation, otherwise set CMAKE_BUILD_TYPE to Debug prior to
#         calling PROJECT()
set (BUILD_TYPE_CACHE_STRING "choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
if (DEFINED CMAKE_BUILD_TYPE)
 set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING ${BUILD_TYPE_CACHE_STRING})
elseif (NOT CMAKE_CONFIGURATION_TYPES)
 set (CMAKE_BUILD_TYPE Debug CACHE STRING ${BUILD_TYPE_CACHE_STRING})
endif ()

cmake_policy (SET CMP0048 NEW) # allow project VERSION
project (ACEStream
         VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
         LANGUAGES CXX C)

# override the installation directories ?
#set (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION OFF)
set (INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set (INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set (INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
if (WIN32 AND NOT CYGWIN)
 set (DEF_INSTALL_CMAKE_DIR CMake)
else ()
 set (DEF_INSTALL_CMAKE_DIR lib/CMake/FooBar)
endif ()
set (INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# make relative paths absolute (needed later on)
foreach (path BIN CMAKE SHARE INCLUDE LIB)
 set (var INSTALL_${path}_DIR)
# if (NOT IS_ABSOLUTE "${${var}}")
 string (TOLOWER "${path}" RELATIVE_INSTALL_PATH)

if (UNIX)
 set (${var} "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_PATH}")
elseif (WIN32)
 set (${var} ${RELATIVE_INSTALL_PATH})
endif ()
# message (STATUS "set \"${var}\" to \"${${var}}\"")

#endif ()
endforeach ()

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export (PACKAGE lib${PROJECT_NAME})

# Create the ACEStreamConfig.cmake and ACEStreamConfigVersion.cmake files
#file (RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set (CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file (${PROJECT_NAME}Config.cmake.in
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)
# ... for the install tree
set (CONF_INCLUDE_DIRS "\${LIBACESTREAM_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file (${PROJECT_NAME}Config.cmake.in
                "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake" @ONLY)
# ... for both
configure_file (${PROJECT_NAME}ConfigVersion.cmake.in
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

# Install the ACEStreamConfig.cmake and ACEStreamConfigVersion.cmake
install (FILES
         "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
         "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#         "${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
#         "${PROJECT_NAME}ConfigVersion.cmake"
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
# Install the export set for use with the install-tree
install (EXPORT ${PROJECT_NAME}Targets
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

set (PACKAGE ${PROJECT_NAME})
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_config.h @ONLY)
#add_custom_target (${PROJECT_NAME}_config)
#set_target_properties (${PROJECT_NAME}_config PROPERTIES
#                       PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.h")

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_config.h"
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT dev)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-g)
  add_compile_options (-O0)
  add_compile_options (-fno-inline)
 endif ()
 add_definitions (-D_DEBUG)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS _DEBUG)
else ()
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-O3)
 endif ()
 add_definitions (-DACE_NDEBUG)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACE_NDEBUG)
endif ()

option (TRACING
        "enable tracing" OFF)
if (TRACING)
 add_definitions (-DACESTREAM_NTRACE=0)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACESTREAM_NTRACE=0)
else ()
 add_definitions (-DACESTREAM_NTRACE=1)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACESTREAM_NTRACE=1)
endif ()

option (VALGRIND_SUPPORT
        "enable valgrind support" OFF)
include (CheckIncludeFiles)
if (VALGRIND_SUPPORT)
 check_include_files (valgrind/valgrind.h HAVE_VALGRIND_H)
#  include (CheckLibraryExists)
#  check_library_exists (libvalgrind "" "" HAVE_LIB_VALGRIND)
 include (FindPkgConfig)
 pkg_check_modules (PKGVALGRIND REQUIRED valgrind)
 add_definitions (-DLIBACESTREAM_ENABLE_VALGRIND_SUPPORT)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS LIBACESTREAM_ENABLE_VALGRIND_SUPPORT)
endif ()

option (BUILD_MODULES
        "build (git) sub-modules" OFF)

if (CMAKE_COMPILER_IS_GNUCXX)
 add_compile_options (-ansi)
 add_compile_options (-std=c++11)
 add_compile_options (-Wall)
 set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -g")
 add_definitions (-D_GNU_SOURCE)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)
endif ()
if (MSVC)  
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")  
endif (MSVC)

#if (WIN32)
# set (CMAKE_GENERATOR_TOOLSET "LLVM-vs2014" CACHE STRING "Platform Toolset" FORCE) 
# set (CMAKE_VS_PLATFORM_TOOLSET "LLVM-vs2014" CACHE STRING "Platform Toolset" FORCE)
# if (CMAKE_VS_PLATFORM_TOOLSET STREQUAL LLVM-vs2014)
#  add_compile_options (-Wno-c++11-narrowing)
# endif ()
#endif ()

#add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)
if (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND
     NOT DEFINED IN_CPACK)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
endif ()

################################################################################

if (UNIX)
 include (GNUInstallDirs)
elseif (WIN32)
 include (InstallRequiredSystemLibraries)
endif ()

#set (NSIS_PROJECT_FILE "lib${PROJECT_NAME}.nsi")
#add_custom_command (TARGET ${PROJECT_NAME}
#                    POST_BUILD
#                    COMMAND make_installer.bat ${NSIS_PROJECT_FILE}
#                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/scripts"
#                    COMMENT "making installer..."
#                    VERBATIM)

# build a CPack driven installer package
include (CPack)

################################################################################

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (UseBackportedModules)
cmake_policy (SET CMP0026 OLD)
cmake_policy (SET CMP0053 OLD)
include (CreateLaunchers)

# enable (unit/integration) tests
enable_testing ()
include (CTest)

################################################################################

add_subdirectory (src)
add_subdirectory (test_u)
add_subdirectory (test_i)
