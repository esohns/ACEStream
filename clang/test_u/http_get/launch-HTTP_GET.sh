#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/clang/../test_u/http_get
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACEStream/modules/ATCD/ACE/lib::$LD_LIBRARY_PATH

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -l -t -u http://www.mirc.com/servers.ini " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_u/http_get/HTTP_GET 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_u/http_get/HTTP_GET" -l -t -u http://www.mirc.com/servers.ini 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_u/http_get/HTTP_GET" -l -t -u http://www.mirc.com/servers.ini 
fi
