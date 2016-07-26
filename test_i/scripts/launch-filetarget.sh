#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/libACEStream
export LD_LIBRARY_PATH=/mnt/win_d/projects/libACEStream/modules/ATCD/ACE/lib:/mnt/win_d/projects/libACEStream/../gtk/bin:/mnt/win_d/projects/libACEStream/../libxml2-2.9.1/.libs:/mnt/win_d/projects/libACEStream/../libCommon/cmake/src/Debug:/mnt/win_d/projects/libACEStream/../libCommon/cmake/src/ui/Debug:/mnt/win_d/projects/libACEStream/cmake/src/Debug:/mnt/win_d/projects/libACEStream/cmake/src/modules/dev/Debug:/mnt/win_d/projects/libACEStream/../libACENetwork/cmake/src/Debug::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "x" = "xYES"; then
		echo "r -g./test_i/filestream/etc/target.glade -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		 -batch -command=$bindir/gdbscript  /mnt/win_d/projects/libACEStream/cmake/test_i/filestream/filetarget 
	else
		"/mnt/win_d/projects/libACEStream/cmake/test_i/filestream/filetarget" -g./test_i/filestream/etc/target.glade -l -t 
	fi
else
	"/mnt/win_d/projects/libACEStream/cmake/test_i/filestream/filetarget" -g./test_i/filestream/etc/target.glade -l -t 
fi
