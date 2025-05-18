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

#ifndef STREAM_LIB_DIRECTSHOW_SOURCE_H
#define STREAM_LIB_DIRECTSHOW_SOURCE_H

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
#include "basetyps.h"
#include "Unknwnbase.h"
#include "strmif.h"
#include "qedit.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

 // forward declarations
class Stream_IAllocator;

extern const char libacestream_default_lib_directshow_source_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,
          ////////////////////////////////
          typename MediaType>
class Stream_MediaFramework_DirectShow_Source_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public ISampleGrabberCB
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_MediaFramework_DirectShow_Source_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_MediaFramework_DirectShow_Source_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement ISampleGrabberCB
  inline virtual STDMETHODIMP QueryInterface (REFIID riid, __deref_out void** ppv) { return NonDelegatingQueryInterface (riid, ppv); }
  inline virtual STDMETHODIMP_(ULONG) AddRef () { return S_OK; }
  inline virtual STDMETHODIMP_ (ULONG) Release () { return S_OK; }
  inline virtual STDMETHODIMP NonDelegatingQueryInterface (REFIID, void**) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (E_FAIL); ACE_NOTREACHED (return E_FAIL;) }
  inline virtual STDMETHODIMP BufferCB (double, BYTE*, long) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (E_FAIL); ACE_NOTREACHED (return E_FAIL;) }
  virtual STDMETHODIMP SampleCB (double,         // SampleTime
                                 IMediaSample*); // Sample

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_T (const Stream_MediaFramework_DirectShow_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Source_T& operator= (const Stream_MediaFramework_DirectShow_Source_T&))

  IGraphBuilder* IGraphBuilder_;
};

// include template definition
#include "stream_lib_directshow_source.inl"

#endif
