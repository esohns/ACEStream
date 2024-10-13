// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#if defined (_MSC_VER)
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// *NOTE*: work around quirky MSVC...
#define NOMINMAX

#include "targetver.h"

// Windows Header Files
#include "windows.h"

//#include "dxgi.h"
//#include "dxgicommon.h"
//#include "dxgitype.h"
//#include "d3d12.h"
//#include "d3dx12.h"
#endif // _MSC_VER

#if defined (VALGRIND_USE)
#include "valgrind/valgrind.h"
#endif // VALGRIND_USE

// C RunTime Header Files
#include <string>

// System Library Header Files
#include "ace/Global_Macros.h"
#include "ace/Log_Msg.h"

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

#include "test_u_camerascreen_common.h"
