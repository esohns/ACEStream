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

#ifndef STREAM_STAT_STATISTIC_REPORT_H
#define STREAM_STAT_STATISTIC_REPORT_H

#include <map>
#include <utility>

#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_icounter.h"
#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_istreamcontrol.h"
#include "stream_resetcounterhandler.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
class Stream_Statistic_StatisticReport_WriterTask_T;

extern const char libacestream_default_stat_report_module_name_string[];

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
          typename TimerManagerType, // implements Common_ITimer
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
class Stream_Statistic_StatisticReport_ReaderTask_T
 : public ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType>
{
  friend class Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                                             TimePolicyType,
                                                             ConfigurationType,
                                                             ControlMessageType,
                                                             DataMessageType,
                                                             SessionMessageType,
                                                             ProtocolCommandType,
                                                             StatisticContainerType,
                                                             TimerManagerType,
                                                             SessionDataType,
                                                             SessionDataContainerType>;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;

  Stream_Statistic_StatisticReport_ReaderTask_T (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Statistic_StatisticReport_ReaderTask_T () {}

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> inherited;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                                        TimePolicyType,
                                                        ConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        ProtocolCommandType,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> WRITER_TASK_T;
  typedef DataMessageType MESSAGE_T;
  typedef ProtocolCommandType COMMAND_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticReport_ReaderTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticReport_ReaderTask_T (const Stream_Statistic_StatisticReport_ReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticReport_ReaderTask_T& operator= (const Stream_Statistic_StatisticReport_ReaderTask_T&))
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
          typename TimerManagerType, // implements Common_ITimer
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
class Stream_Statistic_StatisticReport_WriterTask_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Common_ICounter
 , public Common_IStatistic_T<StatisticContainerType>
{
  friend class Stream_Statistic_StatisticReport_ReaderTask_T<ACE_SYNCH_USE,
                                                             TimePolicyType,
                                                             ConfigurationType,
                                                             ControlMessageType,
                                                             DataMessageType,
                                                             SessionMessageType,
                                                             ProtocolCommandType,
                                                             StatisticContainerType,
                                                             TimerManagerType,
                                                             SessionDataType,
                                                             SessionDataContainerType>;

  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Statistic_StatisticReport_WriterTask_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Statistic_StatisticReport_WriterTask_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Stream_Statistic_StatisticReport_WriterTask_T () { finiTimers (true); }

  // initialization
  virtual bool initialize (const ConfigurationType&,   // configuration
                           Stream_IAllocator* = NULL); // allocator handle

  // implement (part of) Stream_ITaskBase
  virtual void handleControlMessage (ControlMessageType&); // control message handle
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

  unsigned int               controlMessages_;
  unsigned int               outboundControlMessages_;

 private:
  // convenient types
  typedef ACE_Singleton<TimerManagerType,
                        ACE_SYNCH_MUTEX> TIMER_MANAGER_SINGLETON_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                                        TimePolicyType,
                                                        ConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        ProtocolCommandType,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        SessionDataType,          // session data
                                                        SessionDataContainerType> OWN_TYPE_T;
  typedef Common_StatisticHandler_T<StatisticContainerType> STATISTIC_HANDLER_T;
  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_SYNCH_USE,
                                                        TimePolicyType,
                                                        ConfigurationType,
                                                        ControlMessageType,
                                                        DataMessageType,
                                                        SessionMessageType,
                                                        ProtocolCommandType,
                                                        StatisticContainerType,
                                                        TimerManagerType,
                                                        SessionDataType,
                                                        SessionDataContainerType> READER_TASK_T;

  // message type counters
  typedef std::map<ProtocolCommandType,
                   unsigned int> STATISTIC_T;
  typedef typename STATISTIC_T::const_iterator STATISTIC_ITERATOR_T;
  typedef std::pair<ProtocolCommandType,
                    unsigned int> STATISTIC_RECORD_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticReport_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticReport_WriterTask_T (const Stream_Statistic_StatisticReport_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticReport_WriterTask_T& operator= (const Stream_Statistic_StatisticReport_WriterTask_T&))

  // helper method(s)
  void finalReport () const;
  void finiTimers (bool = true); // cancel both timers ? [false: cancel localReportingHandlerID_ only]
  // *IMPORTANT NOTE*: callers must hold lock_ !
  bool putStatisticMessage ();

  bool                       inbound_;

  // timer
  Stream_ResetCounterHandler resetTimeoutHandler_;
  long                       resetTimeoutHandlerId_;
  STATISTIC_HANDLER_T        localReportingHandler_;
  long                       localReportingHandlerId_;
  ACE_Time_Value             reportingInterval_; // [ACE_Time_Value::zero: off]
  bool                       printFinalReport_;
  bool                       pushStatisticMessages_; // 1-second interval

  // used to compute data/message throughput
  size_t                     byteCounter_;
  unsigned int               fragmentCounter_;
  unsigned int               controlMessageCounter_;
  unsigned int               messageCounter_;
  unsigned int               sessionMessageCounter_;

  // *protocol statistic*
  STATISTIC_T                messageTypeStatistic_;
};

// include template definition
#include "stream_stat_statistic_report.inl"

#endif
