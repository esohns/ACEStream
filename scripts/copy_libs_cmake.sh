#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script gathers the dependent libraries in one place for in-source-tree
# debugging
# *NOTE*: it is neither portable nor particularly stable !
# parameters:   - $1 [BUILD] {"Debug" || "Valgrind" || "Release" || ...}
# return value: - 0 success, 1 failure

# sanity check(s)


DEFAULT_PROJECT_DIR="$(dirname $(readlink -f $0))/.."
PROJECT_DIR=${DEFAULT_PROJECT_DIR}
# sanity check(s)
[ ! -d ${PROJECT_DIR} ] && echo "ERROR: invalid project dir (was: \"${PROJECT_DIR}\"), aborting" && exit 1

DEFAULT_BUILD="Debug"
BUILD=${DEFAULT_BUILD}
if [ $# -lt 1 ]; then
 echo "INFO: using default build: \"${BUILD}\""
else
 # parse any arguments
 if [ $# -ge 1 ]; then
  BUILD="$1"
 fi
fi

# sanity check(s)
[ ${BUILD} != "Debug" -a ${BUILD} != "Valgrind" -a ${BUILD} != "Release" ] && echo "WARNING: invalid/unknown build (was: \"${BUILD}\"), continuing"
BUILD=$(echo ${BUILD} | tr '[:upper:]' '[:lower:]')
BUILD_DIR="${PROJECT_DIR}/cmake"
[ ! -d "${BUILD_DIR}" ] && echo "ERROR: invalid build dir (was: \"${BUILD_DIR}\"), aborting" && exit 1

LIB_DIR="lib"
TARGET_DIR="${BUILD_DIR}/${LIB_DIR}"
if [ ! -d "${TARGET_DIR}" ]; then
 mkdir ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to mkdir \"${TARGET_DIR}\", aborting" && exit 1
 echo "INFO: created directory \"${TARGET_DIR}\", continuing"
fi
VERSION="6.3.3"

echo "copying 3rd-party libraries"
LIB_DIR="lib"
MODULES_DIR="${PROJECT_DIR}/modules"
SUB_DIRS="ACE/build/linux"
#declare -a LIBS=("libACE.so")
LIBS="libACE.so"
i=0
for DIR in $SUB_DIRS
do
# LIB="${MODULES_DIR}/${DIR}/${LIB_DIR}/${LIBS[$i]}"
 LIB="${MODULES_DIR}/${DIR}/${LIB_DIR}/${LIBS}"
 [ ! -r "${LIB}.${VERSION}" ] && echo "ERROR: invalid library file (was: \"${LIB}.${VERSION}\"), aborting" && exit 1
 cp -f "${LIB}.${VERSION}" ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to copy \"${LIB}.${VERSION}\" to \"${TARGET_DIR}\": $?, aborting" && exit 1
 echo "copied \"$LIB.${VERSION}\"..."

 i=$i+1
done
VERSION="0.0.1"

echo "copying external module libraries"
SUB_DIRS="modules/Common/cmake/src
modules/Common/cmake/src/ui
src/modules/net/modules/ACENetwork/cmake/src
src/modules/net/modules/ACENetwork/cmake/src/client_server"
#declare -a LIBS=("libCommon.so"
LIBS="libCommon.so libCommonUI.so libACENetwork.so libACENetwork_Client.so"
set -- $LIBS
#i=0
for DIR in $SUB_DIRS
do
# LIB="${BUILD_DIR}/${DIR}/${LIB_DIR}/${LIBS[$i]}"
 LIB="${PROJECT_DIR}/${DIR}/$1"

 [ ! -r "${LIB}.${VERSION}" ] && echo "ERROR: invalid library file (was: \"${LIB}.${VERSION}\"), aborting" && exit 1
 cp -f "${LIB}.${VERSION}" ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to copy \"${LIB}.${VERSION}\" to \"${TARGET_DIR}\": $?, aborting" && exit 1
 echo "copied \"$LIB.${VERSION}\"..."
# i=$i+1
 shift
done

echo "copying framework libraries"
SUB_DIRS="src
src/modules/dev
src/modules/file
src/modules/misc
src/modules/net
src/modules/net/protocols"
#declare -a LIBS=("libACENetwork.so"
LIBS="libACEStream.so
libACEStream_Device.so
libACEStream_File.so
libACEStream_Misc.so
libACEStream_Net.so
libACEStream_Net_Protocol_HTTPGet.so"
set -- $LIBS
#i=0
for DIR in $SUB_DIRS
do
# LIB="${BUILD_DIR}/${DIR}/${LIB_DIR}/${LIBS[$i]}"
 LIB="${BUILD_DIR}/${DIR}/$1"
 [ ! -r "${LIB}.${VERSION}" ] && echo "ERROR: invalid library file (was: \"${LIB}.${VERSION}\"), aborting" && exit 1
 cp -f "${LIB}.${VERSION}" ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to copy \"${LIB}.${VERSION}\" to \"${TARGET_DIR}\": $?, aborting" && exit 1
 echo "copied \"$LIB.${VERSION}\"..."
# i=$i+1
 shift
done
