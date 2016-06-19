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
command -v soffice /dev/null 2>&1 || { echo "libreoffice is not installed, aborting" >&2; exit 1; }

soffice --accept="socket,host=localhost,port=2083;urp;StarOffice.ServiceManager" --nofirststartwizard --nologo --headless --norestore --invisible --display :1 --backtrace >>/var/tmp/libreoffice.log 2>&1
[ $? -ne 0 ] && echo "ERROR: \"soffice\" failed (status was: $?), aborting" && exit 1

