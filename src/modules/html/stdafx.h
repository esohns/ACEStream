// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once
#ifndef STDAFX_H
#define STDAFX_H

#if defined _MSC_VER
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

// Windows Header Files
#include <windows.h>
#endif

// C RunTime Header Files
//#include <sstream>
#include <string>

// System Library Header Files
#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/Log_Msg.h"

// Local Header Files
#include "common.h"
#include "common_macros.h"

#include "stream_common.h"
#include "stream_macros.h"

#include "stream_html_exports.h"

//#if defined _MSC_VER
//#include "targetver.h"
//#endif

// *TODO*: reference additional headers your program requires here

#endif
