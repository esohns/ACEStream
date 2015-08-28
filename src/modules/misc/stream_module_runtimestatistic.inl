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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Time_Value.h"

#include "common_timer_manager_common.h"

#include "stream_iallocator.h"
#include "stream_macros.h"
#include "stream_message_base.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::Stream_Module_Statistic_WriterTask_T ()
 : inherited ()
 , isInitialized_ (false)
 , resetTimeoutHandler_ (this)
 , resetTimeoutHandlerID_ (-1)
 , localReportingHandler_ (ACTION_REPORT,
                           this,
                           false)
 , localReportingHandlerID_ (-1)
 , reportingInterval_ (0)
 , sessionID_ (0)
 , numInboundMessages_ (0)
 , numOutboundMessages_ (0)
 , numSessionMessages_ (0)
 , messageCounter_ (0)
 , lastMessagesPerSecondCount_ (0)
 , numInboundBytes_ (0.0F)
 , numOutboundBytes_ (0.0F)
 , byteCounter_ (0)
 , lastBytesPerSecondCount_ (0)
// , messageTypeStatistics_.clear()
 , allocator_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::Stream_Module_Statistic_WriterTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::~Stream_Module_Statistic_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::~Stream_Module_Statistic_WriterTask_T"));

  // clean up
  fini_timers (true);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
bool
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::initialize (unsigned int reportingInterval_in,
                                                                          bool printFinalReport_in,
                                                                          const Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::initialize"));

  // sanity check(s)
  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    // stop timers
    fini_timers (true);

    reportingInterval_ = 0;
    printFinalReport_ = false;
    sessionID_ = 0;
    // reset various counters...
    {
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

      numInboundMessages_ = 0;
      numOutboundMessages_ = 0;
      numSessionMessages_ = 0;
      messageCounter_ = 0;
      lastMessagesPerSecondCount_ = 0;

      numInboundBytes_ = 0.0F;
      numOutboundBytes_ = 0.0F;
      byteCounter_ = 0;
      lastBytesPerSecondCount_ = 0;

      messageTypeStatistic_.clear ();
    } // end lock scope
    allocator_ = NULL;

    isInitialized_ = false;
  } // end IF

  reportingInterval_ = reportingInterval_in;
  if (reportingInterval_)
  {
    // schedule the second-granularity timer
    ACE_Time_Value interval (1, 0); // one second interval
    ACE_Event_Handler* event_handler_p = &resetTimeoutHandler_;
    resetTimeoutHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (event_handler_p,                  // event handler
                                                                  NULL,                             // ACT
                                                                  COMMON_TIME_NOW + interval, // first wakeup time
                                                                  interval);                        // interval
    if (resetTimeoutHandlerID_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
      return false;
    } // end IF
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("scheduled second-interval timer (ID: %d)...\n"),
//               resetTimeoutHandlerID_));
  } // end IF
  printFinalReport_ = printFinalReport_in;
  allocator_ = allocator_in;
//   // sanity check(s)
//   if (!allocator_)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("invalid argument (was NULL), aborting\n")));
//
//     return false;
//   } // end IF

  isInitialized_ = true;

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // update counters...
    numInboundMessages_++;
    numInboundBytes_ += message_inout->total_length ();
    byteCounter_ += message_inout->total_length ();

    messageCounter_++;

    // add message to statistic...
    messageTypeStatistic_[message_inout->command ()]++;
  } // end lock scope
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // update counters...
    // *NOTE*: currently, session messages travel only downstream...
    //numInboundMessages_++;
    numSessionMessages_++;
    //messageCounter_++;
  } // end lock scope

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // retain session ID for reporting...
      // *TODO*: remove type inferences
      const typename SessionMessageType::SESSION_DATA_TYPE& session_data_container_r =
          message_inout->get ();
      const typename SessionMessageType::SESSION_DATA_TYPE::SESSION_DATA_TYPE* session_data_p =
          session_data_container_r.getData ();
      ACE_ASSERT (session_data_p);
      sessionID_ = session_data_p->sessionID;

      // statistics reporting
      if (reportingInterval_)
      {
        // schedule the reporting interval timer
        ACE_Time_Value interval (reportingInterval_, 0);
        ACE_ASSERT (localReportingHandlerID_ == -1);
        ACE_Event_Handler* event_handler_p = &localReportingHandler_;
        localReportingHandlerID_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (event_handler_p,                  // event handler
                                                                      NULL,                             // act
                                                                      COMMON_TIME_NOW + interval, // first wakeup time
                                                                      interval);                        // interval
        if (localReportingHandlerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", returning\n")));
          return;
        } // end IF
        //     ACE_DEBUG ((LM_DEBUG,
        //                 ACE_TEXT ("scheduled (local) reporting timer (ID: %d) for intervals of %u second(s)...\n"),
        //                 localReportingHandlerID_,
        //                 reportingInterval_in));
      } // end IF
      else
      {
        // *NOTE*: even if this doesn't report, it might still be triggered from outside...
        //     ACE_DEBUG ((LM_DEBUG,
        //                 ACE_TEXT ("(local) statistics reporting has been disabled...\n")));
      } // end IF

      break;
    }
    case STREAM_SESSION_END:
    {
      // stop reporting timer
      fini_timers (false);

      // session finished --> print overall statistics ?
      if (printFinalReport_)
        final_report ();

      break;
    }
    case STREAM_SESSION_STATISTIC:
    {
//       // *NOTE*: protect access to statistics data
//       // from asynchronous API calls (as well as local reporting)...
//       {
//         ACE_Guard<ACE_Thread_Mutex> aGuard (statsLock_);
//
//         currentStats_ = message_inout->getConfiguration ()->getStats ();
//
//         // remember previous timestamp (so we can satisfy our asynchronous API)...
//         lastStatsTimestamp_ = currentStatsTimestamp_;
//
//         currentStatsTimestamp_ = message_inout->getConfiguration ()->getStatGenerationTime ();
//       } // end lock scope

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::reset"));

  // this should happen every second (roughly)...
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // remember this result (satisfies an asynchronous API)...
    lastMessagesPerSecondCount_ = messageCounter_;
    lastBytesPerSecondCount_ = byteCounter_;

    // reset counters
    messageCounter_ = 0;
    byteCounter_ = 0;
  } // end lock scope
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
bool
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::collect"));

  // *NOTE*: external call, fill the argument with meaningful values
  // *TODO*: the temaplate does not know about StatisticContainerType
  //         --> must be overriden by a (specialized) child class

  // initialize return value(s)
  ACE_OS::memset (&data_out, 0, sizeof (StatisticContainerType));

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    data_out.bytes = (numInboundBytes_ + numOutboundBytes_);
    data_out.dataMessages = (numInboundMessages_ + numOutboundMessages_);
//    data_out.droppedMessages = 0;
  } // end lock scope

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::report"));

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %u] RUNTIME STATISTICS ***\n--> Stream Statistics <--\n messages/sec: %u\n messages total [in/out]): %u/%u (data: %.2f%%)\n bytes/sec: %u\n bytes total: %.0f\n--> Cache Statistics <--\n current cache usage [%u messages / %u byte(s) allocated]\n*** RUNTIME STATISTICS ***\\END\n"),
              sessionID_,
              lastMessagesPerSecondCount_,
              numInboundMessages_, numOutboundMessages_,
              (static_cast<float> (numInboundMessages_ + numOutboundMessages_) /
               static_cast<float> (numInboundMessages_ + numOutboundMessages_ + numSessionMessages_) *
               100.0F),
              lastBytesPerSecondCount_,
              (numInboundBytes_ + numOutboundBytes_),
              (allocator_ ? allocator_->cache_size () : 0),
              (allocator_ ? allocator_->cache_depth () : 0)));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::final_report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::final_report"));

  {
    // synchronize access to statistics data
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    if ((numInboundMessages_ + numOutboundMessages_))
    {
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("*** [session: %u] SESSION STATISTIC ***\ntotal # data message(s) [in/out]: %u/%u\n --> Protocol Info <--\n"),
                  sessionID_,
                  numInboundMessages_, numOutboundMessages_));

      std::string protocol_string;
      for (MESSAGE_STATISTICITERATOR_T iterator = messageTypeStatistic_.begin ();
           iterator != messageTypeStatistic_.end ();
           iterator++)
        ACE_DEBUG ((LM_INFO,
                    ACE_TEXT ("\"%s\": %u --> %.2f %%\n"),
                    ACE_TEXT (ProtocolMessageType::CommandType2String (iterator->first).c_str ()),
                    iterator->second,
                    static_cast<double> (((iterator->second * 100.0) / (numInboundMessages_ + numOutboundMessages_)))));
    } // end IF
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("------------------------------------------\ntotal byte(s) [in/out]: %.0f/%.0f\nbytes/s: %u\n"),
                numInboundBytes_, numOutboundBytes_,
                lastBytesPerSecondCount_));
  } // end lock scope
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::fini_timers (bool cancelAllTimers_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::fini_timers"));

  int result = -1;

  const void* act_p = NULL;
  if (cancelAllTimers_in)
  {
    if (resetTimeoutHandlerID_ != -1)
    {
      result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (resetTimeoutHandlerID_,
                                                                  &act_p);
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    resetTimeoutHandlerID_));
      resetTimeoutHandlerID_ = -1;
    } // end IF
  } // end IF

  if (localReportingHandlerID_ != -1)
  {
    act_p = NULL;
    result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (localReportingHandlerID_,
                                                                &act_p);
    if (result <= 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  localReportingHandlerID_));
    localReportingHandlerID_ = -1;
  } // end IF
}

// -----------------------------------------------------------------------------

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
Stream_Module_Statistic_ReaderTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::Stream_Module_Statistic_ReaderTask_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_ReaderTask_T::Stream_Module_Statistic_ReaderTask_T"));

  inherited::flags_ |= ACE_Task_Flags::ACE_READER;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
Stream_Module_Statistic_ReaderTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::~Stream_Module_Statistic_ReaderTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_ReaderTask_T::~Stream_Module_Statistic_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
int
Stream_Module_Statistic_ReaderTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType>::put (ACE_Message_Block* messageBlock_in,
                                                                   ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_ReaderTask_T::put"));

  // pass the message to the sibling
  ACE_Task_Base* sibling_base_p = inherited::sibling ();
  if (!sibling_base_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no sibling task: \"%m\", aborting\n")));
    return -1;
  } // end IF
  WRITER_TASK_T* sibling_p =
    dynamic_cast<WRITER_TASK_T*> (sibling_base_p);
  if (!sibling_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_Module_Statistic_WriterTask_t>: \"%m\", aborting\n")));
    return -1;
  } // end IF
  ProtocolMessageType* message_p =
    dynamic_cast<ProtocolMessageType*> (messageBlock_in);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<ProtocolMessageType>(%@), aborting\n"),
                messageBlock_in));
    return -1;
  } // end IF

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (sibling_p->lock_);

    // update counters...
    sibling_p->numOutboundMessages_++;
    sibling_p->numOutboundBytes_ += messageBlock_in->total_length ();

    sibling_p->byteCounter_ += messageBlock_in->total_length ();

    sibling_p->messageCounter_++;

    // add message to statistic...
    sibling_p->messageTypeStatistic_[message_p->command ()]++;
  } // end lock scope

  return inherited::put (messageBlock_in, timeValue_in);
}
