if (DEFINED ENV{LIB_ROOT})
 set (PGE_DIR "$ENV{LIB_ROOT}/olcPixelGameEngine" )
endif (DEFINED ENV{LIB_ROOT})
find_path (PGE_INCLUDE_DIR
           olcPixelGameEngine.h
           HINTS ${PGE_DIR})
if (NOT PGE_INCLUDE_DIR)
 message (FATAL_ERROR "could not find olcPixelGameEngine.h, aborting")
else ()
 message (STATUS "Found PGE header @ \"${PGE_INCLUDE_DIR}\"")
endif ()
