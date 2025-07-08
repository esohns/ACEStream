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

#ifndef TEST_I_STREAM_3_H
#define TEST_I_STREAM_3_H

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

#include "test_i_camera_ml_common.h"
#include "test_i_common_modules.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Stream_CameraML_DirectShow_Stream_3
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraML_DirectShow_StreamState,
                        struct Stream_CameraML_DirectShow_StreamConfiguration,
                        struct Stream_CameraML_StatisticData,
                        struct Stream_CameraML_DirectShow_ModuleHandlerConfiguration,
                        Stream_CameraML_DirectShow_SessionData,
                        Stream_CameraML_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraML_DirectShow_Message_t,
                        Stream_CameraML_DirectShow_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraML_DirectShow_StreamState,
                        struct Stream_CameraML_DirectShow_StreamConfiguration,
                        struct Stream_CameraML_StatisticData,
                        struct Stream_CameraML_DirectShow_ModuleHandlerConfiguration,
                        Stream_CameraML_DirectShow_SessionData,
                        Stream_CameraML_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraML_DirectShow_Message_t,
                        Stream_CameraML_DirectShow_SessionMessage_t> inherited;

 public:
  Stream_CameraML_DirectShow_Stream_3 ();
  inline virtual ~Stream_CameraML_DirectShow_Stream_3 () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraML_DirectShow_Stream_3 (const Stream_CameraML_DirectShow_Stream_3&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraML_DirectShow_Stream_3& operator= (const Stream_CameraML_DirectShow_Stream_3&))

  // modules
  Stream_CameraML_DirectShow_Source_Module             source_;
  Stream_CameraML_DirectShow_StatisticReport_Module    statisticReport_;
#if defined (FFMPEG_SUPPORT)
  Stream_CameraML_DirectShow_LibAVConvert_Module       convert_; // RGB
  Stream_CameraML_DirectShow_LibAVResize_Module        resize_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
  Stream_CameraML_DirectShow_HFlip_Module              flip_;
  Stream_CameraML_DirectShow_MediaPipeBox2d_Module     mediaPipeBox2d_;
  Stream_CameraML_DirectShow_LibAVConvert_Module       convert_2; // --> BGRA (Direct3D)
#if defined (GTK_SUPPORT)
  Stream_CameraML_DirectShow_GTK_Display_Module        GTKDisplay_;
#endif // GTK_SUPPORT
  Stream_CameraML_DirectShow_GDI_Display_Module        GDIDisplay_;
  Stream_CameraML_DirectShow_Direct2D_Display_Module   Direct2DDisplay_;
  Stream_CameraML_DirectShow_Direct3D_Display_Module   Direct3DDisplay_;
  Stream_CameraML_DirectShow_Direct3D11_Display_Module Direct3D11Display_;
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  Stream_CameraML_DirectShow_Display_Module            DirectShowDisplay_;
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
};

class Stream_CameraML_MediaFoundation_Stream_3
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraML_MediaFoundation_StreamState,
                        struct Stream_CameraML_MediaFoundation_StreamConfiguration,
                        struct Stream_CameraML_StatisticData,
                        struct Stream_CameraML_MediaFoundation_ModuleHandlerConfiguration,
                        Stream_CameraML_MediaFoundation_SessionData,
                        Stream_CameraML_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraML_MediaFoundation_Message_t,
                        Stream_CameraML_MediaFoundation_SessionMessage_t>
 , public IMFAsyncCallback
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraML_MediaFoundation_StreamState,
                        struct Stream_CameraML_MediaFoundation_StreamConfiguration,
                        struct Stream_CameraML_StatisticData,
                        struct Stream_CameraML_MediaFoundation_ModuleHandlerConfiguration,
                        Stream_CameraML_MediaFoundation_SessionData,
                        Stream_CameraML_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraML_MediaFoundation_Message_t,
                        Stream_CameraML_MediaFoundation_SessionMessage_t> inherited;

 public:
  Stream_CameraML_MediaFoundation_Stream_3 ();
  virtual ~Stream_CameraML_MediaFoundation_Stream_3 ();

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
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraML_MediaFoundation_Stream_3 (const Stream_CameraML_MediaFoundation_Stream_3&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraML_MediaFoundation_Stream_3& operator= (const Stream_CameraML_MediaFoundation_Stream_3&))

  // modules
  Stream_CameraML_MediaFoundation_Source_Module          source_;
  //Stream_CameraML_MediaFoundation_StatisticReport_Module statisticReport_;
  //Stream_CameraML_MediaFoundation_Direct3DDisplay_Module direct3DDisplay_;
  Stream_CameraML_MediaFoundation_Display_Module         display_;
  //Stream_CameraML_MediaFoundation_DisplayNull_Module     mediaFoundationDisplayNull_;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  // media session
  IMFMediaSession*                                           mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ULONG                                                      referenceCount_;
};
#else
class Stream_CameraML_Stream_3
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraML_StreamState,
                        struct Stream_CameraML_V4L_StreamConfiguration,
                        struct Stream_CameraML_StatisticData,
                        struct Stream_CameraML_V4L_ModuleHandlerConfiguration,
                        Stream_CameraML_V4L_SessionData,
                        Stream_CameraML_V4L_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraML_Message_t,
                        Stream_CameraML_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CameraML_StreamState,
                        struct Stream_CameraML_V4L_StreamConfiguration,
                        struct Stream_CameraML_StatisticData,
                        struct Stream_CameraML_V4L_ModuleHandlerConfiguration,
                        Stream_CameraML_V4L_SessionData,
                        Stream_CameraML_V4L_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_CameraML_Message_t,
                        Stream_CameraML_SessionMessage_t> inherited;

 public:
  Stream_CameraML_Stream_3 ();
  inline virtual ~Stream_CameraML_Stream_3 () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraML_Stream_3 (const Stream_CameraML_Stream_3&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CameraML_Stream_3& operator= (const Stream_CameraML_Stream_3&))

  // modules
  Stream_CameraML_V4L_Source_Module      source_;
//  Stream_CameraML_StatisticReport_Module statisticReport_;
  Stream_CameraML_LibAVConvert_Module    convert_; // --> RGB24 (tensorflow)
  Stream_CameraML_LibAVResize_Module     resize_; // --> window size/fullscreen
  Stream_CameraML_V4L_HFlip_Module       flip_;
  Stream_CameraML_MediaPipeBox2d_Module  mediaPipeBox2d_;
#if defined (GTK_SUPPORT)
  Stream_CameraML_GTK_Display_Module     GTKDisplay_;
#endif // GTK_SUPPORT
  Stream_CameraML_LibAVConvert_Module    convert_2; // --> BGRA (X11|Wayland)
  Stream_CameraML_Wayland_Display_Module WaylandDisplay_;
  // Stream_CameraML_X11_Display_Module     X11Display_;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
