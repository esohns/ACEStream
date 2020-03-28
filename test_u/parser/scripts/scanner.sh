#!/bin/sh
#//%%%FILE%%%////////////////////////////////////////////////////////////////////
#// File Name: scanner.sh
#//
#// History:
#//   Date   |Name | Description of modification
#// ---------|-----|-------------------------------------------------------------
#// 20/02/06 | soh | Creation.
#//%%%FILE%%%////////////////////////////////////////////////////////////////////

# sanity check(s)
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v flex >/dev/null 2>&1 || { echo "flex is not installed, aborting" >&2; exit 1; }
command -v tee >/dev/null 2>&1 || { echo "tee is not installed, aborting" >&2; exit 1; }

PROJECT_ROOT=$(dirname $0)/..
#[ ! -d ./${PROJECT_ROOT} ] && echo "ERROR: invalid directory (was: ${PROJECT_ROOT}), aborting" && exit 1
SCRIPTS_DIRECTORY=${PROJECT_ROOT}/scripts
[ ! -d ${SCRIPTS_DIRECTORY} ] && echo "ERROR: invalid directory (was: ${SCRIPTS_DIRECTORY}), aborting" && exit 1
BENCODING_L=bencoding.l
[ ! -f ${SCRIPTS_DIRECTORY}/${BENCODING_L} ] && echo "ERROR: invalid file (was: ${SCRIPTS_DIRECTORY}/${BENCODING_L}), aborting" && exit 1

# generate a scanner for use by the bittorrent metainfo file parser
flex --noline ${SCRIPTS_DIRECTORY}/${BENCODING_L} 2>&1 | tee ${SCRIPTS_DIRECTORY}/bencoding_report.txt
[ $? -ne 0 ] && echo "ERROR: failed to flex \"${BENCODING_L}\", aborting" && exit 1

# list generated files
#FILES="bittorrent_bencoding_scanner.cpp bittorrent_bencoding_scanner.h"
FILES="bencoding_scanner.cpp bencoding_scanner.h"

# move the files into the project directory
for FILE in $FILES
do
mv -f $FILE ${SCRIPTS_DIRECTORY}/..
if [ $? -ne 0 ]; then
 echo "ERROR: failed to mv \"$FILE\", aborting"
 exit 1
fi
echo "moved \"$FILE\"..."
done

