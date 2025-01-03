if (WIN32)
# DirectShow base classes
 if (DEFINED ENV{LIB_ROOT})
  set (DIRECTSHOW_BASECLASSES_DIRECTORY "$ENV{LIB_ROOT}/DShowBaseClasses")
 endif (DEFINED ENV{LIB_ROOT})
#      "$ENV{PROGRAMFILES}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
#      "$ENV{${_PF86}}/Microsoft SDKs/Windows/v7.0/Samples/multimedia/directshow/baseclasses")
# message (STATUS "DirectShow baseclasses directory: \"${DIRECTSHOW_BASECLASSES_DIRECTORY}\"")

 set (LIB_FILE_SUFFIX "")
 set (PATH_SUFFIX "Release")
 if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set (LIB_FILE_SUFFIX "d")
  set (PATH_SUFFIX "Debug")
 endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
 set (DIRECTSHOW_BASECLASSES_LIB_FILE BaseClasses${LIB_FILE_SUFFIX}.lib)
# *TODO*: add unicode support
 find_library (BASECLASSES_LIBRARY ${DIRECTSHOW_BASECLASSES_LIB_FILE}
               PATHS ${DIRECTSHOW_BASECLASSES_DIRECTORY}/build/msvc/baseclasses
               PATH_SUFFIXES ${PATH_SUFFIX}
               DOC "searching for \"${DIRECTSHOW_BASECLASSES_LIB_FILE}\""
               NO_DEFAULT_PATH)
 if (BASECLASSES_LIBRARY)
  set (DS_BASECLASSES_FOUND TRUE)
  set (DS_BASECLASSES_INCLUDE_DIRS "${DIRECTSHOW_BASECLASSES_DIRECTORY}/baseclasses")
  set (DS_BASECLASSES_LIBRARIES ${BASECLASSES_LIBRARY})
 endif (BASECLASSES_LIBRARY)

 if (DS_BASECLASSES_FOUND)
  option (DIRECTSHOW_BASECLASSES_SUPPORT "enable DirectShow base classes support" ON)
  if (DIRECTSHOW_BASECLASSES_SUPPORT)
   add_definitions (-DDIRECTSHOW_BASECLASSES_SUPPORT)
  endif (DIRECTSHOW_BASECLASSES_SUPPORT)
 endif (DS_BASECLASSES_FOUND)
endif (WIN32)
