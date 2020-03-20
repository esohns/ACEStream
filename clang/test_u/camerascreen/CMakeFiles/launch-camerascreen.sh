#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/clang/../test_u/camerascreen
export LD_LIBRARY_PATH=:$LD_LIBRARY_PATH

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_u/camerascreen/camerascreen 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_u/camerascreen/camerascreen" -l -t 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_u/camerascreen/camerascreen" -l -t 
fi
