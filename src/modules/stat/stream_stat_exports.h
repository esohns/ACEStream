
// -*- C++ -*-
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -d Stream_Stat
// ------------------------------
#ifndef STREAM_STAT_EXPORT_H
#define STREAM_STAT_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (STREAM_STAT_HAS_DLL)
#  define STREAM_STAT_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && STREAM_STAT_HAS_DLL */

#if !defined (STREAM_STAT_HAS_DLL)
#  define STREAM_STAT_HAS_DLL 1
#endif /* ! STREAM_STAT_HAS_DLL */

#if defined (STREAM_STAT_HAS_DLL) && (STREAM_STAT_HAS_DLL == 1)
#  if defined (STREAM_STAT_BUILD_DLL)
#    define Stream_Stat_Export ACE_Proper_Export_Flag
#    define STREAM_STAT_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define STREAM_STAT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* STREAM_STAT_BUILD_DLL */
#    define Stream_Stat_Export ACE_Proper_Import_Flag
#    define STREAM_STAT_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define STREAM_STAT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* STREAM_STAT_BUILD_DLL */
#else /* STREAM_STAT_HAS_DLL == 1 */
#  define Stream_Stat_Export
#  define STREAM_STAT_SINGLETON_DECLARATION(T)
#  define STREAM_STAT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* STREAM_STAT_HAS_DLL == 1 */

// Set STREAM_STAT_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (STREAM_STAT_NTRACE)
#  if (ACE_NTRACE == 1)
#    define STREAM_STAT_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define STREAM_STAT_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !STREAM_STAT_NTRACE */

#if (STREAM_STAT_NTRACE == 1)
#  define STREAM_STAT_TRACE(X)
#else /* (STREAM_STAT_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define STREAM_STAT_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (STREAM_STAT_NTRACE == 1) */

#endif /* STREAM_STAT_EXPORT_H */

// End of auto generated file.
