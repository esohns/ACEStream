set (DEBUG_DEBUGGER TRUE
     CACHE BOOL "in debugger")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
 add_definitions (-D_DEBUG)
 if (DEBUG_DEBUGGER)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
endif ()

set (${PROJECT_NAME}_Test_U_SRCS
     test_u_common.h
     test_u_gtk_common.h
     test_u_stream_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_U_SRCS
#         DESTINATION include)

add_subdirectory (audioeffect)
add_subdirectory (camsave)
add_subdirectory (filecopy)
add_subdirectory (riffdecoder)
