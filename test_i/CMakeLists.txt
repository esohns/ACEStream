set (DEBUG_DEBUGGER TRUE
     CACHE BOOL "in debugger")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
 add_definitions (-D_DEBUG)
 if (DEBUG_DEBUGGER)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
endif ()

set (${PROJECT_NAME}_Test_I_SRCS
     test_i_configuration.h
     test_i_connection_common.h
     test_i_common.h
     test_i_gtk_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_I_SRCS
#         DESTINATION include)

add_subdirectory (camstream)
add_subdirectory (filestream)
add_subdirectory (http_get)
add_subdirectory (http_get_2)

add_custom_target (test_i
                   COMMAND echo building test_i...)
add_dependencies (test_i filesource filetarget camsource camtarget http_get http_get_2)
