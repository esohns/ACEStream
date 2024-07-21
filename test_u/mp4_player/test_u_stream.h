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

#ifndef TEST_U_STREAM_H
#define TEST_U_STREAM_H

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

#include "test_u_common_modules.h"
#include "test_u_message.h"
#include "test_u_mp4_player_common.h"
#include "test_u_session_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_DirectShow_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_DirectShow_StreamState,
                        struct Test_U_MP4Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_U_MP4Player_DirectShow_ModuleHandlerConfiguration,
                        Test_U_MP4Player_DirectShow_SessionData,
                        Test_U_MP4Player_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_U_DirectShow_Message_t,
                        Test_U_DirectShow_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_DirectShow_StreamState,
                        struct Test_U_MP4Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_U_MP4Player_DirectShow_ModuleHandlerConfiguration,
                        Test_U_MP4Player_DirectShow_SessionData,
                        Test_U_MP4Player_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_U_DirectShow_Message_t,
                        Test_U_DirectShow_SessionMessage_t> inherited;

 public:
  Test_U_DirectShow_Stream ();
  inline virtual ~Test_U_DirectShow_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_DirectShow_Stream (const Test_U_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_DirectShow_Stream& operator= (const Test_U_DirectShow_Stream&))

  // modules
  Test_U_DirectShow_LibAVSource_Module      source_;
  Test_U_DirectShow_StatisticReport_Module  statisticReport_;
  Test_U_Splitter_Module                    splitter_;
#if defined (FAAD_SUPPORT)
  Test_U_DirectShow_FAADDecode_Module       faadAudioDecode_;
#endif // FAAD_SUPPORT
#if defined (SOX_SUPPORT)
  Test_U_DirectShow_SOXResampler_Module     SOXResample_;
#endif // SOX_SUPPORT
#if defined (FFMPEG_SUPPORT)
  Test_U_DirectShow_LibAVAudioDecode_Module audioDecode_;
  Test_U_DirectShow_LibAVDecode_Module      decode_;
  Test_U_DirectShow_LibAVHWDecode_Module    HWDecode_;
  Test_U_DirectShow_LibAVConvert_Module     convert_; // --> RGB
  Test_U_DirectShow_LibAVResize_Module      resize_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
  Test_U_DirectShow_Delay_Module            delay_;
  Test_U_WASAPI_Module                      WASAPISound_;
#if defined (GTK_SUPPORT)
  Test_U_DirectShow_GTK_Display_Module      GTKDisplay_;
#endif // GTK_SUPPORT
  Test_U_DirectShow_GDI_Display_Module      GDIDisplay_;
  Test_U_DirectShow_Direct2D_Display_Module Direct2DDisplay_;
  Test_U_DirectShow_Direct3D_Display_Module Direct3DDisplay_;
  Test_U_DirectShow_Display_Module          DirectShowDisplay_;
};

class Test_U_MediaFoundation_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_MediaFoundation_StreamState,
                        struct Test_U_MP4Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_U_MP4Player_MediaFoundation_ModuleHandlerConfiguration,
                        Test_U_MP4Player_MediaFoundation_SessionData,
                        Test_U_MP4Player_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_U_MediaFoundation_Message_t,
                        Test_U_MediaFoundation_SessionMessage_t>
 , public IMFAsyncCallback
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_MediaFoundation_StreamState,
                        struct Test_U_MP4Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_U_MP4Player_MediaFoundation_ModuleHandlerConfiguration,
                        Test_U_MP4Player_MediaFoundation_SessionData,
                        Test_U_MP4Player_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_U_MediaFoundation_Message_t,
                        Test_U_MediaFoundation_SessionMessage_t> inherited;

 public:
  Test_U_MediaFoundation_Stream ();
  virtual ~Test_U_MediaFoundation_Stream ();

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
  ACE_UNIMPLEMENTED_FUNC (Test_U_MediaFoundation_Stream (const Test_U_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_MediaFoundation_Stream& operator= (const Test_U_MediaFoundation_Stream&))

  // modules
  Test_U_MediaFoundation_LibAVSource_Module     source_;
  //Test_U_MediaFoundation_StatisticReport_Module statisticReport_;
  //Test_U_MediaFoundation_Direct3DDisplay_Module direct3DDisplay_;
  Test_U_MediaFoundation_Display_Module         display_;
  //Test_U_MediaFoundation_DisplayNull_Module     mediaFoundationDisplayNull_;
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Test_U_MediaFoundation_GTKCairoDisplay_Module            GTKCairoDisplay_;
//#endif // GTK_USE
//#endif // GUI_SUPPORT

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  // media session
  IMFMediaSession*                                           mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  ULONG                                                      referenceCount_;
};
#else
class Test_U_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_StreamState,
                        struct Test_U_MP4Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_U_FFMPEG_ModuleHandlerConfiguration,
                        Test_U_MP4Player_SessionData,
                        Test_U_MP4Player_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_U_Message_t,
                        Test_U_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_StreamState,
                        struct Test_U_MP4Player_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_U_FFMPEG_ModuleHandlerConfiguration,
                        Test_U_MP4Player_SessionData,
                        Test_U_MP4Player_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_U_Message_t,
                        Test_U_SessionMessage_t> inherited;

 public:
  Test_U_Stream ();
  inline virtual ~Test_U_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream (const Test_U_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream& operator= (const Test_U_Stream&))

  // modules
  Test_U_LibAVSource_Module      source_;
  Test_U_StatisticReport_Module  statisticReport_;
  Test_U_Splitter_Module         splitter_;
#if defined (FAAD_SUPPORT)
  Test_U_FAADDecode_Module       faadAudioDecode_;
#endif // FAAD_SUPPORT
#if defined (SOX_SUPPORT)
  Test_U_SOXResampler_Module     SOXResample_;
#endif // SOX_SUPPORT
#if defined (FFMPEG_SUPPORT)
  Test_U_LibAVAudioDecode_Module audioDecode_;
  Test_U_LibAVDecode_Module      decode_;
  Test_U_LibAVHWDecode_Module    HWDecode_;
  Test_U_LibAVConvert_Module     convert_; // --> BGRA (Xlib)
  Test_U_LibAVResize_Module      resize_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
  Test_U_Delay_Module            delay_;
  Test_U_ALSA_Module             ALSASound_;
#if defined (GTK_SUPPORT)
  Test_U_GTK_Display_Module      GTKDisplay_;
#endif // GTK_SUPPORT
  Test_U_Wayland_Display_Module  WaylandDisplay_;
  Test_U_X11_Display_Module      X11Display_;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
