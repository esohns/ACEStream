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

#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/Time_Value.h>

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_defines.h"
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
                  UserDataType>::Stream_TaskBase_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_THREAD_NAME), // thread name
              STREAM_MODULE_TASK_GROUP_ID,                      // group id
              1,                                                // # thread(s)
              false,                                            // auto-start ?
              ////////////////////////////
              //NULL)                                           // queue handle
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)                                          // queue handle
 , allocator_ (NULL)
 , configuration_ (NULL)
 , isInitialized_ (false)
 , isLinked_ (false)
 , sessionData_ (NULL)
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::Stream_TaskBase_T"));

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

  if (sessionData_)
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
    if (inherited::mod_)
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: flushed %d message(s)...\n"),
                  inherited::mod_->name (),
                  result));
    else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("flushed %d message(s)...\n"),
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

  allocator_ = allocator_in;
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isLinked_ = false;
  queue_.flush ();

  // *NOTE*: allow re-initialization while retaining the session data
  if (isInitialized_ && sessionData_)
  {
    sessionData_->decrease ();
    sessionData_ = NULL;
  } // end IF

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

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    case STREAM_SESSION_MESSAGE_CONNECT:
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      break;
    case STREAM_SESSION_MESSAGE_LINK:
    {
      isLinked_ = true;

      // *IMPORTANT NOTE*: in case the session has been aborted asynchronously,
      //                   the 'session end' message may already have been
      //                   processed at this point ('concurrent' scenario)
      // sanity check(s)
      if (!sessionData_) break;

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_->get ());

      // *TODO*: avoid race condition here; get() should add a reference
      typename SessionMessageType::DATA_T* session_data_container_p =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->get ());
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
      const typename SessionMessageType::DATA_T::DATA_T& session_data_2 =
        session_data_container_p->get ();

      int result = -1;
      bool release_lock = false;

      // 'upstream' ? --> nothing to do
      // *TODO*: writing this from a 'downstream' perspective may be better code
      if (&session_data_r == &session_data_2) goto continue_;

      ACE_ASSERT (session_data_r.lock);
      ACE_ASSERT (session_data_2.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        if (session_data_r.lock != session_data_2.lock)
        {
          result = session_data_2.lock->acquire ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          release_lock = true;
        } // end IF
        session_data_r.lock = session_data_2.lock;

        // *NOTE*: the idea here is to 'merge' the two datasets
        session_data_r += session_data_2;

        if (release_lock)
        {
          result = session_data_2.lock->release ();
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
      //// *IMPORTANT NOTE*: link()ing two streams implies that there will be
      ////                   two 'session end' messages: one for 'upstream', and
      ////                   one for 'this'
      ////                   --> increase reference count of the session data, so
      ////                       it is not released prematurely
      //sessionData_->increase ();

      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      isLinked_ = false;
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      if (sessionData_) // --> session head modules initialize this in open()
        break;

      sessionData_ =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->get ());
      sessionData_->increase ();

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
      break;
    case STREAM_SESSION_MESSAGE_END:
    {
      //try {
      //  dump_state ();
      //} catch (...) {
      //  if (inherited::mod_)
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: caught exception in dump_state(), continuing\n"),
      //                inherited::mod_->name ()));
      //  else
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("caught exception in dump_state(), continuing\n")));
      //}

      if (isLinked_)
        isLinked_ = false;

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
      break;
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), continuing\n"),
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

  if (inherited::mod_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to process message %@, continuing\n"),
                inherited::mod_->name (),
                messageBlock_in));
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to process message %@, continuing\n"),
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
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      DataMessageType (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                    requestedSize_in));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
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

  // *NOTE*: the default behavior is to pass EVERYTHING downstream
  bool passMessageDownstream = true;
  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_DATA:
    case ACE_Message_Block::MB_PROTO:
    {
      DataMessageType* message_p =
        dynamic_cast<DataMessageType*> (messageBlock_in);
      if (!message_p)
      {
        std::string type_string =
          Stream_Tools::messageType2String (static_cast<Stream_MessageType> (messageBlock_in->msg_type ()));
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: dynamic_cast<DataMessageType*>(%@) failed (type was: \"%s\"), returning\n"),
                      inherited::mod_->name (),
                      messageBlock_in,
                      ACE_TEXT (type_string.c_str ())));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("dynamic_cast<DataMessageType*>(%@) failed (type was: \"%s\"), returning\n"),
                      messageBlock_in,
                      ACE_TEXT (type_string.c_str ())));

        // clean up
        messageBlock_in->release ();
        passMessageDownstream = false;

        return;
      } // end IF

      try {
        this->handleDataMessage (message_p,
                                 passMessageDownstream);
      } catch (...) {
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught an exception in handleDataMessage() (message id was: %u), continuing\n"),
                      inherited::mod_->name (),
                      message_p->id ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleDataMessage() (message id was: %u), continuing\n"),
                      message_p->id ()));
      }

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
        std::string type_string =
          Stream_Tools::messageType2String (static_cast<Stream_MessageType> (messageBlock_in->msg_type ()));
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: dynamic_cast<ControlMessageType>(%@) failed (type was: \"%s\"), returning\n"),
                      inherited::mod_->name (),
                      messageBlock_in,
                      ACE_TEXT (type_string.c_str ())));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("dynamic_cast<ControlMessageType>(%@) failed (type was: \"%s\"), returning\n"),
                      messageBlock_in,
                      ACE_TEXT (type_string.c_str ())));

        // clean up
        messageBlock_in->release ();
        passMessageDownstream = false;

        break;
      } // end IF

      try {
        handleControlMessage (*control_message_p);
      }
      catch (...) {
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught an exception in handleControlMessage(), continuing\n"),
                      inherited::mod_->name ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleControlMessage(), continuing\n")));
      }

      break;
    }
    case ACE_Message_Block::MB_USER:
    {
      try {
        handleUserMessage (messageBlock_in,
                           stopProcessing_out,
                           passMessageDownstream);
      } catch (...) {
        std::string type_string =
          Stream_Tools::messageType2String (static_cast<Stream_MessageType> (messageBlock_in->msg_type ()));
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught an exception in handleUserMessage() (type was: \"%s\"), continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (type_string.c_str ())));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleUserMessage() (type: \"%s\"), continuing\n"),
                      ACE_TEXT (type_string.c_str ())));
      }

      break;
    }
    default:
    {
      if (inherited::mod_)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: received an unknown message (type was: %d), continuing\n"),
                    inherited::mod_->name (),
                    messageBlock_in->msg_type ()));
      else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("received an unknown message (type was: %d), continuing\n"),
                    messageBlock_in->msg_type ()));
      break;
    }
  } // end SWITCH

  // pass message downstream (if there is a stream)
  if (passMessageDownstream)
  {
    // *NOTE*: tasks that are not part of a stream have no notion of the concept
    if (!inherited::mod_)
    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("cannot put_next(): not a module, continuing\n")));

      // clean up
      messageBlock_in->release ();
    } // end IF
    else
    {
      result = inherited::put_next (messageBlock_in, NULL);
      if (result == -1)
      {
        int error = ACE_OS::last_error ();
        if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

        // clean up
        messageBlock_in->release ();
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
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(), aborting\n")));
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
                    ACE_TEXT ("failed to allocate ControlMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ControlMessageType: \"%m\", aborting\n")));
    return false;
  } // end IF

  // pass message downstream
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);
  result = (sendUpStream_in ? this_p->reply (message_block_p,
                                             NULL)
                            : this_p->put (message_block_p,
                                           NULL));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::%s(): \"%m\", aborting\n"),
                (sendUpStream_in ? ACE_TEXT ("reply") : ACE_TEXT ("put"))));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued control message...\n")));

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
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));

      // clean
      if (sessionData_inout)
      {
        sessionData_inout->decrease ();
        sessionData_inout = NULL;
      } // end IF

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
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (eventType_in,
                                          sessionData_inout,
                                          userData_in));
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

    // clean
    if (sessionData_inout)
    {
      sessionData_inout->decrease ();
      sessionData_inout = NULL;
    } // end IF

    return false;
  } // end IF
  if (allocator_)
  {
    // *TODO*: remove type inference
    session_message_p->initialize (eventType_in,
                                   sessionData_inout,
                                   userData_in);
  } // end IF

  // pass message downstream
  result = const_cast<OWN_TYPE_T*> (this)->put (session_message_p,
                                                NULL);
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
      SessionMessageType* session_message_p = NULL;
      session_message_p = dynamic_cast<SessionMessageType*> (messageBlock_in);
      if (!session_message_p)
      {
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: dynamic_cast<SessionMessageType>(%@) failed (type was: %d), aborting\n"),
                      inherited::mod_->name (),
                      messageBlock_in,
                      messageBlock_in->msg_type ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("dynamic_cast<SessionMessageType>(%@) failed (type was: %d), aborting\n"),
                      messageBlock_in,
                      messageBlock_in->msg_type ()));

        // clean up
        passMessageDownstream_out = false;
        messageBlock_in->release ();

        break;
      } // end IF

      // OK: process session message
      Stream_SessionMessageType session_message_type =
        session_message_p->type ();
      // retain/update session data ?
      if (session_message_type != STREAM_SESSION_MESSAGE_END)
        OWN_TYPE_T::handleSessionMessage (session_message_p,
                                          passMessageDownstream_out);
      try {
        handleSessionMessage (session_message_p,
                              passMessageDownstream_out);
      } catch (...) {
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught an exception in handleSessionMessage(), continuing\n"),
                      inherited::mod_->name ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleSessionMessage(), continuing\n")));
      }

      // *NOTE*: if this was a SESSION_END message, stop processing (see above)
      if (session_message_type == STREAM_SESSION_MESSAGE_END)
      {
        // *TODO*: currently, the session data will not be released (see below)
        //         if the module forwards the session end message itself
        if (passMessageDownstream_out)
        {
          ACE_ASSERT (session_message_p);
          OWN_TYPE_T::handleSessionMessage (session_message_p,
                                            passMessageDownstream_out);
        } // end IF
        stopProcessing_out = true;
      } // end IF

      break;
    }
    default:
    {
      if (inherited::mod_)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: received an unknown user message (type was: %d), continuing\n"),
                    inherited::mod_->name (),
                    messageBlock_in->msg_type ()));
      else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("received an unknown user message (type was: %d), continuing\n"),
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
      sessionData_->get ();
    session_id = session_data_r.sessionID;
  } // end IF

  INOTIFY_T* inotify_p = dynamic_cast<INOTIFY_T*> (inherited::mod_);
  if (!inotify_p)
  { // *TODO*: remove type inferences
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_ISessionNotify_T*>(0x%@) failed, returning\n"),
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
