// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined (_MSC_VER)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

#include "targetver.h"

// Windows Header Files
#include <windows.h>

#include <strmif.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
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
#endif

// Local Header Files
#include "common.h"
#include "common_macros.h"
#include "common_pragmas.h"

#include "stream_common.h"
#include "stream_macros.h"

#include "stream_dev_common.h"
