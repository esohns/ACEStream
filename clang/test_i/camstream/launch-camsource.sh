#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/clang/../test_i/camstream
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACEStream/modules/ATCD/ACE/lib::$LD_LIBRARY_PATH

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -g./etc/source.glade -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_i/camstream/camsource 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_i/camstream/camsource" -g./etc/source.glade -l -t 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_i/camstream/camsource" -g./etc/source.glade -l -t 
fi
