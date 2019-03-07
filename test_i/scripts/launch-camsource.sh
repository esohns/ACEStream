#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/libACEStream
export LD_LIBRARY_PATH=D;/projects/ATCD/ACE/lib:D;/projects/gtk/bin:D;/projects/libglade/bin:D;/projects/libxml2-2.9.1/.libs:G;/software/Development/dll:/mnt/win_d/projects/libACEStream/../libCommon/cmake/src/Debug:/mnt/win_d/projects/libACEStream/../libCommon/cmake/src/ui/Debug:/mnt/win_d/projects/libACEStream/cmake/src/Debug:/mnt/win_d/projects/libACEStream/cmake/src/modules/dev/Debug:/mnt/win_d/projects/libACEStream/../libACENetwork/cmake/src/Debug::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "x" = "xYES"; then
		echo "r -g.\test_i\camstream\etc\source.glade -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		 -batch -command=$bindir/gdbscript  /mnt/win_d/projects/libACEStream/cmake/test_i/camstream/camsource 
	else
		"/mnt/win_d/projects/libACEStream/cmake/test_i/camstream/camsource" -g.\test_i\camstream\etc\source.glade -l -t 
	fi
else
	"/mnt/win_d/projects/libACEStream/cmake/test_i/camstream/camsource" -g.\test_i\camstream\etc\source.glade -l -t 
fi
