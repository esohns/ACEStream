if (UNIX)
# pkg_check_modules (PKG_LIBREOFFICE REQUIRED libreoffice-dev) # Ubuntu
 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_PATH})
  message (WARNING "OO_SDK_HOME not set, falling back to UNO_PATH (was: \"${UNO_PATH}\")")
  set (OO_SDK_DIRECTORY $ENV{UNO_PATH}/../sdk)
 else ()
  if (CMAKE_SYSTEM_NAME MATCHES "Linux")
   if (${LSB_RELEASE_ID_SHORT} MATCHES "Fedora")
    set (OO_SDK_DIRECTORY /usr/lib64/libreoffice/sdk)
   elseif (${LSB_RELEASE_ID_SHORT} MATCHES "Ubuntu")
    set (OO_SDK_DIRECTORY /usr/lib/libreoffice/sdk)
   else ()
    set (OO_SDK_DIRECTORY /usr/lib/libreoffice/sdk)
   endif ()
  else ()
   set (OO_SDK_DIRECTORY /usr/lib/libreoffice/sdk)
  endif (CMAKE_SYSTEM_NAME MATCHES "Linux")
  message (WARNING "OO_SDK_HOME|UNO_PATH not set, falling back to ${OO_SDK_DIRECTORY}")
 endif ()
 if (NOT EXISTS ${OO_SDK_DIRECTORY})
  message (FATAL_ERROR "OpenOffice SDK directory not found (was: \"${OO_SDK_DIRECTORY}\"), aborting")
 endif (NOT EXISTS ${OO_SDK_DIRECTORY})
 set (LIBREOFFICE_SAL_LIB_FILE libuno_sal.so)
 find_library (LIBREOFFICE_SAL_LIBRARY ${LIBREOFFICE_SAL_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_SAL_LIB_FILE}\"")
 if (NOT LIBREOFFICE_SAL_LIBRARY)
  set (LIBREOFFICE_FOUND FALSE)
 else ()
  set (LIBREOFFICE_FOUND TRUE)
 endif ()
# if (PKG_LIBREOFFICE_FOUND)
#  set (LIBREOFFICE_FOUND TRUE)
# endif (PKG_LIBREOFFICE_FOUND)
 
 if (LIBREOFFICE_FOUND)
  add_definitions (-DLIBREOFFICE_SUPPORT)
  option (LIBREOFFICE_SUPPORT "enable libreoffice support" ON)

  add_definitions (-DSAL_UNX) # libreoffice
  add_definitions (-DUNX)     # openoffice
  add_definitions (-DCPPU_ENV=gcc3)

  set (PKG_LIBREOFFICE_LIBRARIES "${LIBREOFFICE_SAL_LIBRARY};${OO_SDK_DIRECTORY}/../program/libgcc3_uno.so;${OO_SDK_DIRECTORY}/lib/libuno_cppu.so;${OO_SDK_DIRECTORY}/../program/libstorelo.so;${OO_SDK_DIRECTORY}/../program/libreglo.so;${OO_SDK_DIRECTORY}/../program/libxmlreaderlo.so;${OO_SDK_DIRECTORY}/../program/libunoidllo.so;${OO_SDK_DIRECTORY}/lib/libuno_cppuhelpergcc3.so")
 endif (LIBREOFFICE_FOUND)
elseif (WIN32)
 if (DEFINED ENV{OO_SDK_HOME})
  set (OO_SDK_DIRECTORY $ENV{OO_SDK_HOME})
 elseif (DEFINED ENV{UNO_HOME})
  message (WARNING "OO_SDK_HOME not set, falling back to UNO_HOME (was: \"${UNO_HOME}\")")
  set (OO_SDK_DIRECTORY $ENV{UNO_HOME}\..\sdk)
 else ()
  message (WARNING "%OO_SDK_HOME%/%UNO_HOME% not set, falling back")
  set (OO_SDK_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../openoffice/sdk)
 endif ()
 unset (LIBREOFFICE_SAL_LIB_FILE)
 set (LIBREOFFICE_SAL_LIB_FILE isal.lib)
 find_library (LIBREOFFICE_SAL_LIBRARY ${LIBREOFFICE_SAL_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_SAL_LIB_FILE}\"")
 if (NOT LIBREOFFICE_SAL_LIBRARY)
  set (LIBREOFFICE_FOUND FALSE)
 else ()
  set (LIBREOFFICE_FOUND TRUE)
 endif ()

 if (LIBREOFFICE_FOUND)
  add_definitions (-DLIBREOFFICE_SUPPORT)
  option (LIBREOFFICE_SUPPORT "enable libreoffice support" ON)

  add_definitions (-DCPPU_ENV=msci)
  add_definitions (-DWNT)
# *NOTE*: the OpenOffice SDK provides its own snprintf; however MSVC complains
#         about mutliple definitions (and inconsistent linkage)
#         --> disable that header and use the native function
  if (MSVC)
   add_definitions (-D_SNPRINTF_H)
  endif (MSVC)

  set (PKG_LIBREOFFICE_LIBRARIES "${LIBREOFFICE_SAL_LIBRARY};${OO_SDK_DIRECTORY}/lib/icppu.lib;${OO_SDK_DIRECTORY}/lib/icppuhelper.lib")
 endif (LIBREOFFICE_FOUND)
endif ()