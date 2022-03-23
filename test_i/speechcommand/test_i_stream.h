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

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_stat_common.h"
#include "stream_stat_statistic_report.h"

#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_speechcommand_common.h"
#include "test_i_modules.h"

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_SpeechCommand_DirectShow_StreamState,
                        struct Test_I_DirectShow_StreamConfiguration,
                        struct Test_I_Statistic,
                        struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                        Test_I_SpeechCommand_DirectShow_SessionData,
                        Test_I_SpeechCommand_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_DirectShow_Message,
                        Test_I_DirectShow_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_SpeechCommand_DirectShow_StreamState,
                        struct Test_I_DirectShow_StreamConfiguration,
                        struct Test_I_Statistic,
                        struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration,
                        Test_I_SpeechCommand_DirectShow_SessionData,
                        Test_I_SpeechCommand_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_DirectShow_Message,
                        Test_I_DirectShow_SessionMessage_t> inherited;

 public:
  Test_I_DirectShow_Stream ();
  inline virtual ~Test_I_DirectShow_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Stream (const Test_I_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Stream& operator= (const Test_I_DirectShow_Stream&))
};

class Test_I_MediaFoundation_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_SpeechCommand_MediaFoundation_StreamState,
                        struct Test_I_MediaFoundation_StreamConfiguration,
                        struct Test_I_Statistic,
                        struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_SpeechCommand_MediaFoundation_SessionData,
                        Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_MediaFoundation_Message,
                        Test_I_MediaFoundation_SessionMessage_t>
 , public Common_IGetR_4_T<Test_I_MediaFoundation_Target>
 , public Common_IGetR_5_T<Test_I_MediaFoundation_Source>
 , public Common_IGetR_6_T<Test_I_Mic_Source_MediaFoundation>
 , public IMFAsyncCallback
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_SpeechCommand_MediaFoundation_StreamState,
                        struct Test_I_MediaFoundation_StreamConfiguration,
                        struct Test_I_Statistic,
                        struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_SpeechCommand_MediaFoundation_SessionData,
                        Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_MediaFoundation_Message,
                        Test_I_MediaFoundation_SessionMessage_t> inherited;

 public:
  Test_I_MediaFoundation_Stream ();
  virtual ~Test_I_MediaFoundation_Stream ();

  // override (part of) Stream_IStreamControl_T
  virtual void start ();
  virtual void stop (bool = true,   // wait for completion ?
                     bool = true,   // recurse upstream ?
                     bool = false); // high priority ?

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&); // configuration

  virtual const Test_I_MediaFoundation_Target& getR_4 () const; // return value: type
  virtual const Test_I_MediaFoundation_Source& getR_5 () const; // return value: type
  virtual const Test_I_Mic_Source_MediaFoundation& getR_6 () const; // return value: type

  // implement IMFAsyncCallback
  virtual STDMETHODIMP QueryInterface (REFIID,
                                       void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&referenceCount_); }
  inline virtual STDMETHODIMP_ (ULONG) Release () { ULONG count = InterlockedDecrement (&referenceCount_); return count; }
  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  inline virtual STDMETHODIMP GetParameters (DWORD* flags_out, DWORD* queue_out) { ACE_UNUSED_ARG (flags_out); ACE_UNUSED_ARG (queue_out); return E_NOTIMPL; }
  virtual STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Stream (const Test_I_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Stream& operator= (const Test_I_MediaFoundation_Stream&))

  ACE_SYNCH_CONDITION                      condition_;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IMFMediaSession*                         mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  Test_I_Mic_Source_MediaFoundation_Module frameworkSource_;
  Test_I_MediaFoundation_Source_Module     mediaFoundationSource_;
  Test_I_MediaFoundation_Target_Module     mediaFoundationTarget_;
  ULONG                                    referenceCount_;
  bool                                     topologyIsReady_;
};
#else
class Test_I_ALSA_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_SpeechCommand_ALSA_StreamState,
                        struct Test_I_ALSA_StreamConfiguration,
                        struct Test_I_Statistic,
                        struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                        Test_I_SpeechCommand_ALSA_SessionData,
                        Test_I_SpeechCommand_ALSA_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_ALSA_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_SpeechCommand_ALSA_StreamState,
                        struct Test_I_ALSA_StreamConfiguration,
                        struct Test_I_Statistic,
                        struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration,
                        Test_I_SpeechCommand_ALSA_SessionData,
                        Test_I_SpeechCommand_ALSA_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_ALSA_SessionMessage_t> inherited;

 public:
  Test_I_ALSA_Stream ();
  inline virtual ~Test_I_ALSA_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_ALSA_Stream (const Test_I_ALSA_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_ALSA_Stream& operator= (const Test_I_ALSA_Stream&))
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
