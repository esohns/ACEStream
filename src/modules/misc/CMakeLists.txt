if (UNIX)
# include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/libCommon/src/math)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/math)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../libCommon/src/ui)
endif ()
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dec)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dev)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../file)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../3rd_party/bison)
#include_directories (${CMAKE_CURRENT_BINARY_DIR}/../../../)

############## next target ################
set (${PROJECT_NAME}_Miscellaneous_LIB_SRCS
     stdafx.cpp
     stdafx.h
     stream_misc_aggregator.cpp
     stream_misc_aggregator.h
     stream_misc_aggregator.inl
     stream_misc_common.h
     stream_misc_defines.h
     stream_misc_dump.cpp
     stream_misc_dump.h
     stream_misc_dump.inl
     stream_misc_exports.h
     stream_misc_messagehandler.cpp
     stream_misc_messagehandler.h
     stream_misc_messagehandler.inl
     stream_misc_parser.cpp
     stream_misc_parser.h
     stream_misc_parser.inl
     stream_misc_queue_source.cpp
     stream_misc_queue_source.h
     stream_misc_queue_source.inl
     stream_misc_splitter.cpp
     stream_misc_splitter.h
     stream_misc_splitter.inl)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
 add_compile_options (-std=c++11 -fPIC)
endif ()

add_library (${PROJECT_NAME}_Miscellaneous SHARED
             ${ACEStream_Miscellaneous_LIB_SRCS})
set_property (TARGET ${PROJECT_NAME}_Miscellaneous PROPERTY FOLDER "modules")

add_dependencies (${PROJECT_NAME}_Miscellaneous Common)
add_dependencies (${PROJECT_NAME}_Miscellaneous ${PROJECT_NAME})

if (WIN32)
 if (CMAKE_BUILD_TYPE STREQUAL Debug)
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous ${DIRECTSHOW_BASECLASSES_DIRECTORY}/Debug/strmbasd.lib)
 else ()
#  link_directories (${DIRECTSHOW_BASECLASSES_DIRECTORY}/Release)
#  target_link_libraries (${PROJECT_NAME}_Miscellaneous strmbase.lib)
 endif ()
# target_link_libraries (${PROJECT_NAME}_Miscellaneous winmm.lib)
 target_link_libraries (${PROJECT_NAME}_Miscellaneous ${ACE_LIBRARY})
endif ()

set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES DEFINE_SYMBOL STREAM_MISC_BUILD_DLL)
set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO} SOVERSION 1)

# add PCH
set_target_properties (${PROJECT_NAME}_Miscellaneous PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire (${PROJECT_NAME}_Miscellaneous)

# add all targets to the build-tree export set
export (TARGETS ${PROJECT_NAME}_Miscellaneous
        APPEND FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

############### install files ################
install (FILES
         stream_misc_aggregator.h
         stream_misc_aggregator.inl
         stream_misc_common.h
         stream_misc_defines.h
         stream_misc_dump.h
         stream_misc_dump.inl
         stream_misc_exports.h
         stream_misc_messagehandler.h
         stream_misc_messagehandler.inl
         stream_misc_parser.h
         stream_misc_parser.inl
         stream_misc_queue_source.h
         stream_misc_queue_source.inl
         stream_misc_splitter.h
         stream_misc_splitter.inl
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT mod_include)

install (TARGETS ${PROJECT_NAME}_Miscellaneous
         EXPORT ${PROJECT_NAME}Targets
         ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT mod_lib
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT mod_lib)
