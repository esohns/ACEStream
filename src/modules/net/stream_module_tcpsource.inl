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

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::Stream_Module_TCPSource_T (bool isActive_in,
                                                                     bool autoStart_in)
 : inherited (isActive_in,
              autoStart_in,
              true)
 , isInitialized_ (false)
 , isOpen_ (false)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::Stream_Module_TCPSource_T"));

}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::~Stream_Module_TCPSource_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::~Stream_Module_TCPSource_T"));

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

  if (isOpen_)
  {
  } // end IF
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::initialize"));

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

    if (isOpen_)
    {
      isOpen_ = false;
    } // end IF
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
//          typename StatisticContainerType>
//void
//Stream_Module_TCPSource_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::handleDataMessage"));

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_TCPSource_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
//                                                                          bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::handleSessionMessage"));

//  int result = -1;

//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);

//  // sanity check(s)
//  // *TODO*: remove type inference
//  ACE_ASSERT (inherited::configuration_.streamConfiguration);
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);

//  switch (message_inout->type ())
//  {
//    case SESSION_BEGIN:
//    {
//      if (inherited::configuration_.streamConfiguration->statisticReportingInterval)
//      {
//        // schedule regular statistics collection...
//        ACE_Time_Value interval (STREAM_STATISTICS_COLLECTION, 0);
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

////      // start profile timer...
////      profile_.start ();

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
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::collect"));

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
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
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
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::report"));

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
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
int
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::svc"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);
  ACE_ASSERT (!isOpen_);

  // step0: increment thread count
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited::lock_);

    inherited::threadCount_++;
  } // end IF

  // step1: initialize connector
  // *TODO*: remove type inferences
  ConnectionManagerType* connection_manager_p =
      inherited::configuration_.streamConfiguration.connectionManager;
  ACE_ASSERT (connection_manager_p);
  typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
    connection_manager_p;
  ConnectorType connector (iconnection_manager_p,
                           inherited::configuration_.streamConfiguration.statisticReportingInterval);
  Stream_IInetConnector_t* iconnector_p = &connector;
  char buffer[BUFSIZ];
  typename ConnectionManagerType::CONNECTION_T* connection_p = NULL;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  const typename ConnectionManagerType::CONNECTION_T::STREAM_T* stream_p = NULL;
  Stream_Module_t* module_p = NULL;
  Stream_Module_t* module_2 = NULL;
  if (!iconnector_p->initialize (inherited::configuration_.socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
    goto session_finished;
  } // end IF

  // step2: initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  connection_manager_p->set (inherited::configuration_,
                             &inherited::configuration_.streamUserData);

  // step3: connect
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result =
    inherited::configuration_.signalHandlerConfiguration.peerAddress.addr_to_string (buffer,
                                                                                     sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));
  handle =
    iconnector_p->connect (inherited::configuration_.signalHandlerConfiguration.peerAddress);
  if (!inherited::configuration_.useReactor)
  {
    ACE_Time_Value one_second (1, 0);
    result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &one_second));
  } // end IF
  connection_p =
      connection_manager_p->get (inherited::configuration_.signalHandlerConfiguration.peerAddress);
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\", aborting\n"),
                ACE_TEXT (buffer)));
    goto session_finished;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected to \"%s\"...\n"),
              ACE_TEXT (buffer)));

  // step4: (temporarily) link connection stream to this
  stream_p = &connection_p->stream ();
  module_p = stream_p->tail ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::tail(): \"%m\", aborting\n")));
    goto session_finished;
  } // end IF
  module_2 = inherited::module ();
  ACE_ASSERT (module_2);
  result = module_p->link (module_2);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Module::link(%@): \"%m\", aborting\n"),
                module_2));
    goto session_finished;
  } // end IF

session_finished:
  // step5: send final session message downstream...
  if (!inherited::putSessionMessage (STREAM_SESSION_END,
                                     inherited::sessionData_,
                                     false))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));

done:
  // clean up
  if (connection_p)
  {
    connection_p->close ();
    connection_p->decrease ();
  } // end IF

  // signal the controller
  inherited::finished ();

  return result;
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
ProtocolMessageType*
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_out = NULL;

  if (inherited::configuration_.streamConfiguration->messageAllocator)
  {
    try
    {
      // *TODO*: remove type inference
      message_out =
          static_cast<ProtocolMessageType*> (inherited::configuration_.streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_out = NULL;
    }
  } // end IF
  else
  {
    ACE_NEW_NORETURN (message_out,
                      ProtocolMessageType (requestedSize_in));
  } // end ELSE
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_TCPSource_T<SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType,
                          ConnectionManagerType,
                          ConnectorType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPSource_T::putStatisticMessage"));

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
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       session_data_p,
                                       inherited::configuration_.streamConfiguration->messageAllocator);
}
