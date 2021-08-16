#ifndef __STDAFX__
#define __STDAFX__

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined (_MSC_VER)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

//#define D3D_DEBUG_INFO

#include "targetver.h"

// Windows Header Files
#include <windows.h>

#include <strmif.h>
#include <reftime.h>
#if defined (DEBUG)
//// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#if defined (__WXDEBUG__)
//#undef __WXDEBUG__
//#endif // __WXDEBUG__
#include <wxdebug.h>
#endif // DEBUG
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H

//#include <streams.h>

// support imagemagick
#define ssize_t ssize_t
#endif // _MSC_VER

// C RunTime Header Files
//#include <sstream>
#include <string>

// System Library Header Files
#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/Log_Msg.h"

//#if defined (LIBACESTREAM_ENABLE_VALGRIND_SUPPORT)
#if defined (VALGRIND_SUPPORT)
#include "valgrind/valgrind.h"
#endif // VALGRIND_SUPPORT

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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_u_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "test_u_imagescreen_common.h"

// *TODO*: reference additional headers your program requires here

#endif // __STDAFX__
