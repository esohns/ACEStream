set (SAPI_SUPPORT_DEFAULT ON)
if (WIN32)
 if (VCPKG_USE)
#  cmake_policy (SET CMP0074 OLD)
#  find_package (sapi CONFIG)
  find_library (SAPI_LIBRARY sapi.lib
                PATHS ${VCPKG_LIB_DIR}
                PATH_SUFFIXES lib
                DOC "searching for sapi.lib"
                NO_DEFAULT_PATH)
  if (SAPI_LIBRARY)
   set (SAPI_FOUND TRUE)
   set (SAPI_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR_BASE})
   set (SAPI_LIBRARIES "${SAPI_LIBRARY}")
   set (SAPI_LIB_DIR ${VCPKG_LIB_DIR}/bin)
  endif (SAPI_LIBRARY)
 endif (VCPKG_USE)
 if (NOT SAPI_FOUND)
  set (SAPI_LIB_FILE "sapi.lib")
  find_library (SAPI_LIBRARY ${SAPI_LIB_FILE}
                PATHS "$ENV{ProgramFiles}/Microsoft SDKs/Speech/v11.0"
                PATH_SUFFIXES Lib
                DOC "searching for ${SAPI_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT SAPI_LIBRARY)
   message (WARNING "could not find ${SAPI_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${SAPI_LIB_FILE} library \"${SAPI_LIBRARY}\"")
   set (SAPI_FOUND TRUE)
   set (SAPI_INCLUDE_DIRS "$ENV{ProgramFiles}/Microsoft SDKs/Speech/v11.0/Include")
   set (SAPI_LIBRARIES "${SAPI_LIBRARY}")
   set (SAPI_LIB_DIR "")
  endif (NOT SAPI_LIBRARY)
 endif (NOT SAPI_FOUND)
endif (WIN32)
if (SAPI_FOUND)
 option (SAPI_SUPPORT "enable Microsoft Speech API support" ${SAPI_SUPPORT_DEFAULT})
 if (SAPI_SUPPORT)
  add_definitions (-DSAPI_SUPPORT)
#  include_directories (${faad_INCLUDE_DIRS})
 endif (SAPI_SUPPORT)
endif (SAPI_FOUND)
