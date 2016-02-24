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

#ifndef STREAM_MISC_DIRECTSHOW_SOURCE_FILTER_H
#define STREAM_MISC_DIRECTSHOW_SOURCE_FILTER_H

#include "ace/Global_Macros.h"

#include "dshow.h"
#include "streams.h"

#include "stream_task_base_synch.h"

// forward declarations
template <typename FilterType>
class Stream_Misc_DirectShow_Source_Filter_OutputPin_T;

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
class Stream_Misc_DirectShow_Source_Filter_T
 : public Stream_TaskBaseSynch_T<TimePolicyType,
                                 SessionMessageType,
                                 ProtocolMessageType>
 , public CSource
{
 public:
  static CUnknown* WINAPI CreateInstance (LPUNKNOWN, //
                                          HRESULT*); // return value: result
  //static void WINAPI InitializeInstance (BOOL,
  //                                       const CLSID*);

 private:
  typedef Stream_TaskBaseSynch_T<TimePolicyType,
                                 SessionMessageType,
                                 ProtocolMessageType> inherited;
  typedef CSource inherited2;

  typedef Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                                 SessionMessageType,
                                                 ProtocolMessageType> OWN_TYPE_T;
  typedef Stream_Misc_DirectShow_Source_Filter_OutputPin_T<OWN_TYPE_T> FILTER_T;

  Stream_Misc_DirectShow_Source_Filter_T (LPTSTR,      // name
                                          LPUNKNOWN,   // 
                                          const GUID&, // CLSID
                                          HRESULT*);   // return value: result

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_T (const Stream_Misc_DirectShow_Source_Filter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_T& operator= (const Stream_Misc_DirectShow_Source_Filter_T&))
}; // Stream_Misc_DirectShow_Source_Filter_T

template <typename FilterType>
class Stream_Misc_DirectShow_Source_Filter_OutputPin_T
 : public CSourceStream
{
 public:
  Stream_Misc_DirectShow_Source_Filter_OutputPin_T (HRESULT*,    // result
                                                    FilterType*, // parent
                                                    LPCWSTR);    // name
  virtual ~Stream_Misc_DirectShow_Source_Filter_OutputPin_T ();

  // override / implement (part of) CBasePin
  virtual HRESULT CheckMediaType (const CMediaType*);
  virtual HRESULT GetMediaType (int, CMediaType*);
  virtual HRESULT SetMediaType (const CMediaType*);

  // implement (part of) CBaseOutputPin
  virtual HRESULT DecideBufferSize (IMemAllocator*,
                                    struct _AllocatorProperties*);

  // override /implement (part of) CSourceStream
  virtual HRESULT OnThreadCreate (void);
  virtual HRESULT FillBuffer (IMediaSample*);

  // implement (part of) IQualityControl
  STDMETHODIMP Notify (IBaseFilter*,
                       Quality);

 private:
  typedef CSourceStream inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin_T (const Stream_Misc_DirectShow_Source_Filter_OutputPin_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin_T& operator= (const Stream_Misc_DirectShow_Source_Filter_OutputPin_T&))

  const int defaultFrameInterval_; // Initial frame interval (ms)

  int       frameInterval_;        // Time in msec between frames
  CCritSec  lock_;                 // Lock on sampleTime_
  CRefTime  sampleTime_;           // The time stamp for each sample
}; // Stream_Misc_DirectShow_Source_Filter_OutputPin_T

   // include template definition
#include "stream_misc_directshow_source_filter.inl"

#endif
