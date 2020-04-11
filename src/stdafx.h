// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined _MSC_VER
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// *NOTE*: MSVC2013 conflicts std::min/std::max in minwindef.h To
//         work around that, define NOMINMAX
#define NOMINMAX

// Windows Header Files
#include <windows.h>

#include <Guiddef.h>
#endif

// C RunTime Header Files
//#include <sstream>
#include <string>

// System Library Header Files
#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/Log_Msg.h"

#if defined (VALGRIND_SUPPORT)
#include "valgrind/valgrind.h"
#endif // VALGRIND_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H
#include "common.h"
#include "common_macros.h"
#include "common_pragmas.h"

// Local Header Files
#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H
#include "stream_common.h"
#include "stream_macros.h"
