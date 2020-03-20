#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/test_i/http_get
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACEStream/modules/ATCD/ACE/lib:/mnt/win_d/projects/ACEStream/../libxml2-2.9.4/.libs:/mnt/win_d/projects/ACEStream/../Common/cmake/src/Debug:/mnt/win_d/projects/ACEStream/cmake/src/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/db/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/html/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/net/protocols/http/Debug:/mnt/win_d/projects/ACEStream/../ACENetwork/cmake/src/Debug:/mnt/win_d/projects/ACEStream/../ACENetwork/cmake/src/protocol/http/Debug::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -c.\..\..\..\test_i\http_get\etc\options.cnf -d news -e articles -f output.txt -l -p 80 -t -u http://www.heise.de/newsticker.html " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_i/http_get/HTTP_get_1 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_i/http_get/HTTP_get_1" -c.\..\..\..\test_i\http_get\etc\options.cnf -d news -e articles -f output.txt -l -p 80 -t -u http://www.heise.de/newsticker.html 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_i/http_get/HTTP_get_1" -c.\..\..\..\test_i\http_get\etc\options.cnf -d news -e articles -f output.txt -l -p 80 -t -u http://www.heise.de/newsticker.html 
fi
