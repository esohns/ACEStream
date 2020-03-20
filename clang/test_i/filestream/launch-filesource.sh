#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/clang/../test_i/filestream
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACEStream/modules/ATCD/ACE/lib:/mnt/win_d/projects/ACEStream/../Common/cmake/src/Debug:/mnt/win_d/projects/ACEStream/../Common/cmake/src/ui/Debug:/mnt/win_d/projects/ACEStream/cmake/src/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/dev/Debug:/mnt/win_d/projects/ACEStream/../libACENetwork/cmake/src/Debug::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -f ~/Downloads/keyvalue-0.3.tgz -g -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_i/filestream/filesource 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_i/filestream/filesource" -f ~/Downloads/keyvalue-0.3.tgz -g -l -t 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_i/filestream/filesource" -f ~/Downloads/keyvalue-0.3.tgz -g -l -t 
fi
