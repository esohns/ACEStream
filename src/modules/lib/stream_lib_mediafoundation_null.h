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

#ifndef STREAM_LIB_MEDIAFOUNDATION_NULL_H
#define STREAM_LIB_MEDIAFOUNDATION_NULL_H

#include "mfidl.h"

#include "ace/Global_Macros.h"

#include "common_macros.h"

class Stream_MediaFramework_MediaFoundation_Null
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
 : public IMFSampleGrabberSinkCallback2
#else
 : public IMFSampleGrabberSinkCallback
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
{
 public:
  Stream_MediaFramework_MediaFoundation_Null ();
  inline virtual ~Stream_MediaFramework_MediaFoundation_Null () {}

  // implement IMFSampleGrabberSinkCallback2
  // IUnknown
  STDMETHODIMP QueryInterface (const IID&,
                               void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&referenceCount_); }
  virtual STDMETHODIMP_ (ULONG) Release ();
  // IMFClockStateSink
  STDMETHODIMP OnClockStart (MFTIME,    // (system) clock start time
                             LONGLONG); // clock start offset
  STDMETHODIMP OnClockStop (MFTIME); // (system) clock start time
  STDMETHODIMP OnClockPause (MFTIME); // (system) clock pause time
  STDMETHODIMP OnClockRestart (MFTIME); // (system) clock restart time
  STDMETHODIMP OnClockSetRate (MFTIME, // (system) clock rate set time
                               float); // new playback rate
  // IMFSampleGrabberSinkCallback
  STDMETHODIMP OnSetPresentationClock (IMFPresentationClock*); // presentation clock handle
  STDMETHODIMP OnProcessSample (REFGUID,     // major media type
                                DWORD,       // flags
                                LONGLONG,    // timestamp
                                LONGLONG,    // duration
                                const BYTE*, // buffer
                                DWORD);      // buffer size
  STDMETHODIMP OnShutdown ();
  // IMFSampleGrabberSinkCallback2
  STDMETHODIMP OnProcessSampleEx (REFGUID,         // major media type
                                  DWORD,           // flags
                                  LONGLONG,        // timestamp
                                  LONGLONG,        // duration
                                  const BYTE*,     // buffer
                                  DWORD,           // buffer size
                                  IMFAttributes*); // attributes handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Null (const Stream_MediaFramework_MediaFoundation_Null&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Null& operator= (const Stream_MediaFramework_MediaFoundation_Null&))

  volatile long referenceCount_;
};

#endif
