#!/bin/sh
#//%%%FILE%%%////////////////////////////////////////////////////////////////////
#// File Name: start_soffice.sh
#//
#// History:
#//   Date   |Name | Description of modification
#// ---------|-----|-------------------------------------------------------------
#// 20/02/06 | soh | Creation.
#//%%%FILE%%%////////////////////////////////////////////////////////////////////

# sanity check(s)
command -v killall /dev/null 2>&1 || { echo "killall is not installed, aborting" >&2; exit 1; }

killall soffice.bin 2>&1
[ $? -ne 0 ] && echo "ERROR: \"killall\" failed (status was: $?), aborting" && exit 1

