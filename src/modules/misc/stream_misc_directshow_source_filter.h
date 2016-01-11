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

//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------
// The class managing the output pin
class Stream_Misc_DirectShow_Source_Filter_OutputPin;

class Stream_Misc_DirectShow_Source_Filter
 : public CSource
{
 public:
  static CUnknown* WINAPI CreateInstance (LPUNKNOWN, HRESULT*);

 private:
  Stream_Misc_DirectShow_Source_Filter (LPUNKNOWN, HRESULT*);

  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter (const Stream_Misc_DirectShow_Source_Filter&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter& operator= (const Stream_Misc_DirectShow_Source_Filter&))
}; // Stream_Misc_DirectShow_Source_Filter

class Stream_Misc_DirectShow_Source_Filter_OutputPin
 : public CSourceStream
{
 public:
  Stream_Misc_DirectShow_Source_Filter_OutputPin (HRESULT*,
                                                  Stream_Misc_DirectShow_Source_Filter*,
                                                  LPCWSTR);
  virtual ~Stream_Misc_DirectShow_Source_Filter_OutputPin ();

  // override / implement (part of) CBasePin
  virtual STDMETHODIMP CheckMediaType (const CMediaType*);
  virtual STDMETHODIMP GetMediaType (int, CMediaType*);
  virtual STDMETHODIMP SetMediaType (const CMediaType*);

  // implement (part of) CBaseOutputPin
  virtual STDMETHODIMP DecideBufferSize (IMemAllocator*,
                                         ALLOCATOR_PROPERTIES*);

  // override /implement (part of) CSourceStream
  virtual STDMETHODIMP OnThreadCreate (void);
  virtual STDMETHODIMP FillBuffer (IMediaSample*);

  // implement (part of) IQualityControl
  virtual STDMETHODIMP Notify (IBaseFilter*,
                               Quality);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin (const Stream_Misc_DirectShow_Source_Filter_OutputPin&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_Filter_OutputPin& operator= (const Stream_Misc_DirectShow_Source_Filter_OutputPin&))
}; // Stream_Misc_DirectShow_Source_Filter_OutputPin

#endif
