set (DEBUG_DEBUGGER TRUE
     CACHE BOOL "in debugger")
if (DEBUG_DEBUGGER)
 add_definitions (-DDEBUG_DEBUGGER)
endif ()

add_subdirectory (camstream)
add_subdirectory (filestream)
add_subdirectory (http_get)
add_subdirectory (http_get_2)
