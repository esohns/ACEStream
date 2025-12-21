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

#ifndef STREAM_NET_OUTPUT_H
#define STREAM_NET_OUTPUT_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

extern const char libacestream_default_net_output_module_name_string[];

// forward declarations
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionNotificationType,
          typename ConnectionManagerType,
          typename UserDataType>
class Stream_Module_Net_OutputWriter_T;
class Stream_IOutboundDataNotify;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionControlType,
          typename SessionNotificationType,
          ////////////////////////////////
          typename ConnectionManagerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_OutputReader_T // --> input
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionNotificationType,
                                 UserDataType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionNotificationType,
                                 UserDataType> inherited;

 public:
  Stream_Module_Net_OutputReader_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_Net_OutputReader_T () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_OutputReader_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_OutputReader_T (const Stream_Module_Net_OutputReader_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_OutputReader_T& operator= (const Stream_Module_Net_OutputReader_T&))

  // convenient types
  typedef typename SessionMessageType::DATA_T::DATA_T SESSION_DATA_T;
  typedef Stream_Module_Net_OutputWriter_T<ACE_SYNCH_USE,
                                           TimePolicyType,
                                           ConfigurationType,
                                           ControlMessageType,
                                           DataMessageType,
                                           SessionMessageType,
                                           SessionControlType,
                                           SessionNotificationType,
                                           ConnectionManagerType,
                                           UserDataType> WRITER_T;
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
          ////////////////////////////////
          typename SessionControlType,
          typename SessionEventType,
          ////////////////////////////////
          typename ConnectionManagerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_OutputWriter_T // --> output
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 UserDataType>
{
  friend class Stream_Module_Net_OutputReader_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType,
                                                SessionControlType,
                                                SessionEventType,
                                                ConnectionManagerType,
                                                UserDataType>;

  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 UserDataType> inherited;

 public:
  Stream_Module_Net_OutputWriter_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_Net_OutputWriter_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_OutputWriter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_OutputWriter_T (const Stream_Module_Net_OutputWriter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_OutputWriter_T& operator= (const Stream_Module_Net_OutputWriter_T&))
};

// include template definition
#include "stream_net_output.inl"

#endif
