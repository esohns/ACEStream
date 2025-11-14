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

#include "common_time_common.h"

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
          typename UserDataType>
Stream_Statistic_StatisticReport_WriterTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              UserDataType>::Stream_Statistic_StatisticReport_WriterTask_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inboundBytes_ (0)
 , outboundBytes_ (0)
 , inboundMessages_ (0)
 , outboundMessages_ (0)
 , lastBytesPerSecondCount_ (0)
 , lastDataMessagesPerSecondCount_ (0)
 , controlMessages_ (0)
 , sessionMessages_ (0)
 /////////////////////////////////////////
 , localReportingHandler_ (COMMON_STATISTIC_ACTION_REPORT,
                           this,
                           false)
 , localReportingHandlerId_ (-1)
 , reportingInterval_ (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S, 0)
 , printFinalReport_ (false)
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
          typename UserDataType>
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
                                              UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::initialize"));

  // sanity check(s)
  if (inherited::isInitialized_)
  {
    // stop timers
    finiTimer ();

    reportingInterval_ = ACE_Time_Value::zero;
    printFinalReport_ = false;

    // reset various counters
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
      inboundBytes_ = 0;
      outboundBytes_ = 0;
      inboundMessages_ = 0;
      outboundMessages_ = 0;
      lastBytesPerSecondCount_ = 0;
      lastDataMessagesPerSecondCount_ = 0;
      sessionMessages_ = 0;
      controlMessages_ = 0;

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

  // *NOTE*: if this is an 'outbound' stream, any 'inbound' data (!) will
  //         eventually turn around and travel back upstream for dispatch
  //         --> account for it only once
  //inbound_ = configuration_in.inbound;

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
          typename UserDataType>
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
                                              UserDataType>::handleControlMessage (ControlMessageType& controlMessage_in)
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
          typename UserDataType>
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
                                              UserDataType>::handleDataMessage (DataMessageType*& message_inout,
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
          typename UserDataType>
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
                                              UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

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
      // compute throughput ?
      if (inherited::configuration_->computeThroughput)
      {
        typename TIMER_SECONDPUBLISHER_T::INTERFACE_T* itimer_second_publisher_p =
          TIMER_SECONDPUBLISHER_T::SINGLETON_T::instance ();
        ACE_ASSERT (itimer_second_publisher_p);
        itimer_second_publisher_p->subscribe (this);
      } // end IF

      // statistic reporting
      if (reportingInterval_ != ACE_Time_Value::zero)
      {
        // schedule the reporting interval timer
        ACE_ASSERT (inherited::configuration_);
        ACE_ASSERT (localReportingHandlerId_ == -1);

        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : TimerManagerType::SINGLETON_T::instance ());
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
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled (local) reporting timer (id: %d, interval: %#T)\n"),
                    inherited::mod_->name (),
                    localReportingHandlerId_,
                    &reportingInterval_));
      } // end IF
      // *NOTE*: even if 'this' doesn't report(), it might still be triggered from
      //         outside

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      // sanity check(s)
      if (unlikely (inherited::sessionData_ == NULL))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: out-of-session statistic messages cannot be processed, returning\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF

      // *NOTE*: the message contains statistic information from some upstream
      //         module (e.g. some hardware capture device driver)
      //         --> aggregate this data so that any new (session) messages
      //             generated by this module carry up-to-date information
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      const SessionMessageType::DATA_T& session_data_container_r =
        message_inout->getR ();
      const typename SessionMessageType::DATA_T::DATA_T& session_data_2 =
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
      if (inherited::configuration_->computeThroughput)
      {
        typename TIMER_SECONDPUBLISHER_T::INTERFACE_T* itimer_second_publisher_p =
          TIMER_SECONDPUBLISHER_T::SINGLETON_T::instance ();
        ACE_ASSERT (itimer_second_publisher_p);
        itimer_second_publisher_p->unsubscribe (this);
      } // end IF

      // stop (reporting) timer(s) (--> (re-)initialize())
      finiTimer ();

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
          typename UserDataType>
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
                                              UserDataType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::reset"));

  // *NOTE*: reset() occurs every second

  // step1: reset counters
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    // support asynchronous API
    lastBytesPerSecondCount_ = static_cast<ACE_UINT32> (byteCounter_);
    lastDataMessagesPerSecondCount_ = messageCounter_ - sessionMessageCounter_;

    // reset counters
    byteCounter_ = 0;
    fragmentCounter_ = 0;
    controlMessageCounter_ = 0;
    messageCounter_ = 0;
    sessionMessageCounter_ = 0;
  } // end lock scope

  // step2: update session data and dispatch statistic message
  update (ACE_Time_Value (1, 0));
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
          typename UserDataType>
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
                                              UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::collect"));

  // *NOTE*: external call; fill the argument with meaningful values
  // *TODO*: the template must not know about StatisticContainerType
  //         --> should be overriden by derived classes

  // initialize return value(s)
  ACE_OS::memset (&data_out, 0, sizeof (StatisticContainerType));
  data_out.timeStamp = COMMON_TIME_NOW;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, false);
    // *TODO*: remove type inferences
    data_out.bytes = inboundBytes_ + outboundBytes_;
    data_out.dataMessages =
      inboundMessages_ + outboundMessages_ - sessionMessages_ - controlMessages_;
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
          typename UserDataType>
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
                                              UserDataType>::update (const ACE_Time_Value& interval_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::update"));

  ACE_UNUSED_ARG (interval_in);

  typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
    &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  ACE_ASSERT (session_data_p->lock);

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, *(session_data_p->lock));
    // *TODO*: remove type inferences
    session_data_p->statistic.bytes = inboundBytes_ + outboundBytes_;
    session_data_p->statistic.dataMessages =
      inboundMessages_ + outboundMessages_ - sessionMessages_ - controlMessages_;
    //session_data_r.statistic.droppedMessages = 0;
    session_data_p->statistic.bytesPerSecond =
      static_cast<float> (lastBytesPerSecondCount_);
    session_data_p->statistic.messagesPerSecond =
      static_cast<float> (lastDataMessagesPerSecondCount_);
    session_data_p->statistic.timeStamp = COMMON_TIME_NOW;
  } // end lock scope

  // *TODO*: move this into report()
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
          typename UserDataType>
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
                                              UserDataType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::report"));

  int result = -1;
  typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;
  unsigned int data_messages_i = 0, total_messages_i = 0;

  ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_);

  if (likely (inherited::sessionData_))
    session_data_p = &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  if (likely (session_data_p))
    if (likely (session_data_p->lock))
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

  data_messages_i =
    inboundMessages_ + outboundMessages_ - sessionMessages_ - controlMessages_;
  total_messages_i = inboundMessages_ + outboundMessages_;

  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %d] STATISTIC ***\n--> Stream Statistic <--\n\tmessages/sec: %u\n\tmessages total [in/out]): %u/%u (data: %.2f%%)\n\tbytes/sec: %u\n\tbytes total: %Q\n--> Cache Statistics <--\n\tcurrent cache usage [%u messages / %u byte(s) allocated]\n*** RUNTIME STATISTICS ***\\END\n"),
              (session_data_p ? static_cast<int> (session_data_p->sessionId) : -1),
              lastDataMessagesPerSecondCount_, inboundMessages_, outboundMessages_,
              (static_cast<float> (data_messages_i) / static_cast<float> (total_messages_i) * 100.0F),
              lastBytesPerSecondCount_, inboundBytes_ + outboundBytes_,
              (inherited::allocator_ ? inherited::allocator_->cache_size () : 0),
              (inherited::allocator_ ? inherited::allocator_->cache_depth () : 0)));

  if (likely (session_data_p))
    if (likely (session_data_p->lock))
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
          typename UserDataType>
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
                                              UserDataType>::finalReport () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::finalReport"));

  int result = -1;
  typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;
  if (likely (inherited::sessionData_))
    session_data_p = &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

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
                inboundMessages_ - sessionMessages_ - controlMessages_,
                outboundMessages_ - (sessionMessages_ - 1) - controlMessages_)); // *NOTE*: -1 to account for this message

    unsigned int data_messages =
      (inboundMessages_ - sessionMessages_ - controlMessages_) +
      (outboundMessages_ - (sessionMessages_ - 1) - controlMessages_);
    for (STATISTIC_ITERATOR_T iterator = messageTypeStatistic_.begin ();
         iterator != messageTypeStatistic_.end ();
         iterator++)
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("\t\"%s\": %u --> %.2f %%\n"),
                  ACE_TEXT (DataMessageType::CommandTypeToString (iterator->first).c_str ()),
                  iterator->second,
                  (static_cast<float> (iterator->second) * 100.0F) / static_cast<float> (data_messages)));
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("------------------------------------------\n\ttotal byte(s) [in/out]: %Q/%Q\n\tbytes/s: %u\n"),
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
          typename UserDataType>
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
                                              UserDataType>::finiTimer ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::finiTimer"));

  int result = -1;
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    TimerManagerType::SINGLETON_T::instance ();
  ACE_ASSERT (itimer_manager_p);

  if (localReportingHandlerId_ != -1)
  {
    const void* act_p = NULL;
    result = itimer_manager_p->cancel_timer (localReportingHandlerId_,
                                             &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  localReportingHandlerId_));
    ACE_UNUSED_ARG (act_p);
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
          typename UserDataType>
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
                                              UserDataType>::putStatisticMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_WriterTask_T::putStatisticMessage"));

  typename SessionMessageType::DATA_T* session_data_container_p = NULL;
  if (inherited::sessionData_)
  {
    inherited::sessionData_->increase ();
    session_data_container_p = inherited::sessionData_;
  } // end IF
  else
  {
    typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;
    ACE_NEW_NORETURN (session_data_p,
                      typename SessionMessageType::DATA_T::DATA_T ());
    if (unlikely (!session_data_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF

    // *IMPORTANT NOTE*: fire-and-forget session_data_p
    ACE_NEW_NORETURN (session_data_container_p,
                      typename SessionMessageType::DATA_T (session_data_p));
    if (unlikely (!session_data_container_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
  } // end ELSE
  ACE_ASSERT (session_data_container_p);

  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    session_data_container_p->getR ();

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
                                          session_data_r.userData,
                                          false)); // expedited ?
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
    session_data_container_p->decrease (); session_data_container_p = NULL;
    return false;
  } // end IF
  if (inherited::allocator_)
  {
    // *TODO*: remove type inference
    // *IMPORTANT NOTE*: fire-and-forget session_data_container_p
    session_message_p->initialize (session_data_r.sessionId,
                                   STREAM_SESSION_MESSAGE_STATISTIC,
                                   session_data_container_p,
                                   session_data_r.userData,
                                   false); // expedited ?
    session_data_container_p = NULL;
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
          typename UserDataType>
Stream_Statistic_StatisticReport_ReaderTask_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              ProtocolCommandType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              UserDataType>::Stream_Statistic_StatisticReport_ReaderTask_T (ISTREAM_T* stream_in)
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
          typename UserDataType>
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
                                              UserDataType>::put (ACE_Message_Block* messageBlock_in,
                                                                  ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticReport_ReaderTask_T::put"));

  ACE_Task_Base* task_base_p = inherited::sibling ();
  ACE_ASSERT (task_base_p);
  WRITER_TASK_T* writer_p = static_cast<WRITER_TASK_T*> (task_base_p);

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_CONTROL:
    {
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, writer_p->lock_, -1);
        // update counters
        ++writer_p->outboundMessages_;
        ++writer_p->messageCounter_;
      } // end lock scope
      break;
    }
    case STREAM_MESSAGE_SESSION:
    {
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, writer_p->lock_, -1);
        // update counters
        ++writer_p->outboundMessages_;
        ++writer_p->messageCounter_;
      } // end lock scope
      break;
    }
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
    {
      DataMessageType* message_p =
        static_cast<DataMessageType*> (messageBlock_in);
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
        writer_p->outboundBytes_ += messageBlock_in->total_length ();
        ++writer_p->outboundMessages_;

        writer_p->byteCounter_ += messageBlock_in->total_length ();
        // *TODO*: count fragment(s) as well
        ++writer_p->messageCounter_;
        // add message to statistic
        writer_p->messageTypeStatistic_[message_p->command ()]++;
      } // end lock scope

      break;
    }
    default:
      break;
  } // end SWITCH

  return inherited::put_next (messageBlock_in, timeValue_in);
}
