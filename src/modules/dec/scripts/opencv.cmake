set (OPENCV_SUPPORT_DEFAULT ON)
if (UNIX)
 include (FindPkgConfig)
 pkg_check_modules (PKG_OPENCV opencv)
 if (PKG_OPENCV_FOUND)
  set (OPENCV_FOUND TRUE)
  set (OPENCV_INCLUDE_DIRS "${PKG_OPENCV_INCLUDE_DIRS}")
  set (OPENCV_LIBRARIES "${PKG_OPENCV_LIBRARIES}")
 endif (PKG_OPENCV_FOUND)
elseif (WIN32)
 if (VCPKG_USE)
#  cmake_policy (SET CMP0074 OLD)
  find_package (opencv CONFIG)
  if (opencv_FOUND)
   set (OPENCV_FOUND TRUE)
   set (OPENCV_INCLUDE_DIRS ${VCPKG_INCLUDE_DIR_BASE})
   set (OPENCV_LIB_DIR ${VCPKG_LIB_DIR}/bin)
  endif (opencv_FOUND)
 endif (VCPKG_USE)
 if (NOT OPENCV_FOUND)
  set (OPENCV_VERSION "460")
  set (OPENCV_CORE_LIB_FILE "opencv_core${OPENCV_VERSION}${LIB_FILE_SUFFIX}.lib")
  find_library (OPENCV_CORE_LIBRARY ${OPENCV_CORE_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/opencv/build/lib/
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OPENCV_CORE_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT OPENCV_CORE_LIBRARY)
   message (WARNING "could not find ${OPENCV_CORE_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${OPENCV_CORE_LIB_FILE} library \"${OPENCV_CORE_LIBRARY}\"")
  endif (NOT OPENCV_CORE_LIBRARY)
  set (OPENCV_HIGHGUI_LIB_FILE "opencv_highgui${OPENCV_VERSION}${LIB_FILE_SUFFIX}.lib")
  find_library (OPENCV_HIGHGUI_LIBRARY ${OPENCV_HIGHGUI_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/opencv/build/lib/
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OPENCV_HIGHGUI_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT OPENCV_HIGHGUI_LIBRARY)
   message (WARNING "could not find ${OPENCV_HIGHGUI_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${OPENCV_HIGHGUI_LIB_FILE} library \"${OPENCV_HIGHGUI_LIBRARY}\"")
  endif (NOT OPENCV_HIGHGUI_LIBRARY)
  set (OPENCV_IMGCODECS_LIB_FILE "opencv_imgcodecs${OPENCV_VERSION}${LIB_FILE_SUFFIX}.lib")
  find_library (OPENCV_IMGCODECS_LIBRARY ${OPENCV_IMGCODECS_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/opencv/build/lib/
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OPENCV_IMGCODECS_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT OPENCV_IMGCODECS_LIBRARY)
   message (WARNING "could not find ${OPENCV_IMGCODECS_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${OPENCV_IMGCODECS_LIB_FILE} library \"${OPENCV_IMGCODECS_LIBRARY}\"")
  endif (NOT OPENCV_IMGCODECS_LIBRARY)
  set (OPENCV_IMGPROC_LIB_FILE "opencv_imgproc${OPENCV_VERSION}${LIB_FILE_SUFFIX}.lib")
  find_library (OPENCV_IMGPROC_LIBRARY ${OPENCV_IMGPROC_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/opencv/build/lib/
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OPENCV_IMGPROC_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT OPENCV_IMGPROC_LIBRARY)
   message (WARNING "could not find ${OPENCV_IMGPROC_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${OPENCV_IMGPROC_LIB_FILE} library \"${OPENCV_IMGPROC_LIBRARY}\"")
  endif (NOT OPENCV_IMGPROC_LIBRARY)
  set (OPENCV_OBJDETECT_LIB_FILE "opencv_objdetect${OPENCV_VERSION}${LIB_FILE_SUFFIX}.lib")
  find_library (OPENCV_OBJDETECT_LIBRARY ${OPENCV_OBJDETECT_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/opencv/build/lib/
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OPENCV_OBJDETECT_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT OPENCV_OBJDETECT_LIBRARY)
   message (WARNING "could not find ${OPENCV_OBJDETECT_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${OPENCV_OBJDETECT_LIB_FILE} library \"${OPENCV_OBJDETECT_LIBRARY}\"")
  endif (NOT OPENCV_OBJDETECT_LIBRARY)
  set (OPENCV_VIDEOIO_LIB_FILE "opencv_videoio${OPENCV_VERSION}${LIB_FILE_SUFFIX}.lib")
  find_library (OPENCV_VIDEOIO_LIBRARY ${OPENCV_VIDEOIO_LIB_FILE}
                PATHS $ENV{LIB_ROOT}/opencv/build/lib/
                PATH_SUFFIXES ${CMAKE_BUILD_TYPE}
                DOC "searching for ${OPENCV_VIDEOIO_LIB_FILE}"
                NO_DEFAULT_PATH)
  if (NOT OPENCV_VIDEOIO_LIBRARY)
   message (WARNING "could not find ${OPENCV_VIDEOIO_LIB_FILE}, continuing")
  else ()
   message (STATUS "Found ${OPENCV_VIDEOIO_LIB_FILE} library \"${OPENCV_VIDEOIO_LIBRARY}\"")
  endif (NOT OPENCV_VIDEOIO_LIBRARY)
  if (OPENCV_CORE_LIBRARY AND OPENCV_HIGHGUI_LIBRARY AND OPENCV_IMGCODECS_LIBRARY AND OPENCV_IMGPROC_LIBRARY AND OPENCV_OBJDETECT_LIBRARY AND OPENCV_VIDEOIO_LIBRARY)
   set (OPENCV_FOUND TRUE)
   set (OPENCV_INCLUDE_DIRS "$ENV{LIB_ROOT}/opencv/include;$ENV{LIB_ROOT}/opencv/modules/core/include;$ENV{LIB_ROOT}/opencv/modules/highgui/include;$ENV{LIB_ROOT}/opencv/modules/imgcodecs/include;$ENV{LIB_ROOT}/opencv/modules/imgproc/include;$ENV{LIB_ROOT}/opencv/modules/objdetect/include;$ENV{LIB_ROOT}/opencv/modules/videoio/include;$ENV{LIB_ROOT}/opencv/build")
   set (OPENCV_LIBRARIES "${OPENCV_CORE_LIBRARY};${OPENCV_HIGHGUI_LIBRARY};${OPENCV_IMGCODECS_LIBRARY};${OPENCV_IMGPROC_LIBRARY};${OPENCV_OBJDETECT_LIBRARY};${OPENCV_VIDEOIO_LIBRARY}")
   set (OPENCV_LIB_DIR "$ENV{LIB_ROOT}/opencv/build/bin/${CMAKE_BUILD_TYPE}")
  endif (OPENCV_CORE_LIBRARY AND OPENCV_HIGHGUI_LIBRARY AND OPENCV_IMGCODECS_LIBRARY AND OPENCV_IMGPROC_LIBRARY AND OPENCV_OBJDETECT_LIBRARY AND OPENCV_VIDEOIO_LIBRARY)
 endif (NOT OPENCV_FOUND)
endif ()
if (OPENCV_FOUND)
 option (OPENCV_SUPPORT "enable opencv support" ${OPENCV_SUPPORT_DEFAULT})
 if (OPENCV_SUPPORT)
  add_definitions (-DOPENCV_SUPPORT)
#  include_directories (${OPENCV_INCLUDE_DIRS})
  if (WIN32)
# *TODO*: this is a work-around for opencv support with vs2010 (requires __cplusplus >= 201103L)
#         --> remove ASAP
#  add_definitions (-DCV_CXX11)
  endif (WIN32)
 endif (OPENCV_SUPPORT)
endif (OPENCV_FOUND)
