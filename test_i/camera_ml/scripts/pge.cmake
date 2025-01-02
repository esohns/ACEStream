if (DEFINED ENV{LIB_ROOT})
 set (CGE_DIR "$ENV{LIB_ROOT}/olcConsoleGameEngine" )
endif (DEFINED ENV{LIB_ROOT})
find_path (CGE_INCLUDE_DIR
           olcConsoleGameEngine.h
           HINTS ${CGE_DIR}
           PATH_SUFFIXES "ConsoleGameEngine")
if (NOT CGE_INCLUDE_DIR)
 message (WARNING "could not find olcConsoleGameEngine.h, continuing")
else ()
 message (STATUS "Found CGE header @ \"${CGE_INCLUDE_DIR}\"")
 set (OLC_CGE_FOUND TRUE)
endif (NOT CGE_INCLUDE_DIR)

if (DEFINED ENV{LIB_ROOT})
 set (PGE_DIR "$ENV{LIB_ROOT}/olcPixelGameEngine" )
endif (DEFINED ENV{LIB_ROOT})
find_path (PGE_INCLUDE_DIR
           olcPixelGameEngine.h
           HINTS ${PGE_DIR})
if (NOT PGE_INCLUDE_DIR)
 message (WARNING "could not find olcPixelGameEngine.h, continuing")
else ()
 message (STATUS "Found PGE header @ \"${PGE_INCLUDE_DIR}\"")
 set (OLC_PGE_FOUND TRUE)
endif (NOT PGE_INCLUDE_DIR)

if (OLC_CGE_FOUND)
 option (OLC_CGE_SUPPORT "enable OLC Console Game Engine support" ON)
 if (OLC_CGE_SUPPORT)
  add_definitions (-DOLC_CGE_SUPPORT)
 endif (OLC_CGE_SUPPORT)
endif (OLC_CGE_FOUND)
if (OLC_PGE_FOUND)
 option (OLC_PGE_SUPPORT "enable OLC Pixel Game Engine support" ON)
 if (OLC_PGE_SUPPORT)
  add_definitions (-DOLC_PGE_SUPPORT)
 endif (OLC_PGE_SUPPORT)
endif (OLC_PGE_FOUND)
