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

#include "stream_session_message_base.h"
#include "stream_macros.h"

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_QueueReader_T<SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::Stream_Module_QueueReader_T (bool isActive_in,
                                                                                  bool autoStart_in)
 : inherited (isActive_in,
              autoStart_in,
              true)
 , isInitialized_ (false)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::Stream_Module_QueueReader_T"));

}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_QueueReader_T<SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::~Stream_Module_QueueReader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::~Stream_Module_QueueReader_T"));

  int result = -1;

  if (timerID_ != -1)
  {
    const void* act_p = NULL;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                  &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  timerID_));
    else
      ACE_DEBUG ((LM_WARNING, // this should happen in END_SESSION
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  timerID_));
  } // end IF
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_QueueReader_T<SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::initialize"));

  int result = -1;

  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (timerID_ != -1)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                    &act_p);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    timerID_));
    } // end IF
    timerID_ = -1;
    isInitialized_ = false;
  } // end IF

  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

  return isInitialized_;
}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ProtocolHeaderType>
//void
//Stream_Module_QueueReader_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType,
//                           ProtocolHeaderType>::handleSessionMessage (SessionMessageType*& message_inout,
//                                                                      bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::handleSessionMessage"));
//
//  int result = -1;
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  // sanity check(s)
//  // *TODO*: remove type inference
//  ACE_ASSERT (inherited::configuration_.streamConfiguration);
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//
//  switch (message_inout->type ())
//  {
//    case SESSION_BEGIN:
//    {
//      if (inherited::configuration_.streamConfiguration->statisticReportingInterval)
//      {
//        // schedule regular statistics collection...
//        ACE_Time_Value interval (NET_STREAM_DEFAULT_STATISTICS_COLLECTION,
//                                 0);
//        ACE_ASSERT (timerID_ == -1);
//        ACE_Event_Handler* handler_p = &statisticCollectionHandler_;
//        timerID_ =
//            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
//                                                                        NULL,                       // argument
//                                                                        COMMON_TIME_NOW + interval, // first wakeup time
//                                                                        interval);                  // interval
//        if (timerID_ == -1)
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
//          return;
//        } // end IF
//        //        ACE_DEBUG ((LM_DEBUG,
//        //                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for interval %#T...\n"),
//        //                    timerID_,
//        //                    &interval));
//      } // end IF
//
////      // start profile timer...
////      profile_.start ();
//
//      break;
//    }
//    case SESSION_END:
//    {
//      if (timerID_ != -1)
//      {
//        const void* act_p = NULL;
//        result =
//            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
//                                                                      &act_p);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
//                      timerID_));
//        timerID_ = -1;
//      } // end IF
//
//      break;
//    }
//    default:
//      break;
//  } // end SWITCH
//}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_QueueReader_T<SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timestamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticsMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putSessionMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_QueueReader_T<SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_Module_QueueReader_T<SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::svc"));

  // sanity check(s)
  ACE_ASSERT (queue_);

  // step0: increment thread count
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited::lock_);

    inherited::threadCount_++;
  } // end IF

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  // step1: start processing data...
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("entering processing loop...\n")));
  while (queue_->get (message_block_p, NULL) != -1)
  {
    if (message_block->msg_type () == ACE_Message_Block::MB_STOP)
    {
      // clean up
      message_block_p->release ();

      result = 0;

      goto session_finished;
    } // end IF

    result = inherited::put_next (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));

      // clean up
      message_block_p->release ();

      goto session_finished;
    } // end IF
  } // end WHILE

  // clean up
  message_block_p->release ();

session_finished:
  // step2: send final session message downstream...
  if (!inherited::putSessionMessage (STREAM_SESSION_END,
                                     inherited::sessionData_,
                                     false))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));

  result = stream_.close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                ACE_TEXT (inherited::configuration_.sourceFilename.c_str ())));
  isOpen_ = false;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("closed file stream \"%s\"...\n"),
              ACE_TEXT (inherited::configuration_.sourceFilename.c_str ())));

done:
  // signal the controller
  inherited::finished ();

  return result;
}

//template <typename StreamStateType,
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // (reference counted)
//          typename SessionMessageType,
//          typename ProtocolMessageType>
// Net_Message*
// Stream_Module_QueueReader_T::allocateMessage (unsigned int requestedSize_in)
// {
//STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::allocateMessage"));
//
//   // init return value(s)
//   Net_Message* message_out = NULL;
//
//   try
//   {
//     message_out = static_cast<Net_Message*> (//inherited::allocator_->malloc (requestedSize_in));
//   }
//   catch (...)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
//                 requestedSize_in));
//   }
//   if (!message_out)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
//                 requestedSize_in));
//   } // end IF
//
//   return message_out;
// }

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_QueueReader_T<SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  // *TODO*: remove type inferences
  ACE_ASSERT (inherited::configuration_.streamConfiguration);

  // step1: update session state
  // *TODO*: remove type inferences
  inherited::sessionData_->currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

  // step2: create session data object container
  SessionDataContainerType* session_data_p = NULL;
  ACE_NEW_NORETURN (session_data_p,
                    SessionDataContainerType (inherited::sessionData_,
                                              false));
  if (!session_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
    return false;
  } // end IF

  // step3: send the statistic data downstream
  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return inherited::putSessionMessage (SESSION_STATISTICS,
                                       session_data_p,
                                       inherited::configuration_.streamConfiguration->messageAllocator);
}
