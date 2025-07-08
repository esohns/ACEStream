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
#endif // _MSC_VER

// C RunTime Header Files
#include <string>

// 3rd-Party Library Header Files
#if defined (VALGRIND_USE)
#include "valgrind/valgrind.h"
#endif // VALGRIND_USE

#if defined (_MSC_VER)
#define __LCC__
#define __WIN__
#undef my_socket_defined
#define uint unsigned int
#define ulong unsigned long
#if defined (MYSQL_SUPPORT)
#include "mysql.h"
#endif // MYSQL_SUPPORT
#else
#if defined (MYSQL_SUPPORT)
#include "mysql/mysql.h"
#endif // MYSQL_SUPPORT
#endif // _MSC_VER

// System Library Header Files
#include "ace/config-lite.h"
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

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common.h"
#include "net_macros.h"

#include "test_i_common.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT

#include "test_i_http_get_common.h"
