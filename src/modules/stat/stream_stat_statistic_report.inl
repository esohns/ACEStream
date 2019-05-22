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

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_macros.h"

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
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                              SessionDataContainerType>::Stream_Statistic_StatisticReport_WriterTask_T (ISTREAM_T* stream_in)
#else
                                              SessionDataContainerType>::Stream_Statistic_StatisticReport_WriterTask_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inboundBytes_ (0.0F)
 , outboundBytes_ (0.0F)
 , inboundMessages_ (0)
 , outboundMessages_ (0)
 , lastBytesPerSecondCount_ (0)
 , lastDataMessagesPerSecondCount_ (0)
 , sessionMessages_ (0)
 , controlMessages_ (0)
 , outboundControlMessages_ (0)
 /////////////////////////////////////////
 , inbound_ (true)
 , resetTimeoutHandler_ (this)
 , resetTimeoutHandlerId_ (-1)
 , localReportingHandler_ (COMMON_STATISTIC_ACTION_REPORT,
                           this,
                           false)
 , localReportingHandlerId_ (-1)
 , reportingInterval_ (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
 , printFinalReport_ (false)
 , pushStatisticMessages_ (false)
 , byteCounter_ (0)
 , fragmentCounter_ (0)
 , controlMessageCounter_ (0)
 , messageCounter_ (0)
 , sessionMessageCounter_ (0)
 , messageTypeStatistic_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::Stream_Statistic_StatisticReport_WriterTask_T"));

}

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
bool
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                                    Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::initialize"));

  // sanity check(s)
  if (inherited::isInitialized_)
  {
    // stop timers
    finiTimers (true);

    reportingInterval_ = ACE_Time_Value::zero;
    printFinalReport_ = false;
    pushStatisticMessages_ = false;

    // reset various counters
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
      inboundBytes_ = 0.0F;
      outboundBytes_ = 0.0F;
      inboundMessages_ = 0;
      outboundMessages_ = 0;
      lastBytesPerSecondCount_ = 0;
      lastDataMessagesPerSecondCount_ = 0;
      sessionMessages_ = 0;
      controlMessages_ = 0;
      outboundControlMessages_ = 0;

      inbound_ = true;

      byteCounter_ = 0;
      fragmentCounter_ = 0;
      controlMessageCounter_ = 0;
      messageCounter_ = 0;
      sessionMessageCounter_ = 0;

      messageTypeStatistic_.clear ();
    } // end lock scope
  } // end IF
  reportingInterval_ = configuration_in.reportingInterval;
  printFinalReport_ = configuration_in.printFinalReport;
  pushStatisticMessages_ = configuration_in.pushStatisticMessages;

  // *NOTE*: if this is an 'outbound' stream, any 'inbound' data (!) will
  //         eventually turn around and travel back upstream for dispatch
  //         --> account for it only once
  inbound_ = configuration_in.inbound;

  if ((reportingInterval_ != ACE_Time_Value::zero) ||
      pushStatisticMessages_)
  {
    // schedule the second-granularity timer
    typename TimerManagerType::INTERFACE_T* itimer_manager_p =
        (configuration_in.timerManager ? configuration_in.timerManager
                                       : TIMER_MANAGER_SINGLETON_T::instance ());
    ACE_ASSERT (itimer_manager_p);
    ACE_Time_Value one_second (1, 0); // one-second interval
    resetTimeoutHandlerId_ =
      itimer_manager_p->schedule_timer (&resetTimeoutHandler_,        // event handler handle
                                        NULL,                         // asynchronous completion token
                                        COMMON_TIME_NOW + one_second, // first wakeup time
                                        one_second);                  // interval
    if (unlikely (resetTimeoutHandlerId_ == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  &one_second));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: scheduled second-interval timer (id: %d)\n"),
                inherited::mod_->name (),
                resetTimeoutHandlerId_));
#endif // _DEBUG
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

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
void
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::handleControlMessage (ControlMessageType& controlMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::handleControlMessage"));

  ACE_UNUSED_ARG (controlMessage_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    // update counters
    ++controlMessages_;
    ++inboundMessages_;

    ++messageCounter_;
    ++controlMessageCounter_;
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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::handleDataMessage"));

  ACE_ASSERT (message_inout);
  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    inboundBytes_ += message_inout->total_length ();
    byteCounter_ += message_inout->total_length ();

    ++inboundMessages_;
    for (ACE_Message_Block* message_block_p = message_inout;
         message_block_p;
         message_block_p = message_block_p->cont ())
      ++fragmentCounter_;
    ++messageCounter_;

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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    ++sessionMessages_;
    ++inboundMessages_;

    ++messageCounter_;
    ++sessionMessageCounter_;
  } // end lock scope

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // statistic reporting
      if (reportingInterval_ != ACE_Time_Value::zero)
      {
        // schedule the reporting interval timer
        ACE_ASSERT (inherited::configuration_);
        ACE_ASSERT (localReportingHandlerId_ == -1);

        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        localReportingHandlerId_ =
          itimer_manager_p->schedule_timer (&localReportingHandler_,              // event handler handle
                                            NULL,                                 // asynchronous completion token
                                            COMMON_TIME_NOW + reportingInterval_, // first wakeup time
                                            reportingInterval_);                  // interval
        if (unlikely (localReportingHandlerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      &reportingInterval_));
          goto error;
        } // end IF
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled (local) reporting timer (id: %d, interval: %#T)\n"),
                    inherited::mod_->name (),
                    localReportingHandlerId_,
                    &reportingInterval_));
#endif // _DEBUG
      } // end IF
      // *NOTE*: even if 'this' doesn't report, it might still be triggered from
      //         outside

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      // *NOTE*: the message contains statistic information from some upstream
      //         module (e.g. some hardware capture device driver)
      //         --> aggregate this data so that any new (session) messages
      //             generated by this module carry up-to-date information
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      const SessionDataContainerType& session_data_container_r =
        message_inout->getR ();
      const typename SessionDataContainerType::DATA_T& session_data_2 =
        session_data_container_r.getR ();

      // sanity check(s)
      ACE_ASSERT (session_data_r.lock);
      ACE_ASSERT (session_data_2.lock);
      // upstream has been linked, session data is already synchronized
      if (&session_data_r == &session_data_2)
        break;

      int result = -1;
      bool release_lock = false;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        if (session_data_r.lock != session_data_2.lock)
        {
          result = session_data_2.lock->acquire ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          release_lock = true;
        } // end IF

        // *NOTE*: the idea is to 'effectively merge' the statistic data
        // *TODO*: it may be better to just copy the upstream data instead
        session_data_r.statistic += session_data_2.statistic;

        if (release_lock)
        {
          result = session_data_2.lock->release ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
        } // end IF
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // stop (reporting) timer(s) (--> (re-)initialize())
      finiTimers (true);

      if (pushStatisticMessages_)
      {
        if (!inherited::sessionData_)
          goto continue_;

        typename SessionDataContainerType::DATA_T& session_data_r =
          const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
        ACE_ASSERT (session_data_r.lock);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          // *TODO*: remove type inferences
          session_data_r.statistic.bytes = inboundBytes_ + outboundBytes_;
          session_data_r.statistic.dataMessages =
              (inboundMessages_ - sessionMessages_ - (controlMessages_ - outboundControlMessages_)) +
              (outboundMessages_ - outboundControlMessages_);
          // *NOTE*: if this is an 'outbound' stream, which means that the data (!)
          //         will eventually turn around and travel back upstream for
          //         dispatch, account for it only once
          if (!inherited::configuration_->inbound)
          {
            session_data_r.statistic.bytes -= outboundBytes_;
            session_data_r.statistic.dataMessages -=
              (outboundMessages_ - outboundControlMessages_);
          } // end IF
          //session_data_r.statistic.droppedMessages = 0;
          session_data_r.statistic.bytesPerSecond = 0.0;
          session_data_r.statistic.messagesPerSecond = 0.0;
          session_data_r.statistic.timeStamp = COMMON_TIME_NOW;
        } // end lock scope

        if (unlikely (!putStatisticMessage ()))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Statistic_StatisticReport_WriterTask_T::putStatisticMessage(), continuing\n"),
                      inherited::mod_->name ()));
      } // end IF

continue_:
      // session finished --> print overall statistic ?
      if (printFinalReport_)
        finalReport ();

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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::reset"));

  // *NOTE*: reset() occurs every second

  bool in_session = false;

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    // remember this result (support asynchronous API)
    lastBytesPerSecondCount_ = byteCounter_;
    lastDataMessagesPerSecondCount_ = messageCounter_ - sessionMessageCounter_;

    // reset counters
    byteCounter_ = 0;
    fragmentCounter_ = 0;
    controlMessageCounter_ = 0;
    messageCounter_ = 0;
    sessionMessageCounter_ = 0;

    // update session data ?
    if (!inherited::sessionData_)
      goto continue_;

    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
    ACE_ASSERT (session_data_r.lock);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, *session_data_r.lock);
      // *TODO*: remove type inferences
      session_data_r.statistic.bytes = inboundBytes_ + outboundBytes_;
      session_data_r.statistic.dataMessages =
          (inboundMessages_ - sessionMessages_ - (controlMessages_ - outboundControlMessages_)) +
          (outboundMessages_ - outboundControlMessages_);
      // *NOTE*: if this is an 'outbound' stream, which means that the data (!)
      //         will eventually turn around and travel back upstream for
      //         dispatch, account for it only once
      if (!inbound_)
      {
        session_data_r.statistic.bytes -= outboundBytes_;
        session_data_r.statistic.dataMessages -=
          (outboundMessages_ - outboundControlMessages_);
      } // end IF
      //session_data_r.statistic.droppedMessages = 0;
      session_data_r.statistic.bytesPerSecond =
          static_cast<float> (lastBytesPerSecondCount_);
      session_data_r.statistic.messagesPerSecond =
          static_cast<float> (lastDataMessagesPerSecondCount_);
      session_data_r.statistic.timeStamp = COMMON_TIME_NOW;
    } // end lock scope

    in_session = true;
  } // end lock scope

continue_:
  if (pushStatisticMessages_ &&
      in_session )
    if (unlikely (!putStatisticMessage ()))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Statistic_StatisticReport_WriterTask_T::putStatisticMessage(), continuing\n"),
                  inherited::mod_->name ()));
}

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
bool
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::collect"));

  // *NOTE*: external call; fill the argument with meaningful values
  // *TODO*: the temaplate must not know about StatisticContainerType
  //         --> should be overriden by derived classes

  // initialize return value(s)
  ACE_OS::memset (&data_out, 0, sizeof (StatisticContainerType));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
    // *TODO*: remove type inferences
    data_out.bytes = inboundBytes_ + outboundBytes_;
    data_out.dataMessages =
      (inboundMessages_ - sessionMessages_ - (controlMessages_ - outboundControlMessages_)) +
      (outboundMessages_ - outboundControlMessages_);
    // *NOTE*: if this is an 'outbound' stream, which means that the data (!)
    //         will eventually turn around and travel back upstream for
    //         dispatch
    //         --> account for it only once
    //if (!inherited::configuration_->inbound)
    //{
    //  data_out.bytes -= outboundBytes_;
    //  data_out.dataMessages -= (outboundMessages_ - outboundControlMessages_);
    //} // end IF
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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::report"));

  int result = -1;
  SessionDataType* session_data_p = NULL;

  ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);

  if (inherited::sessionData_)
    session_data_p =
        &const_cast<SessionDataType&> (inherited::sessionData_->getR ());

  if (session_data_p)
    if (session_data_p->lock)
    {
      result = session_data_p->lock->acquire ();
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
    } // end IF

  // *NOTE*: if this is an 'outbound' stream, which means that the data (!)
  //         will eventually turn around and travel back upstream for
  //         dispatch
  //         --> account for it only once
  unsigned int data_messages =
    (inboundMessages_ - sessionMessages_ - (controlMessages_ - outboundControlMessages_)) +
    (outboundMessages_ - outboundControlMessages_);
  unsigned int total_messages = inboundMessages_ + outboundMessages_;
  //if (!inherited::configuration_->inbound)
  //{
  //  data_messages -= (outboundMessages_ - outboundControlMessages_);
  //  total_messages -= outboundMessages_;
  //} // end IF
  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %d] STATISTIC ***\n--> Stream Statistic <--\n\tmessages/sec: %u\n\tmessages total [in/out]): %u/%u (data: %.2f%%)\n\tbytes/sec: %u\n\tbytes total: %.0f\n--> Cache Statistics <--\n\tcurrent cache usage [%u messages / %u byte(s) allocated]\n*** RUNTIME STATISTICS ***\\END\n"),
              (session_data_p ? static_cast<int> (session_data_p->sessionId) : -1),
              lastDataMessagesPerSecondCount_, inboundMessages_, outboundMessages_,
              (static_cast<float> (data_messages) / static_cast<float> (total_messages) * 100.0F),
              lastBytesPerSecondCount_, inboundBytes_ + outboundBytes_,
              (inherited::allocator_ ? inherited::allocator_->cache_size () : 0),
              (inherited::allocator_ ? inherited::allocator_->cache_depth () : 0)));

  if (session_data_p)
    if (session_data_p->lock)
    {
      result = session_data_p->lock->release ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::finalReport () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::finalReport"));

  int result = -1;
  SessionDataType* session_data_p = NULL;
  if (likely (inherited::sessionData_))
    session_data_p =
        &const_cast<SessionDataType&> (inherited::sessionData_->getR ());

  if (likely (session_data_p))
    if (session_data_p->lock)
    {
      result = session_data_p->lock->acquire ();
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
    } // end IF

  if (likely (inboundMessages_ + outboundMessages_))
  {
    // *TODO*: remove type inferences
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("*** [session: %d] STATISTIC ***\n\ttotal # data message(s) [in/out]: %u/%u\n --> Protocol Info <--\n"),
                (session_data_p ? static_cast<int> (session_data_p->sessionId) : -1),
                inboundMessages_ - sessionMessages_ - (controlMessages_ - outboundControlMessages_),
                outboundMessages_ - outboundControlMessages_));

    // *NOTE*: if this is an 'outbound' stream, which means that the data (!)
    //         will eventually turn around and travel back upstream for
    //         dispatch - account for it only once
    unsigned int data_messages =
        (inboundMessages_ - sessionMessages_ - (controlMessages_ - outboundControlMessages_)) +
        (outboundMessages_ - outboundControlMessages_);
    //if (!inherited::configuration_->inbound)
    //  data_messages -= (outboundMessages_ - outboundControlMessages_);
    for (STATISTIC_ITERATOR_T iterator = messageTypeStatistic_.begin ();
         iterator != messageTypeStatistic_.end ();
         iterator++)
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("\t\"%s\": %u --> %.2f %%\n"),
                  ACE_TEXT (DataMessageType::CommandTypeToString (iterator->first).c_str ()),
                  iterator->second,
                  (static_cast<float> (iterator->second) * 100.0F) /
                  static_cast<float> (data_messages)));
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("------------------------------------------\n\ttotal byte(s) [in/out]: %.0f/%.0f\n\tbytes/s: %u\n"),
                inboundBytes_, outboundBytes_,
                lastBytesPerSecondCount_)); // *TODO*: compute average
  } // end IF

  if (likely (session_data_p))
    if (session_data_p->lock)
    {
      result = session_data_p->lock->release ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::finiTimers (bool cancelAllTimers_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::finiTimers"));

  int result = -1;
  const void* act_p = NULL;
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
      (inherited::configuration_ ? (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                                            : TIMER_MANAGER_SINGLETON_T::instance ())
                                 : TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  if (cancelAllTimers_in)
  {
    if (resetTimeoutHandlerId_ != -1)
    {
      result = itimer_manager_p->cancel_timer (resetTimeoutHandlerId_,
                                               &act_p);
      if (unlikely (result <= 0))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    resetTimeoutHandlerId_));
      resetTimeoutHandlerId_ = -1;
    } // end IF
  } // end IF

  if (localReportingHandlerId_ != -1)
  {
    act_p = NULL;
    result = itimer_manager_p->cancel_timer (localReportingHandlerId_,
                                             &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  localReportingHandlerId_));
    localReportingHandlerId_ = -1;
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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::putStatisticMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::putStatisticMessage"));

  SessionDataContainerType* session_data_container_p = NULL;
  if (inherited::sessionData_)
  {
    inherited::sessionData_->increase ();
    session_data_container_p = inherited::sessionData_;
  } // end IF
  else
  {
    SessionDataType* session_data_p = NULL;
    ACE_NEW_NORETURN (session_data_p,
                      SessionDataType ());
    if (unlikely (!session_data_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF

    // *IMPORTANT NOTE*: fire-and-forget session_data_p
    ACE_NEW_NORETURN (session_data_container_p,
                      SessionDataContainerType (session_data_p));
    if (unlikely (!session_data_container_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
  } // end ELSE
  ACE_ASSERT (session_data_container_p);

  const SessionDataType& session_data_r = session_data_container_p->getR ();

  // create session message
  SessionMessageType* session_message_p = NULL;
  if (inherited::allocator_)
  {
allocate:
    try {
      // *IMPORTANT NOTE*: 0 --> session message !
      session_message_p =
        static_cast<SessionMessageType*> (inherited::allocator_->malloc (0));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(0), aborting\n"),
                  inherited::mod_->name ()));
      session_data_container_p->decrease (); session_data_container_p = NULL;
      return false;
    }

    // keep retrying ?
    if (!session_message_p &&
        !inherited::allocator_->block ())
      goto allocate;
  } // end IF
  else
  {
    // *TODO*: remove type inference
    // *IMPORTANT NOTE*: fire-and-forget session_data_container_p
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (session_data_r.sessionId,
                                          STREAM_SESSION_MESSAGE_STATISTIC,
                                          session_data_container_p,
                                          session_data_r.userData));
  } // end ELSE
  if (unlikely (!session_message_p))
  {
    if (inherited::allocator_)
    {
      if (inherited::allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate SessionMessageType: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate SessionMessageType: \"%m\", aborting\n"),
                  inherited::mod_->name ()));

    // clean up
    session_data_container_p->decrease ();

    return false;
  } // end IF
  if (inherited::allocator_)
  {
    // *TODO*: remove type inference
    // *IMPORTANT NOTE*: fire-and-forget session_data_container_p
    session_message_p->initialize (session_data_r.sessionId,
                                   STREAM_SESSION_MESSAGE_STATISTIC,
                                   session_data_container_p,
                                   session_data_r.userData);
  } // end IF

  // pass message downstream
  int result = inherited::put_next (session_message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    session_message_p->release (); session_message_p = NULL;
    return false;
  } // end IF

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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Statistic_StatisticReport_ReaderTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::Stream_Statistic_StatisticReport_ReaderTask_T (ISTREAM_T* stream_in)
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_ReaderTask_T::Stream_Statistic_StatisticReport_ReaderTask_T"));

  ACE_UNUSED_ARG (stream_in);

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
          typename TimerManagerType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_Statistic_StatisticReport_ReaderTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              SessionDataType,
                                              SessionDataContainerType>::put (ACE_Message_Block* messageBlock_in,
                                                                              ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_ReaderTask_T::put"));

  ACE_Task_Base* task_base_p = inherited::sibling ();
  ACE_ASSERT (task_base_p);
  WRITER_TASK_T* writer_p = dynamic_cast<WRITER_TASK_T*> (task_base_p);
  if (unlikely (!writer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T>: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_DATA:
    case ACE_Message_Block::MB_PROTO:
    {
      DataMessageType* message_p =
        dynamic_cast<DataMessageType*> (messageBlock_in);
      if (unlikely (!message_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<DataMessageType>(0x%@), aborting\n"),
                    inherited::mod_->name (),
                    messageBlock_in));
        return -1;
      } // end IF

      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, writer_p->lock_, -1);
        // update counters
        if (!writer_p->inbound_)
        {
          writer_p->outboundBytes_ = writer_p->inboundBytes_;
          writer_p->outboundMessages_ =
            writer_p->inboundMessages_ + writer_p->outboundControlMessages_;
          goto continue_;
        } // end IF

        writer_p->outboundBytes_ += messageBlock_in->total_length ();
        writer_p->outboundMessages_++;

        writer_p->byteCounter_ += messageBlock_in->total_length ();
        // *TODO*: count fragment(s)
        writer_p->messageCounter_++;
        // add message to statistic
        writer_p->messageTypeStatistic_[message_p->command ()]++;
      } // end lock scope

      break;
    }
    case ACE_Message_Block::MB_BREAK:
    case ACE_Message_Block::MB_FLUSH:
    case ACE_Message_Block::MB_HANGUP:
    case ACE_Message_Block::MB_NORMAL: // undifferentiated
    case STREAM_CONTROL_CONNECT:
    case STREAM_CONTROL_LINK:
    case STREAM_CONTROL_STEP:
    {
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, writer_p->lock_, -1);
        // update counters
        writer_p->controlMessages_++;
        writer_p->outboundMessages_++;
        writer_p->outboundControlMessages_++;

        writer_p->messageCounter_++;
        writer_p->controlMessageCounter_++;
      } // end lock scope

      break;
    }
    default:
      break;
  } // end SWITCH

continue_:
  return inherited::put_next (messageBlock_in, timeValue_in);
}
