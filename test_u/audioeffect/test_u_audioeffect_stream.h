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

#ifndef TEST_U_AUDIOEFFECT_STREAM_H
#define TEST_U_AUDIOEFFECT_STREAM_H

#include <ace/config-lite.h>
#include <ace/Atomic_Op.h>
#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>
#include <ace/Thread_Mutex.h>

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_message.h"
#include "test_u_audioeffect_session_message.h"

// forward declarations
class Stream_IAllocator;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_AudioEffect_DirectShow_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Test_U_AudioEffect_DirectShow_StreamConfiguration,
                        Test_U_AudioEffect_RuntimeStatistic,
                        Stream_ModuleConfiguration,
                        Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration,
                        Test_U_AudioEffect_DirectShow_SessionData,
                        Test_U_AudioEffect_DirectShow_SessionData_t,
                        Test_U_AudioEffect_ControlMessage_t,
                        Test_U_AudioEffect_DirectShow_Message,
                        Test_U_AudioEffect_DirectShow_SessionMessage>
{
 public:
  Test_U_AudioEffect_DirectShow_Stream ();
  virtual ~Test_U_AudioEffect_DirectShow_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_U_AudioEffect_DirectShow_StreamConfiguration&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_U_AudioEffect_RuntimeStatistic&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Test_U_AudioEffect_DirectShow_StreamConfiguration,
                        Test_U_AudioEffect_RuntimeStatistic,
                        Stream_ModuleConfiguration,
                        Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration,
                        Test_U_AudioEffect_DirectShow_SessionData,
                        Test_U_AudioEffect_DirectShow_SessionData_t,
                        Test_U_AudioEffect_ControlMessage_t,
                        Test_U_AudioEffect_DirectShow_Message,
                        Test_U_AudioEffect_DirectShow_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_Stream (const Test_U_AudioEffect_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_DirectShow_Stream& operator= (const Test_U_AudioEffect_DirectShow_Stream&))

  static ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> currentSessionID;

  IGraphBuilder* graphBuilder_;
};

//////////////////////////////////////////

class Test_U_AudioEffect_MediaFoundation_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Test_U_AudioEffect_MediaFoundation_StreamConfiguration,
                        Test_U_AudioEffect_RuntimeStatistic,
                        Stream_ModuleConfiguration,
                        Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration,
                        Test_U_AudioEffect_MediaFoundation_SessionData,
                        Test_U_AudioEffect_MediaFoundation_SessionData_t,
                        Test_U_AudioEffect_ControlMessage_t,
                        Test_U_AudioEffect_MediaFoundation_Message,
                        Test_U_AudioEffect_MediaFoundation_SessionMessage>
 , public IMFAsyncCallback
{
 public:
  Test_U_AudioEffect_MediaFoundation_Stream ();
  virtual ~Test_U_AudioEffect_MediaFoundation_Stream ();

  // override (part of) Stream_IStreamControl_T
  virtual const Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_U_AudioEffect_MediaFoundation_StreamConfiguration&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_U_AudioEffect_RuntimeStatistic&); // return value: statistic data
  virtual void report () const;

  // implement IMFAsyncCallback
  virtual STDMETHODIMP QueryInterface (const IID&,
                                       void**);
  virtual STDMETHODIMP_ (ULONG) AddRef ();
  virtual STDMETHODIMP_ (ULONG) Release ();
  virtual STDMETHODIMP GetParameters (DWORD*,  // return value: flags
                                      DWORD*); // return value: queue handle
  virtual STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Test_U_AudioEffect_MediaFoundation_StreamConfiguration,
                        Test_U_AudioEffect_RuntimeStatistic,
                        Stream_ModuleConfiguration,
                        Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration,
                        Test_U_AudioEffect_MediaFoundation_SessionData,
                        Test_U_AudioEffect_MediaFoundation_SessionData_t,
                        Test_U_AudioEffect_ControlMessage_t,
                        Test_U_AudioEffect_MediaFoundation_Message,
                        Test_U_AudioEffect_MediaFoundation_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_Stream (const Test_U_AudioEffect_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_MediaFoundation_Stream& operator= (const Test_U_AudioEffect_MediaFoundation_Stream&))

  // media session
  IMFMediaSession* mediaSession_;
  ULONG            referenceCount_;

  static ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> currentSessionID;
};
#else
class Test_U_AudioEffect_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_U_AudioEffect_StreamConfiguration,
                        struct Test_U_AudioEffect_RuntimeStatistic,
                        struct Stream_ModuleConfiguration,
                        struct Test_U_AudioEffect_ModuleHandlerConfiguration,
                        struct Test_U_AudioEffect_SessionData,
                        Test_U_AudioEffect_SessionData_t,
                        Test_U_AudioEffect_ControlMessage_t,
                        Test_U_AudioEffect_Message,
                        Test_U_AudioEffect_SessionMessage>
{
 public:
  Test_U_AudioEffect_Stream ();
  virtual ~Test_U_AudioEffect_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const struct Test_U_AudioEffect_StreamConfiguration&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (struct Test_U_AudioEffect_RuntimeStatistic&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_U_AudioEffect_StreamConfiguration,
                        struct Test_U_AudioEffect_RuntimeStatistic,
                        struct Stream_ModuleConfiguration,
                        struct Test_U_AudioEffect_ModuleHandlerConfiguration,
                        struct Test_U_AudioEffect_SessionData,
                        Test_U_AudioEffect_SessionData_t,
                        Test_U_AudioEffect_ControlMessage_t,
                        Test_U_AudioEffect_Message,
                        Test_U_AudioEffect_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_Stream (const Test_U_AudioEffect_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AudioEffect_Stream& operator= (const Test_U_AudioEffect_Stream&))

  static ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> currentSessionID;
};
#endif

#endif
