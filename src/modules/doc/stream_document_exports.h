
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl -d Stream_Document
// ------------------------------
#ifndef STREAM_DOCUMENT_EXPORT_H
#define STREAM_DOCUMENT_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (STREAM_DOCUMENT_HAS_DLL)
#  define STREAM_DOCUMENT_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && STREAM_DOCUMENT_HAS_DLL */

#if !defined (STREAM_DOCUMENT_HAS_DLL)
#  define STREAM_DOCUMENT_HAS_DLL 1
#endif /* ! STREAM_DOCUMENT_HAS_DLL */

#if defined (STREAM_DOCUMENT_HAS_DLL) && (STREAM_DOCUMENT_HAS_DLL == 1)
#  if defined (STREAM_DOCUMENT_BUILD_DLL)
#    define Stream_Document_Export ACE_Proper_Export_Flag
#    define STREAM_DOCUMENT_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define STREAM_DOCUMENT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* STREAM_DOCUMENT_BUILD_DLL */
#    define Stream_Document_Export ACE_Proper_Import_Flag
#    define STREAM_DOCUMENT_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define STREAM_DOCUMENT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* STREAM_DOCUMENT_BUILD_DLL */
#else /* STREAM_DOCUMENT_HAS_DLL == 1 */
#  define Stream_Document_Export
#  define STREAM_DOCUMENT_SINGLETON_DECLARATION(T)
#  define STREAM_DOCUMENT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* STREAM_DOCUMENT_HAS_DLL == 1 */

// Set STREAM_DOCUMENT_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (STREAM_DOCUMENT_NTRACE)
#  if (ACE_NTRACE == 1)
#    define STREAM_DOCUMENT_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define STREAM_DOCUMENT_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !STREAM_DOCUMENT_NTRACE */

#if (STREAM_DOCUMENT_NTRACE == 1)
#  define STREAM_DOCUMENT_TRACE(X)
#else /* (STREAM_DOCUMENT_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define STREAM_DOCUMENT_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (STREAM_DOCUMENT_NTRACE == 1) */

#endif /* STREAM_DOCUMENT_EXPORT_H */

// End of auto generated file.
