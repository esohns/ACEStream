#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/clang/../test_u/audioeffect
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACEStream/modules/ATCD/ACE/lib::$LD_LIBRARY_PATH
export GDK_BACKEND=x11

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -g./etc/audioeffect.gtk2 -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_u/audioeffect/audioeffect 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_u/audioeffect/audioeffect" -g./etc/audioeffect.gtk2 -l -t 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_u/audioeffect/audioeffect" -g./etc/audioeffect.gtk2 -l -t 
fi
