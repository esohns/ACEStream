set (DEEPSPEECH_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_DEEPSPEECH deepspeech)
 if (PKG_DEEPSPEECH_FOUND)
  set (DEEPSPEECH_FOUND TRUE)
  set (DEEPSPEECH_INCLUDE_DIRS "${PKG_DEEPSPEECH_INCLUDE_DIRS}")
  set (DEEPSPEECH_LIBRARIES "${PKG_DEEPSPEECH_LIBRARIES}")
 else ()
  set (DEEPSPEECH_LIB_FILE "libdeepspeech.so")
  find_library (DEEPSPEECH_LIBRARY ${DEEPSPEECH_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/DeepSpeech/tensorflow/bazel-bin
                PATH_SUFFIXES native_client
                DOC "searching for ${DEEPSPEECH_LIB_FILE}")
  set (FST_LIB_FILE "libfst.so.10.0.0")
  find_library (FST_LIBRARY ${FST_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/DeepSpeech/native_client/ctcdecode/third_party/openfst-1.6.7/src/lib
                PATH_SUFFIXES .libs
                DOC "searching for ${FST_LIB_FILE}")
  # set (DECODER_3_LIB_FILE "third_party.a")
  # find_library (DECODER_3_LIBRARY ${DECODER_3_LIB_FILE}
  #               PATHS $ENV{LIB_ROOT}/DeepSpeech/native_client
  #               PATH_SUFFIXES ctcdecode
  #               DOC "searching for ${DECODER_3_LIB_FILE}")
  # set (DECODER_1_LIB_FILE "first_party.a")
  # find_library (DECODER_1_LIBRARY ${DECODER_1_LIB_FILE}
  #               PATHS $ENV{LIB_ROOT}/DeepSpeech/native_client
  #               PATH_SUFFIXES ctcdecode
  #               DOC "searching for ${DECODER_1_LIB_FILE}")
  if (NOT DEEPSPEECH_LIBRARY OR NOT FST_LIBRARY)
   message (WARNING "could not find ${DEEPSPEECH_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${DEEPSPEECH_LIB_FILE} library \"${DEEPSPEECH_LIBRARY}\"")
   message (STATUS "Found ${FST_LIB_FILE} library \"${FST_LIBRARY}\"")
   # message (STATUS "Found ${DECODER_3_LIB_FILE} library \"${DECODER_3_LIBRARY}\"")
   # message (STATUS "Found ${DECODER_1_LIB_FILE} library \"${DECODER_1_LIBRARY}\"")
   set (DEEPSPEECH_FOUND TRUE)
   set (DEEPSPEECH_INCLUDE_DIRS "$ENV{LIB_ROOT}/DeepSpeech/native_client")
   set (DEEPSPEECH_LIBRARIES "${DEEPSPEECH_LIBRARY};${FST_LIBRARY}")
#   set (DEEPSPEECH_LIB_DIR "$ENV{LIB_ROOT}/DeepSpeech/native_client")
  endif (NOT DEEPSPEECH_LIBRARY OR NOT FST_LIBRARY)
 endif (PKG_DEEPSPEECH_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
#  cmake_policy (SET CMP0074 OLD)
  find_package (deepspeech CONFIG)
  if (deepspeech_FOUND)
   set (DEEPSPEECH_FOUND TRUE)
   set (DEEPSPEECH_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR_BASE})
   set (DEEPSPEECH_LIB_DIR ${VCPKG_LIB_DIR}/bin)
  endif (deepspeech_FOUND)
 endif (VCPKG_USE)
 if (NOT deepspeech_FOUND)
  set (DEEPSPEECH_LIB_FILE "libdeepspeech.so.if.lib")
  find_library (DEEPSPEECH_LIBRARY ${DEEPSPEECH_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/DeepSpeech/native_client/windows/bin
#                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${DEEPSPEECH_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT DEEPSPEECH_LIBRARY)
   message (WARNING "could not find ${DEEPSPEECH_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${DEEPSPEECH_LIB_FILE} library \"${DEEPSPEECH_LIBRARY}\"")
   set (DEEPSPEECH_FOUND TRUE)
   set (DEEPSPEECH_INCLUDE_DIRS "$ENV{LIB_ROOT}/DeepSpeech/native_client")
   set (DEEPSPEECH_LIBRARIES "${DEEPSPEECH_LIBRARY}")
   set (DEEPSPEECH_LIB_DIR "$ENV{LIB_ROOT}/DeepSpeech/native_client/windows/bin")
  endif (NOT DEEPSPEECH_LIBRARY)
 endif (NOT deepspeech_FOUND)
endif ()
if (DEEPSPEECH_FOUND)
 option (DEEPSPEECH_SUPPORT "enable deepspeech support" ${DEEPSPEECH_SUPPORT_DEFAULT})
 if (DEEPSPEECH_SUPPORT)
  add_definitions (-DDEEPSPEECH_SUPPORT)
#  include_directories (${DEEPSPEECH_INCLUDE_DIRS})
 endif (DEEPSPEECH_SUPPORT)
endif (DEEPSPEECH_FOUND)

