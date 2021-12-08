/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "stdafx.h"

#include "stream_lib_mediafoundation_null.h"

#include "Shlwapi.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Stream_MediaFramework_MediaFoundation_Null::Stream_MediaFramework_MediaFoundation_Null ()
 : referenceCount_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::Stream_MediaFramework_MediaFoundation_Null"));

}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::QueryInterface (const IID& IID_in,
                                                            void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    QITABENT (Stream_MediaFramework_MediaFoundation_Null, IMFSampleGrabberSinkCallback2),
#else
    QITABENT (Stream_MediaFramework_MediaFoundation_Null, IMFSampleGrabberSinkCallback),
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}

ULONG
Stream_MediaFramework_MediaFoundation_Null::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0)
  //  delete this;

  return count;
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnClockStart (MFTIME systemClockTime_in,
                                                          LONGLONG clockStartOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnClockStart"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (clockStartOffset_in);

  return S_OK;
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnClockStop"));

  ACE_UNUSED_ARG (systemClockTime_in);

  return S_OK;
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnClockPause (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnClockPause"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnClockRestart (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnClockRestart"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnClockSetRate (MFTIME systemClockTime_in,
                                                            float playbackRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnClockSetRate"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (playbackRate_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnProcessSample (REFGUID majorMediaType_in,
                                                             DWORD flags_in,
                                                             LONGLONG timeStamp_in,
                                                             LONGLONG duration_in,
                                                             const BYTE* buffer_in,
                                                             DWORD bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnProcessSample"));

  IMFAttributes* attributes_p = NULL;
  return OnProcessSampleEx (majorMediaType_in,
                            flags_in,
                            timeStamp_in,
                            duration_in,
                            buffer_in,
                            bufferSize_in,
                            attributes_p);
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnProcessSampleEx (REFGUID majorMediaType_in,
                                                               DWORD flags_in,
                                                               LONGLONG timeStamp_in,
                                                               LONGLONG duration_in,
                                                               const BYTE* buffer_in,
                                                               DWORD bufferSize_in,
                                                               IMFAttributes* attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnProcessSampleEx"));

  ACE_UNUSED_ARG (majorMediaType_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (timeStamp_in);
  ACE_UNUSED_ARG (duration_in);
  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferSize_in);
  ACE_UNUSED_ARG (attributes_in);

  return S_OK;
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnSetPresentationClock (IMFPresentationClock* presentationClock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnSetPresentationClock"));

  ACE_UNUSED_ARG (presentationClock_in);

  return S_OK;
}

HRESULT
Stream_MediaFramework_MediaFoundation_Null::OnShutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Null::OnShutdown"));

  return S_OK;
}
