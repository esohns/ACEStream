set (FESTIVAL_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_FESTIVAL festival)
 if (PKG_FESTIVAL_FOUND)
  set (FESTIVAL_FOUND TRUE)
  set (FESTIVAL_INCLUDE_DIRS "${PKG_FESTIVAL_INCLUDE_DIRS}")
  set (FESTIVAL_LIBRARIES "${PKG_FESTIVAL_LIBRARIES}")
 else ()
  set (SPEECHTOOLS_BASE_LIB_FILE "libestbase.so")
  find_library (SPEECHTOOLS_BASE_LIBRARY ${SPEECHTOOLS_BASE_LIB_FILE}
                PATHS /usr
                PATH_SUFFIXES lib64
                DOC "searching for ${SPEECHTOOLS_BASE_LIB_FILE}")
  if (NOT SPEECHTOOLS_BASE_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_BASE_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_BASE_LIBRARY)
  set (SPEECHTOOLS_LIB_FILE "libestools.so")
  find_library (SPEECHTOOLS_LIBRARY ${SPEECHTOOLS_LIB_FILE}
                PATHS /usr
                PATH_SUFFIXES lib64
                DOC "searching for ${SPEECHTOOLS_LIB_FILE}")
  if (NOT SPEECHTOOLS_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_LIBRARY)
  set (SPEECHTOOLS_STRING_LIB_FILE "libeststring.so")
  find_library (SPEECHTOOLS_STRING_LIBRARY ${SPEECHTOOLS_STRING_LIB_FILE}
                PATHS /usr
                PATH_SUFFIXES lib64
                DOC "searching for ${SPEECHTOOLS_STRING_LIB_FILE}")
  if (NOT SPEECHTOOLS_STRING_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_STRING_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_STRING_LIBRARY)
  set (FESTIVAL_LIB_FILE "libFestival.a")
  find_library (FESTIVAL_LIBRARY ${FESTIVAL_LIB_FILE}
                PATHS /usr
                PATH_SUFFIXES lib64
                DOC "searching for ${FESTIVAL_LIB_FILE}")
  if (NOT FESTIVAL_LIBRARY)
   message (WARNING "could not find ${FESTIVAL_LIB_FILE}, continuing")
  endif (NOT FESTIVAL_LIBRARY)
  if (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY)
   message (STATUS "Found ${FESTIVAL_LIB_FILE} library \"${FESTIVAL_LIBRARY}\"")
   set (FESTIVAL_FOUND TRUE)
   set (FESTIVAL_INCLUDE_DIRS "/usr/include/speech_tools")
   set (FESTIVAL_LIBRARIES "${SPEECHTOOLS_BASE_LIBRARY};${SPEECHTOOLS_LIBRARY};${SPEECHTOOLS_STRING_LIBRARY};${FESTIVAL_LIBRARY}")
#   set (FESTIVAL_LIB_DIR "$ENV{LIB_ROOT}/festival/lib")
  endif (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY) 
 endif (PKG_FESTIVAL_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
#  cmake_policy (SET CMP0074 OLD)
  find_package (festival CONFIG)
  if (festival_FOUND)
   set (FESTIVAL_FOUND TRUE)
   set (FESTIVAL_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR_BASE})
   set (FESTIVAL_LIB_DIR ${VCPKG_LIB_DIR}/bin)
  endif (festival_FOUND)
 endif (VCPKG_USE)
 if (NOT festival_FOUND)
  set (SPEECHTOOLS_BASE_LIB_FILE "libestbase.lib")
  find_library (SPEECHTOOLS_BASE_LIBRARY ${SPEECHTOOLS_BASE_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_BASE_LIB_FILE}")
  if (NOT SPEECHTOOLS_BASE_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_BASE_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_BASE_LIBRARY)
  set (SPEECHTOOLS_LIB_FILE "libestools.lib")
  find_library (SPEECHTOOLS_LIBRARY ${SPEECHTOOLS_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_LIB_FILE}")
  if (NOT SPEECHTOOLS_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_LIBRARY)
  set (SPEECHTOOLS_STRING_LIB_FILE "libeststring.lib")
  find_library (SPEECHTOOLS_STRING_LIBRARY ${SPEECHTOOLS_STRING_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/speech_tools/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${SPEECHTOOLS_STRING_LIB_FILE}")
  if (NOT SPEECHTOOLS_STRING_LIBRARY)
   message (WARNING "could not find ${SPEECHTOOLS_STRING_LIB_FILE}, continuing")
  endif (NOT SPEECHTOOLS_STRING_LIBRARY)
  set (FESTIVAL_LIB_FILE "libFestival.lib")
  find_library (FESTIVAL_LIBRARY ${FESTIVAL_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/festival/src/lib
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${FESTIVAL_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT FESTIVAL_LIBRARY)
   message (WARNING "could not find ${FESTIVAL_LIB_FILE}, continuing")
  endif (NOT FESTIVAL_LIBRARY)
  if (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY)
   message (STATUS "Found ${FESTIVAL_LIB_FILE} library \"${FESTIVAL_LIBRARY}\"")
   set (FESTIVAL_FOUND TRUE)
   set (FESTIVAL_INCLUDE_DIRS "$ENV{LIB_ROOT}/speech_tools/include;$ENV{LIB_ROOT}/festival/src/include")
   set (FESTIVAL_LIBRARIES "${SPEECHTOOLS_BASE_LIBRARY};${SPEECHTOOLS_LIBRARY};${SPEECHTOOLS_STRING_LIBRARY};${FESTIVAL_LIBRARY}")
#   set (FESTIVAL_LIB_DIR "$ENV{LIB_ROOT}/festival/src/lib")
  endif (SPEECHTOOLS_BASE_LIBRARY AND SPEECHTOOLS_LIBRARY AND SPEECHTOOLS_STRING_LIBRARY AND FESTIVAL_LIBRARY)
 endif (NOT festival_FOUND)
endif ()
if (FESTIVAL_FOUND)
 option (FESTIVAL_SUPPORT "enable festival support" ${FESTIVAL_SUPPORT_DEFAULT})
 if (FESTIVAL_SUPPORT)
  add_definitions (-DFESTIVAL_SUPPORT)
  if (WIN32)
   add_definitions (-DSYSTEM_IS_WIN32)
  endif (WIN32)
#  include_directories (${FESTIVAL_INCLUDE_DIRS})
 endif (FESTIVAL_SUPPORT)
endif (FESTIVAL_FOUND)

##########################################

set (FLITE_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_FLITE flite)
 if (PKG_FLITE_FOUND)
  set (FLITE_FOUND TRUE)
  set (FLITE_INCLUDE_DIRS "${PKG_FLITE_INCLUDE_DIRS}")
  set (FLITE_LIBRARIES "${PKG_FLITE_LIBRARIES}")
 else ()
  set (FLITE_LIB_FILE "libflite.a")
  find_library (FLITE_LIBRARY ${FLITE_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/build/x86_64-linux-gnu
                PATH_SUFFIXES lib
                DOC "searching for ${FLITE_LIB_FILE}")
  if (NOT FLITE_LIBRARY)
   message (WARNING "could not find ${FLITE_LIB_FILE}, continuing")
  endif (NOT FLITE_LIBRARY)
  set (CMU_LEX_LIB_FILE "libflite_cmulex.a")
  find_library (CMU_LEX_LIBRARY ${CMU_LEX_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/build/x86_64-linux-gnu
                PATH_SUFFIXES lib
                DOC "searching for ${CMU_LEX_LIB_FILE}")
  if (NOT CMU_LEX_LIBRARY)
   message (WARNING "could not find ${CMU_LEX_LIB_FILE}, continuing")
  endif (NOT CMU_LEX_LIBRARY)
  set (USENGLISH_LIB_FILE "libflite_usenglish.a")
  find_library (USENGLISH_LIBRARY ${USENGLISH_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/build/x86_64-linux-gnu
                PATH_SUFFIXES lib
                DOC "searching for ${USENGLISH_LIB_FILE}")
  if (NOT USENGLISH_LIBRARY)
   message (WARNING "could not find ${USENGLISH_LIB_FILE}, continuing")
  endif (NOT USENGLISH_LIBRARY)
  set (CMU_GRAPHEME_LANG_LIB_FILE "libflite_cmu_grapheme_lang.a")
  find_library (CMU_GRAPHEME_LANG_LIBRARY ${CMU_GRAPHEME_LANG_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/build/x86_64-linux-gnu
                PATH_SUFFIXES lib
                DOC "searching for ${CMU_GRAPHEME_LANG_LIB_FILE}")
  if (NOT CMU_GRAPHEME_LANG_LIBRARY)
   message (WARNING "could not find ${CMU_GRAPHEME_LANG_LIB_FILE}, continuing")
  endif (NOT CMU_GRAPHEME_LANG_LIBRARY)
  set (CMU_GRAPHEME_LEX_LIB_FILE "libflite_cmu_grapheme_lex.a")
  find_library (CMU_GRAPHEME_LEX_LIBRARY ${CMU_GRAPHEME_LEX_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/build/x86_64-linux-gnu
                PATH_SUFFIXES lib
                DOC "searching for ${CMU_GRAPHEME_LEX_LIB_FILE}")
  if (NOT CMU_GRAPHEME_LEX_LIBRARY)
   message (WARNING "could not find ${CMU_GRAPHEME_LEX_LIB_FILE}, continuing")
  endif (NOT CMU_GRAPHEME_LEX_LIBRARY)
  if (FLITE_LIBRARY AND CMU_LEX_LIBRARY AND USENGLISH_LIBRARY AND CMU_GRAPHEME_LANG_LIBRARY AND CMU_GRAPHEME_LEX_LIBRARY)
   set (FLITE_FOUND TRUE)
   set (FLITE_INCLUDE_DIRS "$ENV{LIB_ROOT}/flite/include;$ENV{LIB_ROOT}/flite/lang")
   set (FLITE_LIBRARIES "${FLITE_LIBRARY};${CMU_LEX_LIBRARY};${USENGLISH_LIBRARY};${CMU_GRAPHEME_LANG_LIBRARY};${CMU_GRAPHEME_LEX_LIBRARY}")
#   set (FLITE_LIB_DIR "$ENV{LIB_ROOT}/flite/bin")
  endif (FLITE_LIBRARY AND CMU_LEX_LIBRARY AND USENGLISH_LIBRARY AND CMU_GRAPHEME_LANG_LIBRARY AND CMU_GRAPHEME_LEX_LIBRARY)
 endif (PKG_FLITE_FOUND)
elseif (WIN32)
 set (PATH_SUFFIX "Release")
 if (DEFINED CMAKE_BUILD_TYPE)
  set (PATH_SUFFIX ${CMAKE_BUILD_TYPE})
 endif (DEFINED CMAKE_BUILD_TYPE)
 if (VCPKG_USE)
#  cmake_policy (SET CMP0074 OLD)
  find_package (flite CONFIG)
  if (FLITE_FOUND)
   set (FLITE_FOUND TRUE)
   if (${PATH_SUFFIX} STREQUAL "Debug" OR
       ${PATH_SUFFIX} STREQUAL "RelWithDebInfo")
    set (FLITE_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/bin")
   else ()
    set (FLITE_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin")
   endif (${PATH_SUFFIX} STREQUAL "Debug" OR
          ${PATH_SUFFIX} STREQUAL "RelWithDebInfo")
  endif (FLITE_FOUND)
 endif (VCPKG_USE)
 if (NOT FLITE_FOUND)
  set (FLITE_LIB_FILE "libflite.lib")
  find_library (FLITE_LIBRARY ${FLITE_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}
                PATH_SUFFIXES ${PATH_SUFFIX}
                DOC "searching for ${FLITE_LIB_FILE}")
  if (NOT FLITE_LIBRARY)
   message (WARNING "could not find ${FLITE_LIB_FILE}, continuing")
  endif (NOT FLITE_LIBRARY)
  set (CMU_LEX_LIB_FILE "libflite_cmulex.lib")
  find_library (CMU_LEX_LIBRARY ${CMU_LEX_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}
                PATH_SUFFIXES ${PATH_SUFFIX}
                DOC "searching for ${CMU_LEX_LIB_FILE}")
  if (NOT CMU_LEX_LIBRARY)
   message (WARNING "could not find ${CMU_LEX_LIB_FILE}, continuing")
  endif (NOT CMU_LEX_LIBRARY)
  set (USENGLISH_LIB_FILE "libflite_usenglish.lib")
  find_library (USENGLISH_LIBRARY ${USENGLISH_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}
                PATH_SUFFIXES ${PATH_SUFFIX}
                DOC "searching for ${USENGLISH_LIB_FILE}")
  if (NOT USENGLISH_LIBRARY)
   message (WARNING "could not find ${USENGLISH_LIB_FILE}, continuing")
  endif (NOT USENGLISH_LIBRARY)
  set (CMU_GRAPHEME_LANG_LIB_FILE "libflite_cmu_grapheme_lang.lib")
  find_library (CMU_GRAPHEME_LANG_LIBRARY ${CMU_GRAPHEME_LANG_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}
                PATH_SUFFIXES ${PATH_SUFFIX}
                DOC "searching for ${CMU_GRAPHEME_LANG_LIB_FILE}")
  if (NOT CMU_GRAPHEME_LANG_LIBRARY)
   message (WARNING "could not find ${CMU_GRAPHEME_LANG_LIB_FILE}, continuing")
  endif (NOT CMU_GRAPHEME_LANG_LIBRARY)
  set (CMU_GRAPHEME_LEX_LIB_FILE "libflite_cmu_grapheme_lex.lib")
  find_library (CMU_GRAPHEME_LEX_LIBRARY ${CMU_GRAPHEME_LEX_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/flite/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}
                PATH_SUFFIXES ${PATH_SUFFIX}
                DOC "searching for ${CMU_GRAPHEME_LEX_LIB_FILE}")
  if (NOT CMU_GRAPHEME_LEX_LIBRARY)
   message (WARNING "could not find ${CMU_GRAPHEME_LEX_LIB_FILE}, continuing")
  endif (NOT CMU_GRAPHEME_LEX_LIBRARY)
  if (FLITE_LIBRARY AND CMU_LEX_LIBRARY AND USENGLISH_LIBRARY AND CMU_GRAPHEME_LANG_LIBRARY AND CMU_GRAPHEME_LEX_LIBRARY)
   set (FLITE_FOUND TRUE)
   set (FLITE_INCLUDE_DIRS "$ENV{LIB_ROOT}/flite/include;$ENV{LIB_ROOT}/flite/lang")
   set (FLITE_LIBRARIES "${FLITE_LIBRARY};${CMU_LEX_LIBRARY};${USENGLISH_LIBRARY};${CMU_GRAPHEME_LANG_LIBRARY};${CMU_GRAPHEME_LEX_LIBRARY}")
#   set (FLITE_LIB_DIR "$ENV{LIB_ROOT}/flite/build/x86_64-mingw32/bin")
  endif (FLITE_LIBRARY AND CMU_LEX_LIBRARY AND USENGLISH_LIBRARY AND CMU_GRAPHEME_LANG_LIBRARY AND CMU_GRAPHEME_LEX_LIBRARY)
 endif (NOT FLITE_FOUND)
endif ()
if (FLITE_FOUND)
 option (FLITE_SUPPORT "enable flite support" ${FLITE_SUPPORT_DEFAULT})
 if (FLITE_SUPPORT)
  add_definitions (-DFLITE_SUPPORT)
 endif (FLITE_SUPPORT)
endif (FLITE_FOUND)
