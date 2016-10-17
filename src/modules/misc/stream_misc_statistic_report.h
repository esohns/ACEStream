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

#ifndef STREAM_MODULE_RUNTIMESTATISTIC_H
#define STREAM_MODULE_RUNTIMESTATISTIC_H

#include <map>
#include <utility>

#include <ace/Global_Macros.h>
#include <ace/Stream_Modules.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include "common_icounter.h"
#include "common_istatistic.h"

#include "stream_common.h"
#include "stream_resetcounterhandler.h"
#include "stream_statistichandler.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType> class Stream_Module_StatisticReport_WriterTask_T; // session message payload (reference counted)

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
class Stream_Module_StatisticReport_ReaderTask_T
 : public ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType>
{
 friend class Stream_Module_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                                         TimePolicyType,
                                                         ConfigurationType,
                                                         ControlMessageType,
                                                         DataMessageType,
                                                         SessionMessageType,
                                                         ProtocolCommandType,
                                                         StatisticContainerType,
                                                         SessionDataType,
                                                         SessionDataContainerType>;

 public:
  Stream_Module_StatisticReport_ReaderTask_T ();
  virtual ~Stream_Module_StatisticReport_ReaderTask_T ();

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> inherited;
  typedef Stream_Module_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                                     TimePolicyType,
                                                     ConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType,
                                                     ProtocolCommandType,
                                                     StatisticContainerType,
                                                     SessionDataType,
                                                     SessionDataContainerType> WRITER_TASK_T;
  typedef DataMessageType MESSAGE_T;
  typedef ProtocolCommandType COMMAND_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_StatisticReport_ReaderTask_T (const Stream_Module_StatisticReport_ReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_StatisticReport_ReaderTask_T& operator= (const Stream_Module_StatisticReport_ReaderTask_T&))

  // *NOTE*: if data travels downstream and upstream again, account for it only
  //         once (e.g. data that travels upstream again to be dispatched from
  //         there)
  bool hasRoundTripData_;
};

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
class Stream_Module_StatisticReport_WriterTask_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType>
 , public Common_ICounter
 , public Common_IStatistic_T<StatisticContainerType>
{
 friend class Stream_Module_StatisticReport_ReaderTask_T<ACE_SYNCH_USE,
                                                         TimePolicyType,
                                                         ConfigurationType,
                                                         ControlMessageType,
                                                         DataMessageType,
                                                         SessionMessageType,
                                                         ProtocolCommandType,
                                                         StatisticContainerType,
                                                         SessionDataType,
                                                         SessionDataContainerType>;

 public:
  Stream_Module_StatisticReport_WriterTask_T ();
  virtual ~Stream_Module_StatisticReport_WriterTask_T ();

  // initialization
  virtual bool initialize (const ConfigurationType&);
  //bool initialize (const ACE_Time_Value&,      // reporting interval (second(s)) [ACE_Time_Value::zero: off]
  //                 bool = false,               // push 1-second interval statistic messages downstream ?
  //                 bool = false,               // print final report ?
  //                 Stream_IAllocator* = NULL); // report cache usage ? [NULL: off]

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ICounter
  virtual void reset ();

  // implement Common_IStatistic
  virtual bool collect (StatisticContainerType&); // return value: info
  // *NOTE*: this also implements locally triggered reporting
  virtual void report () const;

 protected:
  // *NOTE*: protects statistic
  mutable ACE_SYNCH_MUTEX    lock_;

  // *DATA STATISTIC*
  float                      inboundBytes_;
  float                      outboundBytes_;
  unsigned int               inboundMessages_;
  unsigned int               outboundMessages_;

  // *NOTE: support asynchronous collecting/reporting of data
  size_t                     lastBytesPerSecondCount_;
  unsigned int               lastDataMessagesPerSecondCount_;

  // *IMPORTANT NOTE*: data messages == (messageCounter_ - sessionMessages_)
  // *IMPORTANT NOTE*: data messages == (xxboundMessages_ - sessionMessages_)
  // *TODO*: currently, session messages travel only downstream
  unsigned int               sessionMessages_;

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE, 
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType> inherited;

  // convenient types
  typedef Stream_Module_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                                     TimePolicyType,
                                                     ConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType,
                                                     ProtocolCommandType,
                                                     StatisticContainerType,
                                                     SessionDataType,          // session data
                                                     SessionDataContainerType> OWN_TYPE_T;
  typedef Stream_StatisticHandler_Reactor_T<StatisticContainerType> REPORTING_HANDLER_T;
  typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_SYNCH_USE,
                                                     TimePolicyType,
                                                     ConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType,
                                                     ProtocolCommandType,
                                                     StatisticContainerType,
                                                     SessionDataType,
                                                     SessionDataContainerType> READER_TASK_T;

  // message type counters
  typedef std::map<ProtocolCommandType,
                   unsigned int> STATISTIC_T;
  typedef typename STATISTIC_T::const_iterator STATISTIC_ITERATOR_T;
  typedef std::pair<ProtocolCommandType,
                    unsigned int> STATISTIC_RECORD_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_StatisticReport_WriterTask_T (const Stream_Module_StatisticReport_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_StatisticReport_WriterTask_T& operator= (const Stream_Module_StatisticReport_WriterTask_T&))

  // helper method(s)
  void finalReport () const;
  void finiTimers (bool = true); // cancel both timers ? [false: cancel localReportingHandlerID_ only]
  // *IMPORTANT NOTE*: callers must hold lock_ !
  bool putStatisticMessage ();

  // timer stuff
  Stream_ResetCounterHandler resetTimeoutHandler_;
  long                       resetTimeoutHandlerID_;
  //ACE_thread_t               timerThreadID_;
  REPORTING_HANDLER_T        localReportingHandler_;
  long                       localReportingHandlerID_;
  ACE_Time_Value             reportingInterval_; // [ACE_Time_Value::zero: off]
  bool                       printFinalReport_;
  bool                       pushStatisticMessages_; // 1-second interval

  // used to compute data/message throughput
  size_t                     byteCounter_;
  unsigned int               fragmentCounter_;
  unsigned int               messageCounter_;
  unsigned int               sessionMessageCounter_;

  // *protocol statistic*
  STATISTIC_T                messageTypeStatistic_;

  // monitor amount of cached memory
  Stream_IAllocator*         allocator_;
};

// include template definition
#include "stream_misc_statistic_report.inl"

#endif
