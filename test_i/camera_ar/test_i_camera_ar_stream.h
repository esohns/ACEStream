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

#ifndef TEST_I_CAMERA_AR_STREAM_H
#define TEST_I_CAMERA_AR_STREAM_H

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

#include "test_i_camera_ar_common.h"
#include "test_i_camera_ar_common_modules.h"
#include "test_i_camera_ar_message.h"
#include "test_i_camera_ar_session_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Stream_CameraAR_DirectShow_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraAR_DirectShow_StreamState,
                        struct Stream_CameraAR_DirectShow_StreamConfiguration,
                        struct Stream_CameraAR_StatisticData,
                        struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                        Stream_CameraAR_DirectShow_SessionData,
                        Stream_CameraAR_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraAR_DirectShow_Message_t,
                        Stream_CameraAR_DirectShow_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraAR_DirectShow_StreamState,
                        struct Stream_CameraAR_DirectShow_StreamConfiguration,
                        struct Stream_CameraAR_StatisticData,
                        struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                        Stream_CameraAR_DirectShow_SessionData,
                        Stream_CameraAR_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraAR_DirectShow_Message_t,
                        Stream_CameraAR_DirectShow_SessionMessage_t> inherited;

 public:
  Stream_CameraAR_DirectShow_Stream ();
  inline virtual ~Stream_CameraAR_DirectShow_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_DirectShow_Stream (const Stream_CameraAR_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_DirectShow_Stream& operator= (const Stream_CameraAR_DirectShow_Stream&))

  // modules
  Stream_CameraAR_DirectShow_Source_Module           source_;
  Stream_CameraAR_DirectShow_StatisticReport_Module  statisticReport_;
#if defined (FFMPEG_SUPPORT)
  Stream_CameraAR_DirectShow_LibAVConvert_Module     convert_; // RGB
  Stream_CameraAR_DirectShow_LibAVResize_Module      resize_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
  Stream_CameraAR_DirectShow_HFlip_Module            flip_;
#if defined (GTK_SUPPORT)
  Stream_CameraAR_DirectShow_GTK_Display_Module      GTKDisplay_;
#endif // GTK_SUPPORT
  Stream_CameraAR_DirectShow_GDI_Display_Module      GDIDisplay_;
  Stream_CameraAR_DirectShow_Direct2D_Display_Module Direct2DDisplay_;
  Stream_CameraAR_DirectShow_Direct3D_Display_Module Direct3DDisplay_;
  Stream_CameraAR_DirectShow_Display_Module          DirectShowDisplay_;
};

class Stream_CameraAR_MediaFoundation_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraAR_MediaFoundation_StreamState,
                        struct Stream_CameraAR_MediaFoundation_StreamConfiguration,
                        struct Stream_CameraAR_StatisticData,
                        struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                        Stream_CameraAR_MediaFoundation_SessionData,
                        Stream_CameraAR_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraAR_MediaFoundation_Message_t,
                        Stream_CameraAR_MediaFoundation_SessionMessage_t>
 , public IMFAsyncCallback
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraAR_MediaFoundation_StreamState,
                        struct Stream_CameraAR_MediaFoundation_StreamConfiguration,
                        struct Stream_CameraAR_StatisticData,
                        struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                        Stream_CameraAR_MediaFoundation_SessionData,
                        Stream_CameraAR_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraAR_MediaFoundation_Message_t,
                        Stream_CameraAR_MediaFoundation_SessionMessage_t> inherited;

 public:
  Stream_CameraAR_MediaFoundation_Stream ();
  virtual ~Stream_CameraAR_MediaFoundation_Stream ();

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
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_MediaFoundation_Stream (const Stream_CameraAR_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_MediaFoundation_Stream& operator= (const Stream_CameraAR_MediaFoundation_Stream&))

  // modules
  Stream_CameraAR_MediaFoundation_Source_Module          source_;
  //Stream_CameraAR_MediaFoundation_StatisticReport_Module statisticReport_;
  //Stream_CameraAR_MediaFoundation_Direct3DDisplay_Module direct3DDisplay_;
  Stream_CameraAR_MediaFoundation_Display_Module         display_;
  //Stream_CameraAR_MediaFoundation_DisplayNull_Module     mediaFoundationDisplayNull_;
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Stream_CameraAR_MediaFoundation_GTKCairoDisplay_Module            GTKCairoDisplay_;
//#endif // GTK_USE
//#endif // GUI_SUPPORT

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  // media session
  IMFMediaSession*                                           mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ULONG                                                      referenceCount_;
};
#else
class Stream_CameraAR_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraAR_StreamState,
                        struct Stream_CameraAR_V4L_StreamConfiguration,
                        struct Stream_CameraAR_StatisticData,
                        struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                        Stream_CameraAR_V4L_SessionData,
                        Stream_CameraAR_V4L_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraAR_Message_t,
                        Stream_CameraAR_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraAR_StreamState,
                        struct Stream_CameraAR_V4L_StreamConfiguration,
                        struct Stream_CameraAR_StatisticData,
                        struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                        Stream_CameraAR_V4L_SessionData,
                        Stream_CameraAR_V4L_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraAR_Message_t,
                        Stream_CameraAR_SessionMessage_t> inherited;

 public:
  Stream_CameraAR_Stream ();
  inline virtual ~Stream_CameraAR_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_Stream (const Stream_CameraAR_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraAR_Stream& operator= (const Stream_CameraAR_Stream&))

  // modules
  Stream_CameraAR_V4L_Source_Module      source_;
  Stream_CameraAR_StatisticReport_Module statisticReport_;
  Stream_CameraAR_LibAVConvert_Module    convert_; // --> BGRA (Xlib)
  Stream_CameraAR_LibAVResize_Module     resize_; // --> window size/fullscreen
#if defined (GTK_SUPPORT)
  Stream_CameraAR_GTK_Display_Module     GTKDisplay_;
#endif // GTK_SUPPORT
//  Stream_CameraAR_Wayland_Display_Module WaylandDisplay_;
  Stream_CameraAR_X11_Display_Module     X11Display_;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
