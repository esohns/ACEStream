#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/clang/../test_i/camstream
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACEStream/modules/ATCD/ACE/lib:/mnt/win_d/projects/ACEStream/../gtk/gtk/Win32/debug/bin:/mnt/win_d/projects/ACEStream/../libxml2-2.9.1/.libs:/mnt/win_d/projects/ACEStream/../Common/cmake/src/Debug:/mnt/win_d/projects/ACEStream/../Common/cmake/src/ui/Debug:/mnt/win_d/projects/ACEStream/cmake/src/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/dev/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/vis/Debug:/mnt/win_d/projects/ACEStream/../ACENetwork/cmake/src/Debug::$LD_LIBRARY_PATH

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -g./etc/target.glade -l -o -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_i/camstream/camtarget 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_i/camstream/camtarget" -g./etc/target.glade -l -o -t 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_i/camstream/camtarget" -g./etc/target.glade -l -o -t 
fi
