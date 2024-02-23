if (DEFINED ENV{LIB_ROOT})
 set (VORONOI_DIR "$ENV{LIB_ROOT}/voronoi")
endif (DEFINED ENV{LIB_ROOT})
find_path (VORONOI_INCLUDE_DIR
           jc_voronoi.h
           HINTS ${VORONOI_DIR}/src)
if (NOT VORONOI_INCLUDE_DIR)
 message (WARNING "could not find jc_voronoi.h, continuing")
else ()
 message (STATUS "Found voronoi header @ \"${VORONOI_INCLUDE_DIR}\"")
 set (JC_VORONOI_FOUND TRUE)
endif (NOT VORONOI_INCLUDE_DIR)

if (JC_VORONOI_FOUND)
 option (JC_VORONOI_SUPPORT "enable JC Voronoi support" ON)
 if (JC_VORONOI_SUPPORT)
  add_definitions (-DJC_VORONOI_SUPPORT)
 endif (JC_VORONOI_SUPPORT)
endif (JC_VORONOI_FOUND)
