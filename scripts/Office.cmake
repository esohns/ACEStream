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
  message (WARNING "OpenOffice SDK directory not found (was: \"${OO_SDK_DIRECTORY}\"), continuing")
 endif (NOT EXISTS ${OO_SDK_DIRECTORY})
 set (LIBREOFFICE_SAL_LIB_FILE libuno_sal.so)
 find_library (LIBREOFFICE_SAL_LIBRARY ${LIBREOFFICE_SAL_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_SAL_LIB_FILE}\"")
 set (LIBREOFFICE_GCC_UNO_LIB_FILE libgcc3_uno.so)
 find_library (LIBREOFFICE_GCC_UNO_LIBRARY ${LIBREOFFICE_GCC_UNO_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}/..
               PATH_SUFFIXES program
               DOC "searching for \"${LIBREOFFICE_GCC_UNO_LIB_FILE}\"")
 set (LIBREOFFICE_UNO_CPPU_LIB_FILE libuno_cppu.so)
 find_library (LIBREOFFICE_UNO_CPPU_LIBRARY ${LIBREOFFICE_UNO_CPPU_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_UNO_CPPU_LIB_FILE}\"")
 set (LIBREOFFICE_STOREIO_LIB_FILE libstorelo.so)
 find_library (LIBREOFFICE_STOREIO_LIBRARY ${LIBREOFFICE_STOREIO_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}/..
               PATH_SUFFIXES program
               DOC "searching for \"${LIBREOFFICE_STOREIO_LIB_FILE}\"")
 set (LIBREOFFICE_REGIO_LIB_FILE libreglo.so)
 find_library (LIBREOFFICE_REGIO_LIBRARY ${LIBREOFFICE_REGIO_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}/..
               PATH_SUFFIXES program
               DOC "searching for \"${LIBREOFFICE_REGIO_LIB_FILE}\"")
 set (LIBREOFFICE_XMLREADERIO_LIB_FILE libxmlreaderlo.so)
 find_library (LIBREOFFICE_XMLREADERIO_LIBRARY ${LIBREOFFICE_XMLREADERIO_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}/..
               PATH_SUFFIXES program
               DOC "searching for \"${LIBREOFFICE_XMLREADERIO_LIB_FILE}\"")
 set (LIBREOFFICE_UNOIDLLO_LIB_FILE libunoidllo.so)
 find_library (LIBREOFFICE_UNOIDLLO_LIBRARY ${LIBREOFFICE_UNOIDLLO_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}/..
               PATH_SUFFIXES program
               DOC "searching for \"${LIBREOFFICE_UNOIDLLO_LIB_FILE}\"")
 set (LIBREOFFICE_CPPUHELPERGCC_LIB_FILE libuno_cppuhelpergcc3.so)
 find_library (LIBREOFFICE_CPPUHELPERGCC_LIBRARY ${LIBREOFFICE_CPPUHELPERGCC_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_CPPUHELPERGCC_LIB_FILE}\"")
 if (LIBREOFFICE_SAL_LIBRARY AND LIBREOFFICE_GCC_UNO_LIBRARY AND LIBREOFFICE_UNO_CPPU_LIBRARY AND LIBREOFFICE_STOREIO_LIBRARY AND LIBREOFFICE_REGIO_LIBRARY AND LIBREOFFICE_XMLREADERIO_LIBRARY AND LIBREOFFICE_UNOIDLLO_LIBRARY AND LIBREOFFICE_CPPUHELPERGCC_LIBRARY)
  set (LIBREOFFICE_FOUND TRUE)
  set (LIBREOFFICE_INCLUDE_DIRS "${OO_SDK_DIRECTORY}/include")
  set (LIBREOFFICE_LIBRARIES "${LIBREOFFICE_SAL_LIBRARY};${LIBREOFFICE_GCC_UNO_LIBRARY};${LIBREOFFICE_UNO_CPPU_LIBRARY};${LIBREOFFICE_STOREIO_LIBRARY};${LIBREOFFICE_REGIO_LIBRARY};${LIBREOFFICE_XMLREADERIO_LIBRARY};${LIBREOFFICE_UNOIDLLO_LIBRARY};§{LIBREOFFICE_CPPUHELPERGCC_LIBRARY}")
 endif (LIBREOFFICE_SAL_LIBRARY AND LIBREOFFICE_GCC_UNO_LIBRARY AND LIBREOFFICE_UNO_CPPU_LIBRARY AND LIBREOFFICE_STOREIO_LIBRARY AND LIBREOFFICE_REGIO_LIBRARY AND LIBREOFFICE_XMLREADERIO_LIBRARY AND LIBREOFFICE_UNOIDLLO_LIBRARY AND LIBREOFFICE_CPPUHELPERGCC_LIBRARY)
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
 set (LIBREOFFICE_SAL_LIB_FILE isal.lib)
 find_library (LIBREOFFICE_SAL_LIBRARY ${LIBREOFFICE_SAL_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_SAL_LIB_FILE}\"")
 set (LIBREOFFICE_ICPPU_LIB_FILE icppu.lib)
 find_library (LIBREOFFICE_ICPPU_LIBRARY ${LIBREOFFICE_ICPPU_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_ICPPU_LIB_FILE}\"")
 set (LIBREOFFICE_ICPPUHELPER_LIB_FILE icppuhelper.lib)
 find_library (LIBREOFFICE_ICPPUHELPER_LIBRARY ${LIBREOFFICE_ICPPUHELPER_LIB_FILE}
               PATHS ${OO_SDK_DIRECTORY}
               PATH_SUFFIXES lib
               DOC "searching for \"${LIBREOFFICE_ICPPUHELPER_LIB_FILE}\"")
 if (LIBREOFFICE_SAL_LIBRARY AND LIBREOFFICE_ICPPU_LIBRARY AND LIBREOFFICE_ICPPUHELPER_LIBRARY)
  set (LIBREOFFICE_FOUND TRUE)
  set (LIBREOFFICE_INCLUDE_DIRS "${OO_SDK_DIRECTORY}/include")
  set (LIBREOFFICE_LIBRARIES "${LIBREOFFICE_SAL_LIBRARY};${LIBREOFFICE_ICPPU_LIBRARY};${LIBREOFFICE_ICPPUHELPER_LIBRARY}")
  set (LIBREOFFICE_LIB_DIR "${OO_SDK_DIRECTORY}/bin")
 endif (LIBREOFFICE_SAL_LIBRARY AND LIBREOFFICE_ICPPU_LIBRARY AND LIBREOFFICE_ICPPUHELPER_LIBRARY)
endif ()
if (LIBREOFFICE_FOUND)
 option (LIBREOFFICE_SUPPORT "enable libreoffice support" ON)
 if (LIBREOFFICE_SUPPORT)
  add_definitions (-DLIBREOFFICE_SUPPORT)
  if (UNIX)
   add_definitions (-DSAL_UNX) # libreoffice
   add_definitions (-DUNX)     # openoffice
   add_definitions (-DCPPU_ENV=gcc3)
  elseif (WIN32)
   add_definitions (-DCPPU_ENV=msci)
   add_definitions (-DWNT)
# *NOTE*: the OpenOffice SDK provides its own snprintf; however MSVC complains
#         about mutliple definitions (and inconsistent linkage)
#         --> disable that header and use the native function
   if (MSVC)
    add_definitions (-D_SNPRINTF_H)
   endif (MSVC)
  endif ()
 endif (LIBREOFFICE_SUPPORT)
endif (LIBREOFFICE_FOUND)
