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
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::Stream_Module_Statistic_WriterTask_T ()
 : inherited ()
 , isInitialized_ (false)
 , resetTimeoutHandler_ (this)
 , resetTimeoutHandlerID_ (-1)
 , localReportingHandler_ (ACTION_REPORT,
                           this,
                           false)
 , localReportingHandlerID_ (-1)
 , reportingInterval_ (0)
 , sendStatisticMessages_ (false)
 , printFinalReport_ (false)
 , lock_ ()
 , sessionData_ (NULL)
 , inboundMessages_ (0)
 , outboundMessages_ (0)
 , sessionMessages_ (0)
 , messageCounter_ (0)
 , lastMessagesPerSecondCount_ (0)
 , inboundBytes_ (0.0F)
 , outboundBytes_ (0.0F)
 , byteCounter_ (0)
 , lastBytesPerSecondCount_ (0)
 , messageTypeStatistic_ ()
 , allocator_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::Stream_Module_Statistic_WriterTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::~Stream_Module_Statistic_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::~Stream_Module_Statistic_WriterTask_T"));

  // clean up
  finiTimers (true);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::initialize (unsigned int reportingInterval_in,
                                                                            bool sendStatisticMessages_in,
                                                                            bool printFinalReport_in,
                                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::initialize"));

  // sanity check(s)
  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("re-initializing...\n")));

    // stop timers
    finiTimers (true);

    reportingInterval_ = 0;
    sendStatisticMessages_ = false;
    printFinalReport_ = false;
    sessionData_ = NULL;
    // reset various counters...
    {
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

      inboundMessages_ = 0;
      outboundMessages_ = 0;
      sessionMessages_ = 0;
      messageCounter_ = 0;
      lastMessagesPerSecondCount_ = 0;

      inboundBytes_ = 0.0F;
      outboundBytes_ = 0.0F;
      byteCounter_ = 0;
      lastBytesPerSecondCount_ = 0;

      messageTypeStatistic_.clear ();
    } // end lock scope
    allocator_ = NULL;

    isInitialized_ = false;
  } // end IF

  reportingInterval_ = reportingInterval_in;
  if (reportingInterval_ &&
      (resetTimeoutHandlerID_ == -1))
  {
    // schedule the second-granularity timer
    ACE_Time_Value one_second (1, 0); // one second interval
    ACE_Event_Handler* event_handler_p = &resetTimeoutHandler_;
    resetTimeoutHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (event_handler_p,              // event handler
                                                                  NULL,                         // ACT
                                                                  COMMON_TIME_NOW + one_second, // first wakeup time
                                                                  one_second);                  // interval
    if (resetTimeoutHandlerID_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(%#T): \"%m\", aborting\n"),
                  &one_second));
      return false;
    } // end IF
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("scheduled second-interval timer (ID: %d)...\n"),
//               resetTimeoutHandlerID_));
  } // end IF
  sendStatisticMessages_ = sendStatisticMessages_in;
  printFinalReport_ = printFinalReport_in;
  allocator_ = allocator_in;

  isInitialized_ = true;

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
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
    inboundMessages_++;
    inboundBytes_ += message_inout->total_length ();
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
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
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
    //inboundMessages_++;
    sessionMessages_++;
    messageCounter_++;
  } // end lock scope

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // *TODO*: remove type inferences
      const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
          message_inout->get ();
      const typename SessionMessageType::SESSION_DATA_T::SESSION_DATA_T& session_data_r =
          session_data_container_r.get ();
      sessionData_ =
          &const_cast<typename SessionMessageType::SESSION_DATA_T::SESSION_DATA_T&> (session_data_r);

      // statistic reporting
      if (reportingInterval_ && (reportingInterval_ != 1))
      {
        // schedule the reporting interval timer
        ACE_Time_Value interval (reportingInterval_, 0);
        ACE_ASSERT (localReportingHandlerID_ == -1);
        ACE_Event_Handler* event_handler_p = &localReportingHandler_;
        localReportingHandlerID_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (event_handler_p,            // event handler
                                                                      NULL,                       // ACT
                                                                      COMMON_TIME_NOW + interval, // first wakeup time
                                                                      interval);                  // interval
        if (localReportingHandlerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(%#T): \"%m\", returning\n"),
                      &interval));
          return;
        } // end IF
        //     ACE_DEBUG ((LM_DEBUG,
        //                 ACE_TEXT ("scheduled (local) reporting timer (ID: %d) for intervals of %u second(s)...\n"),
        //                 localReportingHandlerID_,
        //                 reportingInterval_in));
      } // end IF
      // *NOTE*: even if this doesn't report, it might still be triggered from
      //         outside

      break;
    }
    case STREAM_SESSION_END:
    {
      // stop (reporting) timer(s) --> need to re-initialize after this
      finiTimers (true);

      // session finished --> print overall statistic ?
      if (printFinalReport_)
        finalReport ();

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
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::reset"));

  // happens every second (roughly)
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // remember this result (satisfies an asynchronous API)
    lastMessagesPerSecondCount_ = messageCounter_;
    lastBytesPerSecondCount_ = byteCounter_;

    // reset counters
    messageCounter_ = 0;
    byteCounter_ = 0;

    // update session data
    if (sessionData_)
    {
      ACE_ASSERT (sessionData_->lock);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard_2 (*sessionData_->lock);

      // *TODO*: remove type inferences
      sessionData_->currentStatistic.bytesPerSecond =
        static_cast<float> (lastBytesPerSecondCount_);
      sessionData_->currentStatistic.messagesPerSecond =
        static_cast<float> (lastMessagesPerSecondCount_);
    } // end IF
  } // end lock scope

  if (sendStatisticMessages_ &&
      (reportingInterval_ == 1))
    sendStatistic ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::collect"));

  // *NOTE*: external call; fill the argument with meaningful values
  // *TODO*: the temaplate does not know about StatisticContainerType
  //         --> must be overriden by a (specialized) child class

  // initialize return value(s)
  ACE_OS::memset (&data_out, 0, sizeof (StatisticContainerType));

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // *TODO*: remove type inferences
    data_out.bytes = (inboundBytes_ + outboundBytes_);
    data_out.dataMessages = (inboundMessages_ + outboundMessages_);
//    data_out.droppedMessages = 0;
  } // end lock scope

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::report"));

  if (sendStatisticMessages_ &&
      (reportingInterval_ != 1))
    const_cast<OWN_TYPE_T*> (this)->sendStatistic ();

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %u] RUNTIME STATISTICS ***\n--> Stream Statistics <--\n messages/sec: %u\n messages total [in/out]): %u/%u (data: %.2f%%)\n bytes/sec: %u\n bytes total: %.0f\n--> Cache Statistics <--\n current cache usage [%u messages / %u byte(s) allocated]\n*** RUNTIME STATISTICS ***\\END\n"),
              (sessionData_ ? sessionData_->sessionID
                            : std::numeric_limits<unsigned int>::max ()),
              lastMessagesPerSecondCount_,
              inboundMessages_, outboundMessages_,
              (static_cast<float> (inboundMessages_ + outboundMessages_) /
               static_cast<float> (inboundMessages_ + outboundMessages_ + sessionMessages_) *
               100.0F),
              lastBytesPerSecondCount_,
              (inboundBytes_ + outboundBytes_),
              (allocator_ ? allocator_->cache_size () : 0),
              (allocator_ ? allocator_->cache_depth () : 0)));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::finalReport () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::finalReport"));

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    if ((inboundMessages_ + outboundMessages_))
    {
      // *TODO*: remove type inferences
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("*** [session: %u] SESSION STATISTIC ***\ntotal # data message(s) [in/out]: %u/%u\n --> Protocol Info <--\n"),
                  (sessionData_ ? sessionData_->sessionID
                                : std::numeric_limits<unsigned int>::max ()),
                  inboundMessages_, outboundMessages_));

      std::string protocol_string;
      for (STATISTIC_ITERATOR_T iterator = messageTypeStatistic_.begin ();
           iterator != messageTypeStatistic_.end ();
           iterator++)
        ACE_DEBUG ((LM_INFO,
                    ACE_TEXT ("\"%s\": %u --> %.2f %%\n"),
                    ACE_TEXT (ProtocolMessageType::CommandType2String (iterator->first).c_str ()),
                    iterator->second,
                    static_cast<double> (((iterator->second * 100.0) / (inboundMessages_ + outboundMessages_)))));
    } // end IF
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("------------------------------------------\ntotal byte(s) [in/out]: %.0f/%.0f\nbytes/s: %u\n"),
                inboundBytes_, outboundBytes_,
                lastBytesPerSecondCount_)); // *TODO*: compute average
  } // end lock scope
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::finiTimers (bool cancelAllTimers_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::finiTimers"));

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

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Statistic_WriterTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::sendStatistic ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::sendStatistic"));

  // create session data
  SessionDataContainerType* session_data_container_p = NULL;
  ACE_NEW_NORETURN (session_data_container_p,
                    SessionDataContainerType (const_cast<SessionDataType*> (sessionData_),
                                              false));
  if (!session_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", returning\n")));
    return;
  } // end IF

  // create session message
  SessionMessageType* session_message_p = NULL;
  if (allocator_)
  {
allocate:
    try
    {
      // *IMPORTANT NOTE*: 0 --> session message !
      session_message_p =
        static_cast<SessionMessageType*> (allocator_->malloc (0));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), returning\n")));

      // clean up
      session_data_container_p->decrease ();

      return;
    }

    // keep retrying ?
    if (!session_message_p &&
        !allocator_->block ())
        goto allocate;
  } // end IF
  else
  {
    // *IMPORTANT NOTE*: session message assumes responsibility for
    //                   session_data_container_p
    // *TODO*: remove type inference
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (STREAM_SESSION_STATISTIC,
                                          session_data_container_p,
                                          (sessionData_ ? sessionData_->userData
                                                        : NULL)));
  } // end ELSE
  if (!session_message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", returning\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", returning\n")));

    // clean up
    session_data_container_p->decrease ();

    return;
  } // end IF
  if (allocator_)
  {
    // *IMPORTANT NOTE*: session message assumes responsibility for
    //                   session_data_container_p
    // *TODO*: remove type inference
    session_message_p->initialize (STREAM_SESSION_STATISTIC,
                                   session_data_container_p,
                                   (sessionData_ ? sessionData_->userData
                                                 : NULL));
  } // end IF

  // pass message downstream...
  int result = inherited::put (session_message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put(): \"%m\", returning\n")));

    // clean up
    session_message_p->release ();

    return;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued session message...\n")));
}

// -----------------------------------------------------------------------------

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Statistic_ReaderTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::Stream_Module_Statistic_ReaderTask_T ()
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
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Statistic_ReaderTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::~Stream_Module_Statistic_ReaderTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_ReaderTask_T::~Stream_Module_Statistic_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_Module_Statistic_ReaderTask_T<TaskSynchType,
                                     TimePolicyType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType>::put (ACE_Message_Block* messageBlock_in,
                                                                     ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_ReaderTask_T::put"));

  // pass the message to the sibling
  ACE_Task_Base* task_base_p = inherited::sibling ();
  if (!task_base_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no sibling task: \"%m\", aborting\n")));
    return -1;
  } // end IF
  WRITER_TASK_T* writer_p = dynamic_cast<WRITER_TASK_T*> (task_base_p);
  if (!writer_p)
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
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (writer_p->lock_);

    // update counters...
    writer_p->outboundMessages_++;
    writer_p->outboundBytes_ += messageBlock_in->total_length ();

    writer_p->byteCounter_ += messageBlock_in->total_length ();

    writer_p->messageCounter_++;

    // add message to statistic...
    writer_p->messageTypeStatistic_[message_p->command ()]++;
  } // end lock scope

  return inherited::put (messageBlock_in, timeValue_in);
}
