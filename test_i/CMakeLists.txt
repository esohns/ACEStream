set (DEBUG_DEBUGGER TRUE
     CACHE BOOL "in debugger")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
 add_definitions (-D_DEBUG)
 if (DEBUG_DEBUGGER)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
endif ()

set (${PROJECT_NAME}_Test_I_SRCS
     test_u_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_I_SRCS
#         DESTINATION include)

add_subdirectory (camstream)
add_subdirectory (filestream)
add_subdirectory (http_get)
add_subdirectory (http_get_2)
