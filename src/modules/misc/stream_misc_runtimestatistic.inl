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

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_macros.h"
#include "stream_message_base.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::Stream_Module_Statistic_WriterTask_T ()
 : inherited ()
 , lock_ ()
 , sessionData_ (NULL)
 /////////////////////////////////////////
 , initialized_ (false)
 , resetTimeoutHandler_ (this)
 , resetTimeoutHandlerID_ (-1)
 , timerThreadID_ (0)
 , localReportingHandler_ (ACTION_REPORT,
                           this,
                           false)
 , localReportingHandlerID_ (-1)
 , reportingInterval_ (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
 , printFinalReport_ (false)
 , pushStatisticMessages_ (false)
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::~Stream_Module_Statistic_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::~Stream_Module_Statistic_WriterTask_T"));

  finiTimers (true);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

  // clean up
  if (sessionData_)
    sessionData_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
bool
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::initialize"));

  // sanity check(s)
  if (initialized_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("re-initializing...\n")));

    // stop timers
    finiTimers (true);

    reportingInterval_ = ACE_Time_Value::zero;
    printFinalReport_ = false;
    pushStatisticMessages_ = false;
    // reset various counters...

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

    allocator_ = NULL;

    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    initialized_ = false;
  } // end IF
  reportingInterval_ = configuration_in.reportingInterval;
  printFinalReport_ = configuration_in.printFinalReport;
  pushStatisticMessages_ = configuration_in.pushStatisticMessages;
  allocator_ = configuration_in.messageAllocator;

  if ((reportingInterval_ != ACE_Time_Value::zero) ||
      pushStatisticMessages_)
  {
    Common_Timer_Manager_t* timer_manager_p =
        COMMON_TIMERMANAGER_SINGLETON::instance ();
    ACE_ASSERT (timer_manager_p);
    timerThreadID_ = timer_manager_p->thr_id ();
    // schedule the second-granularity timer
    ACE_Time_Value one_second (1, 0); // one-second interval
    ACE_Event_Handler* event_handler_p = &resetTimeoutHandler_;
    resetTimeoutHandlerID_ =
      timer_manager_p->schedule_timer (event_handler_p,              // event handler
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
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("scheduled second-interval timer (ID: %d)...\n"),
//                resetTimeoutHandlerID_));
  } // end IF

  initialized_ = inherited::initialize (configuration_in);

  return initialized_;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
void
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::handleControlMessage"));

  ACE_UNUSED_ARG (message_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
void
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
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

    // add message to statistic
    messageTypeStatistic_[message_inout->command ()]++;
  } // end lock scope
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
void
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (initialized_);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

  // update counters
  // *NOTE*: currently, session messages travel only downstream
  //inboundMessages_++;
  sessionMessages_++;
  //messageCounter_++;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);

      // *TODO*: remove type inferences
      sessionData_ =
          &const_cast<SessionDataContainerType&> (message_inout->get ());
      sessionData_->increase ();

      // statistic reporting
      if (reportingInterval_ != ACE_Time_Value::zero)
      {
        // schedule the reporting interval timer
        ACE_ASSERT (localReportingHandlerID_ == -1);
        ACE_Event_Handler* event_handler_p = &localReportingHandler_;
        localReportingHandlerID_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (event_handler_p,                      // event handler
                                                                      NULL,                                 // ACT
                                                                      COMMON_TIME_NOW + reportingInterval_, // first wakeup time
                                                                      reportingInterval_);                  // interval
        if (localReportingHandlerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(%#T): \"%m\", returning\n"),
                      &reportingInterval_));
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
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      // *NOTE*: in some scenarios, the reset timer generates statistic session
      //         messages (see reset()). In this case, the session data has
      //         already been updated
      //         --> return
      // *TODO*: this will not work if 'this' is asynchronous
      if (ACE_Thread::self () == timerThreadID_)
        break; // done

      // sanity check(s)
      ACE_ASSERT (sessionData_);

      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (sessionData_->get ());

      // *NOTE*: the message contains statistic information (most probably, from
      //         some upstream module, e.g. some hardware capture device driver)
      //         --> aggregate this data
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      const typename SessionDataContainerType::DATA_T& session_data_2 =
        session_data_container_r.get ();

      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard_2 (*session_data_r.lock);
        //ACE_Guard<ACE_SYNCH_MUTEX> aGuard_3 (*session_data_2.lock);

        session_data_r.currentStatistic += session_data_2.currentStatistic;
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // stop (reporting) timer(s) --> need to re-initialize after this
      finiTimers (true);

      // session finished --> print overall statistic ?
      if (printFinalReport_)
        finalReport ();

      // clean up
      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
void
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::reset"));

  // *NOTE*: reset() occurs every second (roughly)

  bool in_session = false;

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // remember this result (support asynchronous API)
    lastBytesPerSecondCount_ = byteCounter_;
    lastMessagesPerSecondCount_ = messageCounter_;

    // reset counters
    messageCounter_ = 0;
    byteCounter_ = 0;

    // update session data ?
    if (!sessionData_)
      goto continue_;

    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_->get ());
    ACE_ASSERT (session_data_r.lock);
    {
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard_2 (*session_data_r.lock);

      // *TODO*: remove type inferences
      session_data_r.currentStatistic.bytes = inboundBytes_;
      session_data_r.currentStatistic.dataMessages = inboundMessages_;
      //session_data_r.currentStatistic.droppedMessages = 0;
      session_data_r.currentStatistic.bytesPerSecond =
          static_cast<float> (lastBytesPerSecondCount_);
      session_data_r.currentStatistic.messagesPerSecond =
          static_cast<float> (lastMessagesPerSecondCount_);
      session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;
    } // end lock scope

    in_session = true;
  } // end lock scope

continue_:
  if (in_session && pushStatisticMessages_)
    if (!putStatisticMessage ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Statistic_WriterTask_T::putStatisticMessage(), continuing\n")));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
bool
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
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
  } // end lock scope

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
void
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::report"));

  int result = -1;
  SessionDataType* session_data_p = NULL;

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

  if (sessionData_)
    session_data_p = &const_cast<SessionDataType&> (sessionData_->get ());

  if (session_data_p)
    if (session_data_p->lock)
    {
      result = session_data_p->lock->acquire ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", returning\n")));
        return;
      } // end IF
    } // end IF

  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %u] RUNTIME STATISTICS ***\n--> Stream Statistics <--\n messages/sec: %u\n messages total [in/out]): %u/%u (data: %.2f%%)\n bytes/sec: %u\n bytes total: %.0f\n--> Cache Statistics <--\n current cache usage [%u messages / %u byte(s) allocated]\n*** RUNTIME STATISTICS ***\\END\n"),
              (session_data_p ? session_data_p->sessionID
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

  if (session_data_p)
    if (session_data_p->lock)
    {
      result = session_data_p->lock->release ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
    } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
void
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::finalReport () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::finalReport"));

  int result = -1;
  SessionDataType* session_data_p = NULL;
  if (sessionData_)
    session_data_p = &const_cast<SessionDataType&> (sessionData_->get ());

  if (session_data_p)
    if (session_data_p->lock)
    {
      result = session_data_p->lock->acquire ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", returning\n")));
        return;
      } // end IF
    } // end IF

  if ((inboundMessages_ + outboundMessages_))
  {
    // *TODO*: remove type inferences
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("*** [session: %u] SESSION STATISTIC ***\ntotal # data message(s) [in/out]: %u/%u\n --> Protocol Info <--\n"),
                (session_data_p ? session_data_p->sessionID
                                : std::numeric_limits<unsigned int>::max ()),
                inboundMessages_, outboundMessages_));

    std::string protocol_string;
    for (STATISTIC_ITERATOR_T iterator = messageTypeStatistic_.begin ();
         iterator != messageTypeStatistic_.end ();
         iterator++)
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("\"%s\": %u --> %.2f %%\n"),
                  ACE_TEXT (DataMessageType::CommandType2String (iterator->first).c_str ()),
                  iterator->second,
                  static_cast<double> (((iterator->second * 100.0) / (inboundMessages_ + outboundMessages_)))));
  } // end IF
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("------------------------------------------\ntotal byte(s) [in/out]: %.0f/%.0f\nbytes/s: %u\n"),
              inboundBytes_, outboundBytes_,
              lastBytesPerSecondCount_)); // *TODO*: compute average

  if (session_data_p)
    if (session_data_p->lock)
    {
      result = session_data_p->lock->release ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
    } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
void
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
bool
Stream_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::putStatisticMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_WriterTask_T::putStatisticMessage"));

  SessionDataContainerType* session_data_container_p = NULL;
  if (sessionData_)
  {
    sessionData_->increase ();
    session_data_container_p = sessionData_;
  } // end IF
  else
  {
    SessionDataType* session_data_p = NULL;
    ACE_NEW_NORETURN (session_data_p,
                      SessionDataType ());
    if (!session_data_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionDataType: \"%m\", aborting\n")));
      return false;
    } // end IF

    // *IMPORTANT NOTE*: fire-and-forget session_data_p
    ACE_NEW_NORETURN (session_data_container_p,
                      SessionDataContainerType (session_data_p));
    if (!session_data_container_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end ELSE
  ACE_ASSERT (session_data_container_p);

  SessionDataType* session_data_p =
    &const_cast<SessionDataType&> (session_data_container_p->get ());

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
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));

      // clean up
      session_data_container_p->decrease ();

      return false;
    }

    // keep retrying ?
    if (!session_message_p &&
        !allocator_->block ())
      goto allocate;
  } // end IF
  else
  {
    // *TODO*: remove type inference
    // *IMPORTANT NOTE*: fire-and-forget session_data_container_p
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (STREAM_SESSION_MESSAGE_STATISTIC,
                                          session_data_container_p,
                                          (session_data_p ? session_data_p->userData
                                                          : NULL)));
  } // end ELSE
  if (!session_message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));

    // clean up
    session_data_container_p->decrease ();

    return false;
  } // end IF
  if (allocator_)
  {
    // *TODO*: remove type inference
    // *IMPORTANT NOTE*: fire-and-forget session_data_container_p
    session_message_p->initialize (STREAM_SESSION_MESSAGE_STATISTIC,
                                   session_data_container_p,
                                   (session_data_p ? session_data_p->userData
                                                   : NULL));
  } // end IF

  // pass message downstream...
  int result = inherited::put (session_message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put(): \"%m\", aborting\n")));

    // clean up
    session_message_p->release ();

    return false;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued session message...\n")));

  return true;
}

// -----------------------------------------------------------------------------

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
Stream_Module_Statistic_ReaderTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::Stream_Module_Statistic_ReaderTask_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_ReaderTask_T::Stream_Module_Statistic_ReaderTask_T"));

  inherited::flags_ |= ACE_Task_Flags::ACE_READER;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
Stream_Module_Statistic_ReaderTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
                                     SessionDataContainerType>::~Stream_Module_Statistic_ReaderTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Statistic_ReaderTask_T::~Stream_Module_Statistic_ReaderTask_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
int
Stream_Module_Statistic_ReaderTask_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     ProtocolCommandType,
                                     StatisticContainerType,
                                     SessionDataType,          // session data
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
  DataMessageType* message_p =
    dynamic_cast<DataMessageType*> (messageBlock_in);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<DataMessageType>(%@), aborting\n"),
                messageBlock_in));
    return -1;
  } // end IF

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (writer_p->lock_);

    // update counters
    writer_p->outboundMessages_++;
    writer_p->outboundBytes_ += messageBlock_in->total_length ();

    writer_p->byteCounter_ += messageBlock_in->total_length ();

    writer_p->messageCounter_++;

    // add message to statistic...
    writer_p->messageTypeStatistic_[message_p->command ()]++;
  } // end lock scope

  return inherited::put_next (messageBlock_in, timeValue_in);
}
