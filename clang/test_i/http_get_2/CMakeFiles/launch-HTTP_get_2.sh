#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACEStream/clang/test_i/http_get_2
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACEStream/modules/ATCD/ACE/lib:/mnt/win_d/projects/ACEStream/../gtk/bin:/mnt/win_d/projects/ACEStream/../libxml2-2.9.1/.libs:/mnt/win_d/projects/ACEStream/../Common/cmake/src/Debug:/mnt/win_d/projects/ACEStream/../Common/cmake/src/ui/Debug:/mnt/win_d/projects/ACEStream/cmake/src/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/dev/Debug:/mnt/win_d/projects/ACEStream/cmake/src/modules/vis/Debug:/mnt/win_d/projects/ACEStream/../ACENetwork/cmake/src/Debug::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -c./../../../test_i/http_get_2/etc/soffice.ini -f./../../../test_i/http_get_2/etc/symbols.ini -h localhost -l -o ./output.ods -p 2083 -t -u http://kurse.boerse.ard.de/ard/kurse_einzelkurs_suche.htn " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACEStream/clang/test_i/http_get_2/HTTP_get_2 
	else
		"/mnt/win_d/projects/ACEStream/clang/test_i/http_get_2/HTTP_get_2" -c./../../../test_i/http_get_2/etc/soffice.ini -f./../../../test_i/http_get_2/etc/symbols.ini -h localhost -l -o ./output.ods -p 2083 -t -u http://kurse.boerse.ard.de/ard/kurse_einzelkurs_suche.htn 
	fi
else
	"/mnt/win_d/projects/ACEStream/clang/test_i/http_get_2/HTTP_get_2" -c./../../../test_i/http_get_2/etc/soffice.ini -f./../../../test_i/http_get_2/etc/symbols.ini -h localhost -l -o ./output.ods -p 2083 -t -u http://kurse.boerse.ard.de/ard/kurse_einzelkurs_suche.htn 
fi
