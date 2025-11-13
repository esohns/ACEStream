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

#ifndef TEST_I_STREAM_H
#define TEST_I_STREAM_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
#include "winnt.h"
#include "guiddef.h"
#undef GetObject
#include "mfidl.h"
#include "mfobjects.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_i_camera_msa_common.h"
#include "test_i_common_modules.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_CameraMSA_DirectShow_StreamState,
                        struct Test_I_DirectShow_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                        Test_I_DirectShow_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_DirectShow_Message_t,
                        Test_I_DirectShow_SessionMessage_t,
                        struct Stream_UserData>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_CameraMSA_DirectShow_StreamState,
                        struct Test_I_DirectShow_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                        Test_I_DirectShow_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_DirectShow_Message_t,
                        Test_I_DirectShow_SessionMessage_t,
                        struct Stream_UserData> inherited;

 public:
  Test_I_DirectShow_Stream ();
  inline virtual ~Test_I_DirectShow_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Stream (const Test_I_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Stream& operator= (const Test_I_DirectShow_Stream&))

  // modules
  Test_I_DirectShow_Source_Module          source_;
  //Test_I_DirectShow_StatisticReport_Module statisticReport_;
#if defined (FFMPEG_SUPPORT)
  Test_I_DirectShow_LibAVConvert_Module    convert_; // RGB
  Test_I_DirectShow_LibAVResize_Module     resize_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
  Test_I_DirectShow_HFlip_Module           flip_;
};

class Test_I_MediaFoundation_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_CameraMSA_MediaFoundation_StreamState,
                        struct Test_I_MediaFoundation_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_MediaFoundation_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_MediaFoundation_Message_t,
                        Test_I_MediaFoundation_SessionMessage_t,
                        struct Stream_UserData>
 , public IMFAsyncCallback
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_CameraMSA_MediaFoundation_StreamState,
                        struct Test_I_MediaFoundation_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_MediaFoundation_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_MediaFoundation_Message_t,
                        Test_I_MediaFoundation_SessionMessage_t,
                        struct Stream_UserData> inherited;

 public:
  Test_I_MediaFoundation_Stream ();
  virtual ~Test_I_MediaFoundation_Stream ();

  // override (part of) Stream_IStreamControl_T
  virtual const Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,   // wait for completion ?
                     bool = true,   // recurse upstream (if any) ?
                     bool = false); // locked access ?

  // implement IMFAsyncCallback
  virtual STDMETHODIMP QueryInterface (const IID&,
                                       void**);
  virtual STDMETHODIMP_ (ULONG) AddRef ();
  virtual STDMETHODIMP_ (ULONG) Release ();
  virtual STDMETHODIMP GetParameters (DWORD*,  // return value: flags
                                      DWORD*); // return value: queue handle
  virtual STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Stream (const Test_I_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Stream& operator= (const Test_I_MediaFoundation_Stream&))

  // modules
  Test_I_MediaFoundation_Source_Module          source_;
  //Test_I_MediaFoundation_StatisticReport_Module statisticReport_;
#if defined (FFMPEG_SUPPORT)
  Test_I_MediaFoundation_LibAVConvert_Module    convert_; // RGB
  Test_I_MediaFoundation_LibAVResize_Module     resize_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
  Test_I_MediaFoundation_HFlip_Module           flip_;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  // media session
  IMFMediaSession*                              mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  ULONG                                         referenceCount_;
};
#else
class Test_I_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_CameraMSA_StreamState,
                        struct Test_I_V4L_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                        Test_I_V4L_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Message_t,
                        Test_I_SessionMessage_t,
                        struct Stream_UserData>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_CameraMSA_StreamState,
                        struct Test_I_V4L_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                        Test_I_V4L_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Message_t,
                        Test_I_SessionMessage_t,
                        struct Stream_UserData> inherited;

 public:
  Test_I_Stream ();
  inline virtual ~Test_I_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream (const Test_I_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream& operator= (const Test_I_Stream&))

  // modules
  Test_I_V4L_Source_Module      source_;
  // Test_I_StatisticReport_Module statisticReport_;
#if defined (FFMPEG_SUPPORT)
  Test_I_LibAVDecode_Module     decode_; // --> BGRA (Xlib)
  Test_I_LibAVConvert_Module    convert_; // --> BGRA (Xlib)
  Test_I_LibAVResize_Module     resize_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
  Test_I_V4L_HFlip_Module       flip_;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif // TEST_I_STREAM_H
