
set (IN_CPACK TRUE)

set (CPACK_COMPONENTS_ALL config include mod_include lib mod_lib test_u test_i)
set (CPACK_COMPONENT_CONFIG_DESCRIPTION "configuration data")
set (CPACK_COMPONENT_CONFIG_DISPLAY_NAME "configuration data")
set (CPACK_COMPONENT_CONFIG_HIDDEN TRUE)
#set (CPACK_COMPONENT_CONFIG_REQUIRED TRUE)
set (CPACK_COMPONENT_INCLUDE_DESCRIPTION "C++ headers")
set (CPACK_COMPONENT_INCLUDE_DISPLAY_NAME "C++ headers (library)")
set (CPACK_COMPONENT_MOD_INCLUDE_DESCRIPTION "C++ headers of the modules")
set (CPACK_COMPONENT_MOD_INCLUDE_DISPLAY_NAME "C++ headers (modules)")
set (CPACK_COMPONENT_LIB_DESCRIPTION "C++ (import) library and DLL")
set (CPACK_COMPONENT_LIB_DISPLAY_NAME "C++ (import) library and DLL")
set (CPACK_COMPONENS_MOD_LIB_DESCRIPTION "C++ (import) libraries and DLLs (modules)")
set (CPACK_COMPONENS_MOD_LIB_DISPLAY_NAME "C++ (import) libraries and DLLs (modules)")
set (CPACK_COMPONENT_TEST_I_DISPLAY_NAME "integration tests")
set (CPACK_COMPONENT_TEST_U_DISPLAY_NAME "unit tests")

#set (CPACK_COMPONENT_LIB_DEPENDS include)
#set (CPACK_COMPONENT_MOD_LIB_DEPENDS mod_include)
set (CPACK_COMPONENT_MOD_LIB_DEPENDS lib)
set (CPACK_COMPONENT_TEST_I_DEPENDS config lib mod_lib)
set (CPACK_COMPONENT_TEST_U_DEPENDS config lib mod_lib)

set (CPACK_COMPONENT_INCLUDE_GROUP "Development")
set (CPACK_COMPONENT_MOD_INCLUDE_GROUP "Development")
set (CPACK_COMPONENT_LIB_GROUP "Development")
set (CPACK_COMPONENT_MOD_LIB_GROUP "Development")
set (CPACK_COMPONENT_TEST_I_GROUP "Tests")
set (CPACK_COMPONENT_TEST_U_GROUP "Tests")

set (CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "headers and (import) libraries")
set (CPACK_COMPONENT_GROUP_TESTS_DESCRIPTION "Unit and Integration Tests")

set (CPACK_ALL_INSTALL_TYPES Full Developer)
set (CPACK_COMPONENT_INCLUDE_INSTALL_TYPES Full Developer)
set (CPACK_COMPONENT_MOD_INCLUDE_INSTALL_TYPES Full Developer)
set (CPACK_COMPONENT_LIB_INSTALL_TYPES Full Developer)
set (CPACK_COMPONENT_MOD_LIB_INSTALL_TYPES Full Developer)
set (CPACK_COMPONENT_TEST_I_INSTALL_TYPES Full)
set (CPACK_COMPONENT_TEST_U_INSTALL_TYPES Full)

#set (CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

#set (CPACK_INSTALL_COMMANDS )
set (CPACK_INSTALL_CMAKE_PROJECTS "/mnt/win_d/projects/ACEStream/cmake;ACEStream;ALL;/")

#set (CPACK_INSTALL_SCRIPT )
#set (CPACK_INSTALLED_DIRECTORIES )
#set (CPACK_MONOLITHIC_INSTALL )

#set (CPACK_OUTPUT_FILE_PEFIX )
set (CPACK_OUTPUT_CONFIG_FILE "release/CPackConfig.cmake")

set (CPACK_PACKAGE_CONTACT "eriksohns@123mail.org")
set (CPACK_PACKAGE_DESCRIPTION_FILE "/mnt/win_d/projects/ACEStream/README")
set (CPACK_PACKAGE_DESCRIPTION user-space wrapper library for the pipes-and-filters pattern, based on the ACE framework (see: http://www.cs.wustl.edu/~schmidt/ACE.html). In particular, the library lightly encapsulates the ACE_Stream and ACE_Module classes, introducing a new set of (control) interfaces to support asynchronous operation and additional concepts, such as 'session' data and messages. In conjunction with additional, modular data processing functionality (see e.g.: https://github.com/esohns/libACENetwork), this approach facilitates the separation of data processing from application-specific logic and therefore enables portable approaches to (distributed) application design)
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY (wrapper) library for streams functionality, based on the ACE framework)
#set (CPACK_PACKAGE_EXECUTABLES "camsave;camera save;\
#camsource;camera source;\
#camtarget;camera target;\
#filecopy;file copy;\
#filesource;file source;\
#filetarget;file target;\
#http_get;HTTP get;\
#riffdecoder;RIFF decoder")
if (UNIX)
#set (CPACK_PACKAGING_INSTALL_PREFIX /usr/local)
elseif (WIN32)
 set (CPACK_PACKAGE_INSTALL_DIRECTORY "$ENV{PROGRAMFILES64}ACEStream")
 set (CPACK_PACKAGE_INSTALL_REGISTRY_KEY "ACEStream")
endif ()
set (CPACK_PACKAGE_ICON "/mnt/win_d/projects/ACEStream/include/libacestream_icon.png")
set (CPACK_PACKAGE_NAME "libACEStream")
set (CPACK_PACKAGE_VENDOR "")
set (CPACK_PACKAGE_VERSION "0.0.1")
set (CPACK_PACKAGE_VERSION_MAJOR 0)
set (CPACK_PACKAGE_VERSION_MINOR 0)
set (CPACK_PACKAGE_VERSION_PATCH 1)

# File included at cpack time, once per generator after setting CPACK_GENERATOR
# to the actual generator being used; allows per-generator setting of CPACK_*
# variables at cpack time. (${PROJECT_BINARY_DIR}/CPackOptions.cmake)
#set (CPACK_PROJECT_CONFIG_FILE )

set (CPACK_RESOURCE_FILE_LICENSE "/mnt/win_d/projects/ACEStream/LICENSE")
set (CPACK_RESOURCE_FILE_README "/mnt/win_d/projects/ACEStream/README")
set (CPACK_RESOURCE_FILE_WELCOME "")

#set (CPACK_SOURCE_GENERATOR )
#set (CPACK_SOURCE_IGNORE_FILES )
set (CPACK_SOURCE_OUTPUT_CONFIG_FILE "/mnt/win_d/projects/ACEStream/release/CPackSourceConfig.cmake")
set (CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set (CPACK_SOURCE_STRIP_FILES "")

# use CMAKE_INSTALL_PREFIX ? : CPACK_PACKAGING_INSTALL_PREFIX
set (CPACK_SET_DESTDIR FALSE)

set (CPACK_STRIP_FILES "bin/camsource;bin/camtarget")

set (CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})

set (CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
#set (CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
#message ("package file name: \"${CPACK_PACKAGE_FILE_NAME}\"")

################################################################################

if (UNIX)
 set (CPACK_TOPLEVEL_TAG "Linux-i686")

 set (CPACK_CMAKE_GENERATOR "Unix Makefiles")

#set (CPACK_SOURCE_GENERATOR "TGZ;TZ")
 set (CPACK_SOURCE_GENERATOR "TGZ")

#set (CPACK_GENERATOR "STGZ;TGZ;TZ")
 set (CPACK_GENERATOR "DEB;TGZ")
#set (CPACK_GENERATOR "RPM;TGZ")

# set (CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})

 if ("${CPACK_GENERATOR}" STREQUAL "DEB")
  set (CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} Debian)

  set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
# Package dependencies (use dpkg -s <packagename> to retrieve version)
#set (CPACK_DEBIAN_PACKAGE_DEPENDS )
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})
  set (CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
  set (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
#set (CPACK_DEBIAN_PACKAGE_RECOMMENDS )
#set (CPACK_DEBIAN_PACKAGE_SUGGESTS )
  set (CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})

# This variable allow advanced user to add custom script to the control.tar.gz
# (inside the .deb archive) ${CMAKE_CURRENT_SOURCE_DIR}/postinst
#  set (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA)
# Package section (see http://packages.debian.org/stable/)
  set (CPACK_DEBIAN_PACKAGE_SECTION devel)

  set (CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})

 elseif ("${CPACK_GENERATOR}" STREQUAL "RPM")
  set (CPACK_RPM_PACKAGE_SUMMARY (wrapper) library for streams functionality, based on the ACE framework)
  set (CPACK_RPM_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
  set (CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
  set (CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
  set (CPACK_RPM_PACKAGE_RELEASE 1)
  set (CPACK_RPM_PACKAGE_LICENSE "LGPL")
# The RPM package group (see /usr/share/doc/rpm-*/GROUPS)
  set (CPACK_RPM_PACKAGE_GROUP devel)
#set (CPACK_RPM_PACKAGE_VENDOR)
  set (CPACK_RPM_PACKAGE_DESCRIPTION user-space wrapper library for the pipes-and-filters pattern, based on the ACE framework (see: http://www.cs.wustl.edu/~schmidt/ACE.html). In particular, the library lightly encapsulates the ACE_Stream and ACE_Module classes, introducing a new set of (control) interfaces to support asynchronous operation and additional concepts, such as 'session' data and messages. In conjunction with additional, modular data processing functionality (see e.g.: https://github.com/esohns/libACENetwork), this approach facilitates the separation of data processing from application-specific logic and therefore enables portable approaches to (distributed) application design)
#set (CPACK_RPM_PACKAGE_REQUIRES )
#set (CPACK_RPM_PACKAGE_PROVIDES )
#set (CPACK_RPM_SPEC_INSTALL_POST )
#set (CPACK_RPM_SPEC_MORE_DEFINE )
#set (CPACK_RPM_USER_BINARY_SPECFILE )
#set (CPACK_RPM_GENERATE_USER_BINARY_SPECFILE_TEMPLATE )
#set (CPACK_RPM_<POST/PRE>_<UN>INSTALL_SCRIPT_FILE )
  set (CPACK_RPM_PACKAGE_DEBUG 1)

 endif ()
elseif (WIN32)
 set (CPACK_CMAKE_GENERATOR "Visual Studio 14 2015")

#set (CPACK_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION OFF)

 set (CPACK_GENERATOR "NSIS")

 # There is a bug in NSI that does not handle full unix paths properly. Make
 # sure there is at least one set of four (4) backslashes.
 set (CPACK_PACKAGE_ICON "/mnt/win_d/projects/ACEStream/include\\\\libacestream_icon.ico")

#set (CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_BUILD_TYPE})
# set (CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})

#set (CPACK_NSIS_CREATE_ICONS_EXTRA )
#set (CPACK_NSIS_COMPRESSOR )
 set (CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})

#set (CPACK_NSIS_DELETE_ICONS_EXTRA )
 set (CPACK_NSIS_DISPLAY_NAME "ACEStream")

 set (CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
#set (CPACK_NSIS_EXECUTABLES_DIRECTORY )
set (CPACK_NSIS_EXTRA_INSTALL_COMMANDS "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\file copy.lnk' '$INSTDIR\\\\bin\\\\filecopy.exe' '-g.\\data\\ACEStream\\filecopy\\filecopy.glade -l -t' $INSTDIR\\\\include\\\\libacestream_icon.ico 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F1 'file copy'
                                        CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\file source.lnk' '$INSTDIR\\\\bin\\\\filesource.exe' '-g.\\data\\ACEStream\\filestream\\source.glade -l -t' $INSTDIR\\\\include\\\\libacestream_icon.ico 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F3 'filestream source'
                                        CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\file target.lnk' '$INSTDIR\\\\bin\\\\filetarget.exe' '-g.\\data\\ACEStream\\filestream\\target.glade -l -t' $INSTDIR\\\\include\\\\libacestream_icon.ico 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F4 'filestream target'
                                        CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\camera save.lnk' '$INSTDIR\\\\bin\\\\camsave.exe' '-g.\\data\\ACEStream\\camsave\\camsave.glade -l -t' $INSTDIR\\\\include\\\\libacestream_icon.ico 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F2 'camera save'
                                        CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\camera source.lnk' '$INSTDIR\\\\bin\\\\camsource.exe' '-g.\\data\\ACEStream\\camstream\\source.glade -l -t' $INSTDIR\\\\include\\\\libacestream_icon.ico 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F5 'camstream source'
                                        CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\camera target.lnk' '$INSTDIR\\\\bin\\\\camtarget.exe' '-g.\\data\\ACEStream\\camstream\\target.glade -l -t' $INSTDIR\\\\include\\\\libacestream_icon.ico 0 SW_SHOWNORMAL ALT|CONTROL|SHIFT|F6 'camstream target'")
#set (CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\VSXu Player Fullscreen.lnk\\\" \\\"$INSTDIR\\\\.\\\\vsxu_player.exe \\\" -f")
#set (CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "Delete \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\VSXu Player Fullscreen.lnk\\\")

 set (CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.github.com\\\\esohns\\\\ACEStream")

 # The default installation directory presented to the end user by the NSIS installer
 set (CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64") 
#set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\camsource.exe")
#set (CPACK_NSIS_INSTALLER_MUI_ICON_CODE )

#set (CPACK_NSIS_MENU_LINKS "doc/cmake-${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}/cmake.html" "CMake Help")
 set (CPACK_NSIS_MENU_LINKS "${CPACK_NSIS_HELP_LINK}" "ACEStream Project Web Site")
 set (CPACK_NSIS_MODIFY_PATH ON)
 set (CPACK_NSIS_MUI_ICON ${CPACK_PACKAGE_ICON})
#set (CPACK_NSIS_MUI_FINISHPAGE_RUN )
 set (CPACK_NSIS_MUI_UNIICON ${CPACK_PACKAGE_ICON})

 set (CPACK_NSIS_PAGE_COMPONENTS " ")

 # Title displayed at the top of the installer
 set (CPACK_NSIS_PACKAGE_NAME "ACEStream 0.0")

 set (CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.github.com\\\\esohns\\\\ACEStream")
endif ()
