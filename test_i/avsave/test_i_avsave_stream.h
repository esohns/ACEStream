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

#ifndef TEST_I_AVSAVE_STREAM_H
#define TEST_I_AVSAVE_STREAM_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#include "winnt.h"
#include "guiddef.h"
#include "mfidl.h"
#include "mfobjects.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_i_avsave_common.h"
#include "test_i_avsave_common_modules.h"
#include "test_i_avsave_message.h"
#include "test_i_avsave_session_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];
extern const char stream_name_string_2[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Stream_AVSave_DirectShow_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_DirectShow_StreamState,
                        struct Stream_AVSave_DirectShow_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                        Stream_AVSave_DirectShow_SessionData,
                        Stream_AVSave_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_DirectShow_Message_t,
                        Stream_AVSave_DirectShow_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_DirectShow_StreamState,
                        struct Stream_AVSave_DirectShow_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                        Stream_AVSave_DirectShow_SessionData,
                        Stream_AVSave_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_DirectShow_Message_t,
                        Stream_AVSave_DirectShow_SessionMessage_t> inherited;

 public:
  Stream_AVSave_DirectShow_Stream ();
  virtual ~Stream_AVSave_DirectShow_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_DirectShow_Stream (const Stream_AVSave_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_DirectShow_Stream& operator= (const Stream_AVSave_DirectShow_Stream&))

  // modules
  Stream_AVSave_DirectShow_Source_Module            source_;
  Stream_AVSave_DirectShow_Video_Tagger_Module      tagger_;
  //Stream_AVSave_DirectShow_StatisticReport_Module   statisticReport_;
  //Stream_AVSave_DirectShow_Direct3DDisplay_Module   display_;
  //Stream_AVSave_DirectShow_DirectShowDisplay_Module directShowDisplay_;
  //Stream_AVSave_DirectShow_GTKCairoDisplay_Module   GTKCairoDisplay_;
  //Stream_AVSave_DirectShow_Encoder_Module           encoder_;
};

class Stream_AVSave_MediaFoundation_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_MediaFoundation_StreamState,
                        struct Stream_AVSave_MediaFoundation_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
                        Stream_AVSave_MediaFoundation_SessionData,
                        Stream_AVSave_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_MediaFoundation_Message_t,
                        Stream_AVSave_MediaFoundation_SessionMessage_t>
 , public IMFAsyncCallback
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_MediaFoundation_StreamState,
                        struct Stream_AVSave_MediaFoundation_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration,
                        Stream_AVSave_MediaFoundation_SessionData,
                        Stream_AVSave_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_MediaFoundation_Message_t,
                        Stream_AVSave_MediaFoundation_SessionMessage_t> inherited;

 public:
  Stream_AVSave_MediaFoundation_Stream ();
  virtual ~Stream_AVSave_MediaFoundation_Stream ();

  // override (part of) Stream_IStreamControl_T
  virtual const Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true,  // recurse upstream (if any) ?
                     bool = true); // locked access ?

  // implement IMFAsyncCallback
  virtual STDMETHODIMP QueryInterface (const IID&,
                                       void**);
  virtual STDMETHODIMP_ (ULONG) AddRef ();
  virtual STDMETHODIMP_ (ULONG) Release ();
  virtual STDMETHODIMP GetParameters (DWORD*,  // return value: flags
                                      DWORD*); // return value: queue handle
  virtual STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_MediaFoundation_Stream (const Stream_AVSave_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_MediaFoundation_Stream& operator= (const Stream_AVSave_MediaFoundation_Stream&))

  // modules
  Stream_AVSave_MediaFoundation_Source_Module                     source_;
  //Stream_AVSave_MediaFoundation_StatisticReport_Module            statisticReport_;
  //Stream_AVSave_MediaFoundation_MediaFoundationDisplay_Module     mediaFoundationDisplay_;
  //Stream_AVSave_MediaFoundation_MediaFoundationDisplayNull_Module mediaFoundationDisplayNull_;
  //Stream_AVSave_MediaFoundation_Direct3DDisplay_Module            display_;
  //Stream_AVSave_MediaFoundation_GTKCairoDisplay_Module            GTKCairoDisplay_;
  //Stream_AVSave_MediaFoundation_AVIEncoder_Module                 encoder_;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  // media session
  IMFMediaSession*                                                 mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ULONG                                                            referenceCount_;
};

class Stream_AVSave_WaveIn_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_DirectShow_StreamState,
                        struct Stream_AVSave_DirectShow_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                        Stream_AVSave_DirectShow_SessionData,
                        Stream_AVSave_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_DirectShow_Message_t,
                        Stream_AVSave_DirectShow_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_DirectShow_StreamState,
                        struct Stream_AVSave_DirectShow_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration,
                        Stream_AVSave_DirectShow_SessionData,
                        Stream_AVSave_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_DirectShow_Message_t,
                        Stream_AVSave_DirectShow_SessionMessage_t> inherited;

 public:
  Stream_AVSave_WaveIn_Stream ();
  virtual ~Stream_AVSave_WaveIn_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_WaveIn_Stream (const Stream_AVSave_WaveIn_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_WaveIn_Stream& operator= (const Stream_AVSave_WaveIn_Stream&))

  // modules
  Stream_AVSave_WaveIn_Source_Module     source_;
//  Stream_AVSave_StatisticReport_Module statisticReport_;
  Stream_AVSave_WaveIn_Tagger_Module     tagger_;
};
#else
class Stream_AVSave_V4L_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_V4L_StreamState,
                        struct Stream_AVSave_V4L_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                        Stream_AVSave_V4L_SessionData,
                        Stream_AVSave_V4L_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_Message_t,
                        Stream_AVSave_V4L_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_V4L_StreamState,
                        struct Stream_AVSave_V4L_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_V4L_ModuleHandlerConfiguration,
                        Stream_AVSave_V4L_SessionData,
                        Stream_AVSave_V4L_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_Message_t,
                        Stream_AVSave_V4L_SessionMessage_t> inherited;

 public:
  Stream_AVSave_V4L_Stream ();
  inline virtual ~Stream_AVSave_V4L_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_V4L_Stream (const Stream_AVSave_V4L_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_V4L_Stream& operator= (const Stream_AVSave_V4L_Stream&))

  // modules
  Stream_AVSave_V4L_Source_Module      source_;
  //  Stream_AVSave_StatisticReport_Module statisticReport_;
#if defined (FFMPEG_SUPPORT)
  Stream_AVSave_LibAVDecoder_Module    decoder_; // --> RGB
#endif // FFMPEG_SUPPORT
//  Stream_AVSave_Distributor_Module     distributor_; // (sub-)branch ?
  ////////////////////////////////////////
#if defined (FFMPEG_SUPPORT)
  Stream_AVSave_LibAVConverter_Module  converter_; // --> 24-bit RGB (display format)
//  Stream_AVSave_LibAVResize_Module     resizer_; // --> window size/fullscreen
#endif // FFMPEG_SUPPORT
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  Stream_AVSave_GTKCairoDisplay_Module GTKCairoDisplay_;
//  Stream_AVSave_Display_2_Module       display_2_;
#endif // GTK_USE
//  Stream_AVSave_Display_2_Module       display_2_;
#endif // GUI_SUPPORT
  ////////////////////////////////////////
#if defined (FFMPEG_SUPPORT)
  Stream_AVSave_LibAVConverter_Module  converter_2; // --> 32-bit RGB (AVI format)
#endif // FFMPEG_SUPPORT
  Stream_AVSave_V4L_Tagger_Module      tagger_;
  Stream_AVSave_Display_Module         display_;
//  Stream_AVSave_V4L_AVIEncoder_Module  encoder_; // --> AVI
//  Stream_AVSave_FileWriter_Module      fileWriter_;
};

class Stream_AVSave_ALSA_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_ALSA_StreamState,
                        struct Stream_AVSave_ALSA_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_ALSA_ModuleHandlerConfiguration,
                        Stream_AVSave_ALSA_SessionData,
                        Stream_AVSave_ALSA_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_Message_t,
                        Stream_AVSave_ALSA_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_AVSave_ALSA_StreamState,
                        struct Stream_AVSave_ALSA_StreamConfiguration,
                        struct Stream_AVSave_StatisticData,
                        struct Stream_AVSave_ALSA_ModuleHandlerConfiguration,
                        Stream_AVSave_ALSA_SessionData,
                        Stream_AVSave_ALSA_SessionData_t,
                        Stream_ControlMessage_t,
                        Stream_AVSave_Message_t,
                        Stream_AVSave_ALSA_SessionMessage_t> inherited;

 public:
  Stream_AVSave_ALSA_Stream ();
  inline virtual ~Stream_AVSave_ALSA_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_ALSA_Stream (const Stream_AVSave_ALSA_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_ALSA_Stream& operator= (const Stream_AVSave_ALSA_Stream&))

  // modules
  Stream_AVSave_ALSA_Source_Module     source_;
//  Stream_AVSave_StatisticReport_Module statisticReport_;
  Stream_AVSave_ALSA_Tagger_Module     tagger_;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif

