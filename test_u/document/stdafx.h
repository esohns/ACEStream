// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined (_MSC_VER)
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// <limits> does not work correctly with these macros
#define NOMINMAX

#include "targetver.h"

// Windows Header Files
#include "windows.h"

#if defined (_DEBUG)
// *TODO*: currently, these do not work well with ACE...
//#define _CRTDBG_MAP_ALLOC
#include "stdlib.h"
#include "crtdbg.h"
#endif // _DEBUG

// *NOTE*: nmake complains (see also:
//         C:\Program Files (x86)\Windows Kits\8.1\include\shared\sspi.h(64))
#define SECURITY_WIN32
#endif // _MSC_VER

// C++ RunTime Header Files
#include <string>

// System Library Header Files
#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/Log_Msg.h"

#if defined (VALGRIND_USE)
#include "valgrind/valgrind.h"
#endif // VALGRIND_USE

// Local Header Files
#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common.h"
#include "common_macros.h"
#include "common_pragmas.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_common.h"
#include "stream_macros.h"

#include "test_u_common.h"
#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT
