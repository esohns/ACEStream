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

#ifndef STREAM_LIB_MEDIAFOUNDATION_TARGET_H
#define STREAM_LIB_MEDIAFOUNDATION_TARGET_H

#undef GetObject
#include "mfidl.h"
#include "mfobjects.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

#include "common_ui_windowtype_converter.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediafoundation_mediasource.h"
#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_lib_mediafoundation_target_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename MediaType>
class Stream_MediaFramework_MediaFoundation_Target_T
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
 , public Common_UI_WindowTypeConverter_T<HWND>
 , public Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                              DataMessageType,
                                                              struct Stream_MediaFramework_MediaFoundation_Configuration>
 , public IMFAsyncCallback
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
  typedef Common_UI_WindowTypeConverter_T<HWND> inherited3;
  typedef Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                              DataMessageType,
                                                              struct Stream_MediaFramework_MediaFoundation_Configuration> inherited4;

 public:
  Stream_MediaFramework_MediaFoundation_Target_T (ISTREAM_T*); // stream handle
  virtual ~Stream_MediaFramework_MediaFoundation_Target_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);
  using inherited4::initialize;

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement IMFAsyncCallback
  virtual STDMETHODIMP QueryInterface (REFIID,
                                       void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&(inherited4::referenceCount_)); }
  inline virtual STDMETHODIMP_ (ULONG) Release () { ULONG count = InterlockedDecrement (&(inherited4::referenceCount_)); return count; }
  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  inline virtual STDMETHODIMP GetParameters (DWORD* flags_out, DWORD* queue_out) { ACE_UNUSED_ARG (flags_out); ACE_UNUSED_ARG (queue_out); return E_NOTIMPL; }
  virtual STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

 protected:
  // helper methods
  virtual bool initializeMediaSession (IMFMediaType*,                  // sample grabber sink input media type handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                       IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle
#else
                                       IMFSampleGrabberSinkCallback*,  // sample grabber sink callback handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                       TOPOID&,                        // return value: sample grabber node id
                                       IMFMediaSession*&);             // intput/return value: media session handle
  inline virtual void finalizeMediaSession () {}
  inline virtual bool updateMediaSession (IMFMediaType*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) } // (new) source media type handle

  bool             isFirst_;

  LONGLONG         baseTimeStamp_;
  bool             manageMediaSession_;
  IMFMediaSession* mediaSession_;

 private:
  // convenient types
  typedef Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                                         TimePolicyType,
                                                         ConfigurationType,
                                                         ControlMessageType,
                                                         DataMessageType,
                                                         SessionMessageType,
                                                         SessionDataType,
                                                         SessionDataContainerType,
                                                         MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Target_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Target_T (const Stream_MediaFramework_MediaFoundation_Target_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Target_T& operator= (const Stream_MediaFramework_MediaFoundation_Target_T&))
};

// include template definition
#include "stream_lib_mediafoundation_target.inl"

#endif
