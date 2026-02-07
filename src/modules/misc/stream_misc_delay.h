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

#ifndef STREAM_MODULE_DELAY_H
#define STREAM_MODULE_DELAY_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_icounter.h"

#include "common_timer_resetcounterhandler.h"

#include "stream_common.h"
#include "stream_task_base_asynch.h"
#include "stream_task_base_synch.h"

#include "stream_misc_common.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_misc_delay_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ///////////////////////////////
          typename MediaType,
          ///////////////////////////////
          typename UserDataType>
class Stream_Module_Delay_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_ICounter
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Module_Delay_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_Delay_T () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message handle
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Delay_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Delay_T (const Stream_Module_Delay_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Delay_T& operator= (const Stream_Module_Delay_T&))

  // implement Common_ICounter
  virtual void reset ();

  // enqueue MB_STOP --> stop worker thread(s)
  virtual void stop (bool = true,   // wait for completion ?
                     bool = false); // high priority ? (i.e. do not wait for queued messages)

  // override (part of) ACE_Task_Base
  virtual int svc (void);
  int svc_2 (void); // outbound dispatch

  void dispatch (ACE_Message_Block*); // next message

  ACE_UINT64                          availableTokens_;
  ACE_SYNCH_CONDITION                 condition_;
  bool                                isFirstDispatchingThread_;
  typename inherited::MESSAGE_QUEUE_T queue_; // inbound-
  typename inherited::MESSAGE_QUEUE_T queue_2_; // outbound-
  Common_Timer_ResetCounterHandler    resetTimeoutHandler_;
  long                                resetTimeoutHandlerId_;
  // *NOTE*: only needed for the 'synchronous' version !
  bool                                resizeOccured_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HANDLE                              task_; // inbound dispatch task
  HANDLE                              task_2_; // outbound dispatch task
#endif // ACE_WIN32 || ACE_WIN64
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ///////////////////////////////
          typename MediaType,
          ///////////////////////////////
          typename UserDataType>
class Stream_Module_Delay_2
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_ICounter
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Module_Delay_2 (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_Delay_2 () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Delay_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Delay_2 (const Stream_Module_Delay_2&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Delay_2& operator= (const Stream_Module_Delay_2&))

  // implement Common_ICounter
  virtual void reset ();

  ACE_UINT64                       availableTokens_;
  ACE_SYNCH_CONDITION              condition_;
  Common_Timer_ResetCounterHandler resetTimeoutHandler_;
  long                             resetTimeoutHandlerId_;
};

// include template definition
#include "stream_misc_delay.inl"

#endif
