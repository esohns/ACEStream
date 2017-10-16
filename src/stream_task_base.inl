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

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/Time_Value.h"

#include "stream_defines.h"
#include "stream_ilink.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"
#include "stream_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::Stream_TaskBase_T (ISTREAM_T* stream_in)
 : inherited (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_THREAD_NAME), // thread name
              STREAM_MODULE_TASK_GROUP_ID,                      // group id
              1,                                                // # thread(s)
              false,                                            // auto-start ?
              ////////////////////////////
              //NULL)                                           // queue handle
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)                                          // queue handle
 , aggregate_ (false)
 , allocator_ (NULL)
 , configuration_ (NULL)
 , isInitialized_ (false)
 , linked_ (0)
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
 , sessionData_ (NULL)
 , sessionDataLock_ (NULL)
 , stream_ (stream_in)
 /////////////////////////////////////////
 , freeSessionData_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::Stream_TaskBase_T"));

  // sanity check(s)
  //ACE_ASSERT (stream_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::~Stream_TaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::~Stream_TaskBase_T"));

  int result = -1;

  if (freeSessionData_ &&
      sessionData_)
    sessionData_->decrease ();

  //   // *TODO*: check whether this sequence works
  //   queue_.deactivate ();
  //   queue_.wait ();

  // *NOTE*: deactivate the queue so it does not accept new data
  result = queue_.deactivate ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));

  result = queue_.flush ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
  else if (result)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: flushed %d message(s)\n"),
                inherited::mod_->name (),
                result));
  } // end ELSE IF
  inherited::msg_queue (NULL);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::initialize (const ConfigurationType& configuration_in,
                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::initialize"));

  if (isInitialized_)
  {
    isInitialized_ = false;

    if (freeSessionData_ &&
        sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF
    freeSessionData_ = true;
    sessionDataLock_ = NULL;

    if (aggregate_)
      goto continue_;

    queue_.flush ();
  } // end IF

  linked_ = 0;

continue_:
  allocator_ = allocator_in;
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  stream_ = configuration_in.stream;

  isInitialized_ = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleSessionMessage"));

  // initialize return value(s)
  passMessageDownstream_out = true;

  // *NOTE*: the default behavior is to simply dump the module state at the end
  //         of a session

  const typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    case STREAM_SESSION_MESSAGE_CONNECT:
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      break;
    case STREAM_SESSION_MESSAGE_LINK:
    {
      ++linked_;

      int result = -1;
      bool release_lock = false;
      typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      typename SessionMessageType::DATA_T::DATA_T* session_data_2 = NULL;

      // *IMPORTANT NOTE*: in case the session has been aborted asynchronously,
      //                   the 'session end' message may already have been
      //                   processed at this point ('concurrent' scenario)

      // sanity check(s)
      if (!sessionData_)
        goto continue_;

      session_data_p = &sessionData_->getR ();

      // *TODO*: avoid race condition here; get() should add a reference
      session_data_container_p =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->getR ());
      session_data_container_p->increase ();
      // *IMPORTANT NOTE*: although reuse of the upstream session data is
      //                   warranted, it may not be safe (e.g. connection might
      //                   close unexpectedly, ...)
      //                   --> use 'this' streams' session data lock instead
      // *TODO*: this precaution may be completely unnecessary. Points to
      //         consider:
      //         - linking/unlinking code may have to be synchronized
      //         - upstream session resources (e.g. connection handles, ...)
      //           must not be allocated/used/freed until the streams have been
      //           un/linked
      //         - ...
      session_data_2 =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());

      // 'upstream' ? --> nothing to do
      // *TODO*: writing this from a 'downstream' perspective may result in
      //         better code
      if (session_data_p == session_data_2)
        goto continue_;

      ACE_ASSERT (session_data_p->lock);
      ACE_ASSERT (session_data_2->lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);
        if (session_data_p->lock != session_data_2->lock)
        {
          result = session_data_2->lock->acquire ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          release_lock = true;
        } // end IF
        const_cast<typename SessionMessageType::DATA_T::DATA_T*> (session_data_p)->lock =
          session_data_2->lock;

        // *NOTE*: the idea here is to 'merge' the two datasets
        *session_data_2 += *session_data_p;

        if (release_lock)
        {
          result = session_data_2->lock->release ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX_T::release(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
        } // end IF
      } // end lock scope

      // switch session data
      sessionData_->decrease ();
      sessionData_ = session_data_container_p;

continue_:
      Stream_ILinkCB* ilink_p = dynamic_cast<Stream_ILinkCB*> (this);
      if (ilink_p)
      {
        try {
          ilink_p->onLink ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILinkCB::onLink(), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      // sanity check(s)
      if (!linked_)
        break;
      --linked_;

      // *IMPORTANT NOTE*: in case the session has been aborted asynchronously,
      //                   the 'session end' message may already have been
      //                   processed at this point ('concurrent' scenario)

      // sanity check(s)
      if (!sessionData_)
        goto continue_2;

      session_data_p = &sessionData_->getR ();
      { ACE_ASSERT (sessionDataLock_);
        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *sessionDataLock_);
        const_cast<typename SessionMessageType::DATA_T::DATA_T*> (session_data_p)->lock =
          sessionDataLock_;
      } // end lock scope
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: stream has been unlinked, reset session data lock\n"),
      //            inherited::mod_->name ()));

continue_2:
      Stream_ILinkCB* ilink_p = dynamic_cast<Stream_ILinkCB*> (this);
      if (ilink_p)
      {
        try {
          ilink_p->onUnlink ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILinkCB::onUnlink(), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      if (aggregate_)
      {
        if (freeSessionData_ &&
            sessionData_)
          sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      // sanity check(s)
      if (sessionData_) // --> head modules initialize this in open()
      {
        freeSessionData_ = false;
        goto continue_3;
      } // end IF

      sessionData_ =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->getR ());
      sessionData_->increase ();

continue_3:
      // sanity check(s)
      ACE_ASSERT (sessionData_);

      session_data_p =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_->getR ());
      // *NOTE*: retain a handle to the original lock
      sessionDataLock_ = session_data_p->lock;

      // sanity check(s)
      if (!isInitialized_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: not initialized, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      break;

error:
      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
      break;
    case STREAM_SESSION_MESSAGE_END:
    {
#if defined (_DEBUG)
      try {
        dump_state ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Comon_IDumpState::dump_state(), continuing\n"),
                    inherited::mod_->name ()));
      }
#endif

      if (!linked_ &&
          sessionData_)
      {
        if (freeSessionData_) // --> head modules finalize this in close()
          sessionData_->decrease ();
        sessionData_ = NULL;
        sessionDataLock_ = NULL;
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
      break;
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: invalid/unknown session message type (was: %d), continuing\n"),
                  inherited::mod_->name (),
                  message_inout->type ()));
      break;
    }
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::handleProcessingError (const ACE_Message_Block* const messageBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleProcessingError"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: failed to process message %@, continuing\n"),
              inherited::mod_->name (),
              messageBlock_in));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
DataMessageType*
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_p = NULL;

  // *TODO*: remove type inference
  if (allocator_)
  {
allocate:
    try {
      // *TODO*: remove type inference
      message_p =
          static_cast<DataMessageType*> (allocator_->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  inherited::mod_->name (),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p && !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      DataMessageType (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to allocate data message: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory (requested %u byte(s)): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  requestedSize_in));
  } // end IF

  return message_p;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::handleMessage (ACE_Message_Block* messageBlock_in,
                                                bool& stopProcessing_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleMessage"));

  int result = -1;

  // sanity check
  ACE_ASSERT (messageBlock_in);

  // initialize return value(s)
  stopProcessing_out = false;

  // *NOTE*: the default behavior is to pass everything downstream
  bool forward_b = true;
  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_DATA:
    case ACE_Message_Block::MB_PROTO:
    {
      DataMessageType* message_p =
        dynamic_cast<DataMessageType*> (messageBlock_in);
      if (!message_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<DataMessageType>(0x%@) failed (type was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    messageBlock_in,
                    ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
        goto error;
      } // end IF

      //// *IMPORTANT NOTE*: in certain scenarios (e.g. asynchronous 
      ////                   configurations with a network data source), data may
      ////                   start arriving before the corresponding session has
      ////                   finished initializing (i.e. before the
      ////                   STREAM_SESSION_MESSAGE_BEGIN message has been
      ////                   processed by all modules). Due to this race
      ////                   condition, no session data is available at this
      ////                   stage, and the modules may not behave as intended
      ////                   --> prevent dispatch of data messages in this case
      //// *WARNING*: this test does not work reliably, it only mitigates the race
      ////            condition described
      //// *TODO*: find a way to prevent this from occurring (e.g. pre-buffer all
      ////         'early' messages in the head module, introduce an intermediate
      ////         state machine state 'in_session') to handle these situations
      //if (!sessionData_)
      //{ ACE_ASSERT (inherited::mod_);
      //  if (this == inherited::mod_->writer ())
      //  {
      //    //ACE_DEBUG ((LM_WARNING,
      //    //            ACE_TEXT ("%s: no session: dropping 'early' data message, continuing\n"),
      //    //            inherited::mod_->name ()));
      //    goto error;
      //  } // end IF
      //} // end IF

      try {
        this->handleDataMessage (message_p,
                                 forward_b);
      } catch (...) {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleDataMessage() (message id was: %u), continuing\n"),
//                      inherited::mod_->name (),
//                      message_p->id ()));
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleDataMessage(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      }

      break;

error:
      // clean up
      messageBlock_in->release ();
      forward_b = false;

      stopProcessing_out = true;

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
      ControlMessageType* control_message_p =
        dynamic_cast<ControlMessageType*> (messageBlock_in);
      if (!control_message_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<ControlMessageType>(0x%@) failed (type was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    messageBlock_in,
                    ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
        goto error_2;
      } // end IF

      try {
        handleControlMessage (*control_message_p);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleControlMessage(), aborting\n"),
                    inherited::mod_->name ()));
      }

      break;

error_2:
      // clean up
      messageBlock_in->release ();
      forward_b = false;

      stopProcessing_out = true;

      break;
    }
    case ACE_Message_Block::MB_USER:
    {
      try {
        handleUserMessage (messageBlock_in,
                           stopProcessing_out,
                           forward_b);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleUserMessage() (type was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
        goto error_3;
      }

      break;

error_3:
      // clean up
      messageBlock_in->release ();
      forward_b = false;

      stopProcessing_out = true;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: received invalid/unknown message (type was: \"%s\"), continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));

      // clean up
      messageBlock_in->release ();
      forward_b = false;

      break;
    }
  } // end SWITCH

  // pass message downstream ?
  if (forward_b)
  {
    result = inherited::put_next (messageBlock_in, NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

        // clean up
        messageBlock_in->release ();

        stopProcessing_out = true;
      } // end IF
    } // end IF
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::putControlMessage (SessionControlType messageType_in,
                                                    bool sendUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::putControlMessage"));

  int result = -1;

  // create control message
  ACE_Message_Block* message_block_p = NULL;
  if (allocator_)
  {
allocate:
    try {
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<ACE_Message_Block*> (allocator_->calloc ()),
                               ControlMessageType (messageType_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::calloc(), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    }

    // keep retrying ?
    if (!message_block_p &&
        !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      ControlMessageType (messageType_in));
  if (!message_block_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate control message: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate control message: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    return false;
  } // end IF

  // forward message
  result = (sendUpStream_in ? inherited::reply (message_block_p, NULL)
                            : inherited::put (message_block_p, NULL));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::%s(): \"%m\", aborting\n"),
                inherited::mod_->name (),
                (sendUpStream_in ? ACE_TEXT ("reply") : ACE_TEXT ("put"))));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  return true;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::putSessionMessage (SessionEventType eventType_in,
                                                    typename SessionMessageType::DATA_T*& sessionData_inout,
                                                    UserDataType* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::putSessionMessage"));

  typename SessionMessageType::DATA_T::DATA_T* session_data_p =
    (sessionData_inout ? &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_inout->getR ())
                       : NULL);
  int result = -1;

  // create a session message
  SessionMessageType* session_message_p = NULL;
  if (allocator_)
  {
allocate:
    try {
      // *IMPORTANT NOTE*: 0 --> session message !
      session_message_p =
        static_cast<SessionMessageType*> (allocator_->malloc (0));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(0), aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    }

    // keep retrying ?
    if (!session_message_p &&
        !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType ((session_data_p ? session_data_p->sessionId : -1),
                                          eventType_in,
                                          sessionData_inout,
                                          userData_in));
  if (!session_message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate session message: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate session message: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    goto error;
  } // end IF
  if (allocator_)
    session_message_p->initialize ((session_data_p ? session_data_p->sessionId : -1),
                                   eventType_in,
                                   sessionData_inout,
                                   userData_in);

  // forward message
  result = put (session_message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::put(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

    // clean up
    session_message_p->release ();

    goto error;
  } // end IF

  return true;

error:
  if (sessionData_inout)
  {
    sessionData_inout->decrease ();
    sessionData_inout = NULL;
  } // end IF

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::handleUserMessage (ACE_Message_Block* messageBlock_in,
                                                    bool& stopProcessing_out,
                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleUserMessage"));

  // initialize return value(s)
  stopProcessing_out = false;
  passMessageDownstream_out = true;

  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_USER:
    { // *NOTE*: currently, all of these are 'session' messages
      SessionMessageType* session_message_p =
        dynamic_cast<SessionMessageType*> (messageBlock_in);
      if (!session_message_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_SessionMessageBase_T>(0x%@) failed (type was: %d), aborting\n"),
                    inherited::mod_->name (),
                    messageBlock_in,
                    messageBlock_in->msg_type ()));

        // clean up
        passMessageDownstream_out = false;
        messageBlock_in->release ();

        break;
      } // end IF

      // OK: process session message
      enum Stream_SessionMessageType session_message_type =
        session_message_p->type ();
      // retain/update session data ?
      if ((session_message_type != STREAM_SESSION_MESSAGE_UNLINK) &&
          (session_message_type != STREAM_SESSION_MESSAGE_END))
        OWN_TYPE_T::handleSessionMessage (session_message_p,
                                          passMessageDownstream_out);
      try {
        handleSessionMessage (session_message_p,
                              passMessageDownstream_out);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in handleSessionMessage(), continuing\n"),
                    inherited::mod_->name ()));
      }

      if ((session_message_type == STREAM_SESSION_MESSAGE_UNLINK) ||
          (session_message_type == STREAM_SESSION_MESSAGE_END))
      {
        // *TODO*: currently, the session data will not be released (see below)
        //         if the module forwards the session end message itself
        //         --> memory leakage, resolve ASAP
        if (passMessageDownstream_out)
        {
          ACE_ASSERT (session_message_p);
          OWN_TYPE_T::handleSessionMessage (session_message_p,
                                            passMessageDownstream_out);
        } // end IF
        // *NOTE*: iff this was a SESSION_END message, stop processing (see above)
        if (session_message_type == STREAM_SESSION_MESSAGE_END)
          stopProcessing_out = true;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: received an unknown user message (type was: %d), continuing\n"),
                  inherited::mod_->name (),
                  messageBlock_in->msg_type ()));
      break;
    }
  } // end SWITCH
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionControlType,
                  SessionEventType,
                  UserDataType>::notify (SessionEventType sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::notify"));

  SessionIdType session_id = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  if (sessionData_)
  {
    const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      sessionData_->getR ();
    session_id = session_data_r.sessionId;
  } // end IF

  INOTIFY_T* inotify_p = dynamic_cast<INOTIFY_T*> (inherited::mod_);
  if (!inotify_p)
  { // *TODO*: remove type inferences
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_ISessionNotify_T>(0x%@) failed, returning\n"),
                inherited::mod_->name (),
                inherited::mod_));
    return;
  } // end IF
  try {
    inotify_p->notify (session_id,
                       sessionEvent_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_ISessionNotify_T::notify(%u,%d), continuing\n"),
                inherited::mod_->name (),
                session_id, sessionEvent_in));
  }
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionIdType,
//          typename SessionControlType,
//          typename SessionEventType,
//          typename UserDataType>
//void
//Stream_TaskBase_T<ACE_SYNCH_USE,
//                  TimePolicyType,
//                  ConfigurationType,
//                  ControlMessageType,
//                  DataMessageType,
//                  SessionMessageType,
//                  SessionIdType,
//                  SessionControlType,
//                  SessionEventType,
//                  UserDataType>::next (typename inherited::TASK_T* task_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::next"));
//
//  Stream_IModuleLinkCB* imodulelink_p =
//    (task_in ? dynamic_cast<Stream_IModuleLinkCB*> (task_in->module ())
//             : (inherited::next_ ? dynamic_cast<Stream_IModuleLinkCB*> (inherited::next_->module ())
//                                 : NULL));
//
//  if (task_in)
//    inherited::next (task_in);
//
//  // notify ? 
//  if (!imodulelink_p)
//    goto continue_;
//  if (task_in)
//  {
//    try {
//      imodulelink_p->onLink ();
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: caught exception in Stream_IModuleLinkCB::onLink(), continuing\n"),
//                  task_in->module ()->name ()));
//    }
//  } // end IF
//  else
//  {
//    try {
//      imodulelink_p->onUnlink ();
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: caught exception in Stream_IModuleLinkCB::onUnlink(), continuing\n"),
//                  (inherited::next_ ? inherited::next_->module ()->name () : ACE_TEXT ("N/A"))));
//    }
//  } // end ELSE
//
//continue_:
//  if (!task_in)
//    inherited::next (task_in);
//}
