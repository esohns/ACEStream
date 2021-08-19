#!/bin/sh
# //%%%FILE%%%////////////////////////////////////////////////////////////////////
# // File Name: make_oo_headers.sh
# //
# // History:
# //   Date   |Name | Description of modification
# // ---------|-----|-------------------------------------------------------------
# // 20/02/06 | soh | Creation.
# //%%%FILE%%%////////////////////////////////////////////////////////////////////

# sanity check(s)
command -v basename >/dev/null 2>&1 || { echo "basename is not installed, aborting" >&2; exit 1; }

#OO_SDK_HOME=/usr/lib64/libreoffice/sdk # Fedora
OO_SDK_HOME=/usr/lib/libreoffice/sdk # Ubuntu
if [ ! -d ${OO_SDK_HOME} ]
then
 echo "invalid OO SDK directory (was: \"${OO_SDK_HOME}\"), exiting"
 exit
fi
OO_SDK_INCLUDE=${OO_SDK_HOME}/include
#OO_SDK_INCLUDE=/usr/include/libreoffice
if [ ! -d ${OO_SDK_INCLUDE} ]
then
 echo "invalid OO SDK include directory (was: \"${OO_SDK_INCLUDE}\"), exiting"
 exit
fi

# generate c++ headers
CPPUMAKER=${OO_SDK_HOME}/bin/cppumaker
if [ ! -x ${CPPUMAKER} ]
then
 echo "cppumaker tool not found (was: \"${CPPUMAKER}\"), exiting"
 exit
fi

TYPES_RDB=${OO_SDK_HOME}/../program/types.rdb
if [ ! -r ${TYPES_RDB} ]
 then
 echo "invalid .rdb file (was: \"${TYPES_RDB}\"), exiting"
 exit
fi
OFFAPI_TYPES_RDB=${OO_SDK_HOME}/../program/types/offapi.rdb
if [ ! -r ${OFFAPI_TYPES_RDB} ]
 then
 echo "invalid .rdb file (was: \"${OFFAPI_TYPES_RDB}\"), exiting"
 exit
fi
#OOVBAAPI_TYPES_RDB=${OO_SDK_HOME}/../program/oovbaapi.rdb
#if [ ! -r ${OOVBAAPITYPES_RDB} ]
# then
# echo "invalid .rdb file (was: \"${OOVBAAPITYPES_RDB}\"), exiting"
# exit
#fi
#SERVICES_RDB=${OO_SDK_HOME}/../program/services.rdb
#if [ ! -r ${SERVICES_RDB} ]
# then
# echo "invalid .rdb file (was: \"${SERVICES_RDB}\"), exiting"
# exit
#fi
#UNO_SERVICES_RDB=${OO_SDK_HOME}/../program/services/services.rdb
#if [ ! -r ${UNO_SERVICES_RDB} ]
# then
# echo "invalid .rdb file (was: \"${UNO_SERVICES_RDB}\"), exiting"
# exit
#fi

echo generating headers from \"${TYPES_RDB}\"...
${CPPUMAKER} -C -O ${OO_SDK_INCLUDE} ${TYPES_RDB} ${OFFAPI_TYPES_RDB}
if [ $? -ne 0 ]; then
 echo "ERROR: failed to cppumaker \"$(basename ${TYPES_RDB})\": $?, aborting"
 exit 1
fi
#echo generating headers from \"${OFFAPI_TYPES_RDB}\"...
#${CPPUMAKER} -C -O ${OO_SDK_INCLUDE} ${OFFAPI_TYPES_RDB}
#if [ $? -ne 0 ]; then
# echo "ERROR: failed to cppumaker \"$(basename ${OFFAPI_TYPES_RDB})\": $?, aborting"
# exit 1
#fi
echo generating headers...DONE
