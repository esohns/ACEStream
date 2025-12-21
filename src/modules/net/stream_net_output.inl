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

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_net_common.h"

#include "net_common_tools.h"
#include "net_iconnector.h"

#include "net_client_defines.h"

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
Stream_Module_Net_OutputReader_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionNotificationType,
                                 ConnectionManagerType,
                                 UserDataType>::Stream_Module_Net_OutputReader_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_OutputReader_T::Stream_Module_Net_OutputReader_T"));

}

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
void
Stream_Module_Net_OutputReader_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionNotificationType,
                                 ConnectionManagerType,
                                 UserDataType>::handleControlMessage (ControlMessageType& controlMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_OutputReader_T::handleControlMessage"));

  ConnectionManagerType* connection_manager_p =
      ConnectionManagerType::SINGLETON_T::instance ();
  typename ConnectionManagerType::ICONNECTION_T* connection_p = NULL;
  WRITER_T* sibling_task_p = static_cast<WRITER_T*> (inherited::sibling ());
  if (unlikely (!sibling_task_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Module_Net_OutputWriter_T>(0x%@), returning\n"),
                inherited::mod_->name (),
                inherited::sibling ()));
    return;
  } // end IF
  SESSION_DATA_T* session_data_p = NULL;

  // sanity check(s)
  ACE_ASSERT (connection_manager_p);
  // *TODO*: remove type inferences
  if (unlikely (!sibling_task_p->sessionData_))
    goto continue_; // something went wrong: session aborted ?

  session_data_p =
    &const_cast<SESSION_DATA_T&> (sibling_task_p->sessionData_->getR ());
  //  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);
    ACE_ASSERT (!session_data_p->connectionStates.empty ());
    ACE_ASSERT (session_data_p->connectionStates.size () == 1);
    ACE_ASSERT ((*session_data_p->connectionStates.begin ()).first != ACE_INVALID_HANDLE);
    connection_p =
      connection_manager_p->get ((*session_data_p->connectionStates.begin ()).first);
  //} // end lock scope
  if (unlikely (!connection_p))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve connection (id was: 0x%@), returning\n"),
                inherited::mod_->name (),
                (*session_data_p->connectionStates.begin ()).first));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve connection (id was: %d), returning\n"),
                inherited::mod_->name (),
                (*session_data_p->connectionStates.begin ()).first));
#endif // ACE_WIN32 || ACE_WIN64
    return;
  } // end IF

continue_:
  switch (controlMessage_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_DISCONNECT:
    { ACE_ASSERT (connection_p);

      // *WARNING*: regular disconnections must enforce that all enqueued
      //            outbound data has been dispatched by the kernel. This
      //            implementation works as long as there are no asynchronous
      //            upstream module reader tasks
      Stream_IMessageQueue* i_message_queue_p =
        dynamic_cast<Stream_IMessageQueue*> (connection_p);
      if (unlikely (!i_message_queue_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Stream_IMessageQueue>(0x%@), returning\n"),
                    inherited::mod_->name (),
                    connection_p));
        return;
      } // end IF
      try {
        i_message_queue_p->waitForIdleState ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Net_IStreamConnection_T::waitForIdleState() (id was: %u), continuing\n"),
                    inherited::mod_->name (),
                    connection_p->id ()));
        return;
      }

      // *WARNING*: the control flow falls through here
      ACE_FALLTHROUGH;
    }
    case STREAM_CONTROL_MESSAGE_ABORT:
    { ACE_ASSERT (connection_p);
      try {
        connection_p->abort ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Net_IConnection_T::abort() (id was: %u), continuing\n"),
                    inherited::mod_->name (),
                    connection_p->id ()));
        return;
      }
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: aborted connection (id was: %u)\n"),
                  inherited::mod_->name (),
                  connection_p->id ()));

      break;
    }
    default:
      break;
  } // end SWITCH

  connection_p->decrease (); connection_p = NULL;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_OutputWriter_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 ConnectionManagerType,
                                 UserDataType>::Stream_Module_Net_OutputWriter_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_OutputWriter_T::Stream_Module_Net_OutputWriter_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_OutputWriter_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 ConnectionManagerType,
                                 UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_OutputWriter_T::initialize"));

  bool result = false;

  if (unlikely (inherited::isInitialized_))
  {
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBaseSynch_T::initialize(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_OutputWriter_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 ConnectionManagerType,
                                 UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_OutputWriter_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // *IMPORTANT NOTE*: this module dispatches outbound data: route message to
  //                   the siblings' queue; it is forwarded to the
  //                   (sub-)streams' head and notify()d to the (network-) event
  //                   dispatch from there
  // *TODO*: this works as long as there is only a net_target module; if there
  //         is also a net_source module on the same stream, things get mixed up
  int result = -1;

  // sanity check(s)
  ACE_ASSERT (message_inout);

  ACE_Message_Block* message_block_p = message_inout->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = inherited::reply (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 108,10058: connection/stream has/is shut/ting
                            //            down
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_OutputWriter_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 ConnectionManagerType,
                                 UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_OutputWriter_T::handleSessionMessage"));

  int result = -1;
  Stream_Task_t* task_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      break;
    case STREAM_SESSION_MESSAGE_BEGIN:
      break;
    case STREAM_SESSION_MESSAGE_LINK:
    case STREAM_SESSION_MESSAGE_UNLINK:
      break;
    case STREAM_SESSION_MESSAGE_CONNECT:
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      break;
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}
