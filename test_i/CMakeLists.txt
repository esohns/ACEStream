option (BUILD_TEST_I "build test_i" OFF)

set (${PROJECT_NAME}_Test_I_SRCS
     test_i_configuration.h
     test_i_connection_common.h
     test_i_common.h
		 test_i_defines.h
     test_i_gtk_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_I_SRCS
#         DESTINATION include)

add_subdirectory (filestream)
add_subdirectory (camstream)
add_subdirectory (http_get)
add_subdirectory (http_get_2)

add_custom_target (test_i ALL
                   COMMENT "building test_i..."
                   SOURCES ${ACEStream_Test_I_SRCS})
add_dependencies (test_i filesource filetarget camsource camtarget HTTP_get HTTP_get_2)

cmake_policy (SET CMP0012 NEW)
if (NOT BUILD_TEST_I)
 set_target_properties (test_i PROPERTIES EXCLUDE_FROM_ALL 1)
 if (WIN32)
  set_target_properties (test_i PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
endif ()
