set (${PROJECT_NAME}_Test_U_SRCS
     test_u_common.h
     test_u_common_modules.h
     test_u_defines.h
     test_u_gtk_common.h)
#install (FILES
#         ${PROJECT_NAME}_Test_U_SRCS
#         DESTINATION include)

add_subdirectory (audioeffect)
add_subdirectory (camsave)
add_subdirectory (filecopy)
add_subdirectory (httpget)
add_subdirectory (riffdecoder)

add_custom_target (test_u
                   COMMAND echo building test_u...)
add_dependencies (test_u audioeffect camsave filecopy httpget riffdecoder)

cmake_policy (SET CMP0012 NEW)
if (NOT ${BUILD_TEST_U})
 set_target_properties (test_u PROPERTIES EXCLUDE_FROM_ALL 1)
 if (WIN32)
  set_target_properties (test_u PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
 endif ()
endif ()
