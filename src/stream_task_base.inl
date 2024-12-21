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
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::Stream_TaskBase_T (ISTREAM_T* stream_in,
                                                    MESSAGE_QUEUE_T* queue_in)
 : inherited (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_THREAD_NAME), // thread name
              STREAM_MODULE_TASK_GROUP_ID,                      // group id
              0,                                                // # thread(s)
              false,                                            // auto-start ?
              ////////////////////////////
              queue_in)                                         // queue handle
 , aggregate_ (false)
 , allocator_ (NULL)
 , configuration_ (NULL)
 , isInitialized_ (false)
 , linked_ (0)
 , sessionData_ (NULL)
 /////////////////////////////////////////
 , freeSessionData_ (false)
 , sessionData_2 (NULL)
 , sessionDataLock_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::Stream_TaskBase_T"));

  // *TODO*
  ACE_UNUSED_ARG (stream_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::~Stream_TaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::~Stream_TaskBase_T"));

  if (freeSessionData_ &&
      sessionData_)
    sessionData_->decrease ();
  if (unlikely (sessionData_2))
    sessionData_2->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::initialize (const ConfigurationType& configuration_in,
                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::initialize"));

  if (unlikely (isInitialized_))
  {
    isInitialized_ = false;

    if (freeSessionData_ &&
        sessionData_)
    {
      sessionData_->decrease (); sessionData_ = NULL;
    } // end IF
    freeSessionData_ = false;
    if (unlikely (sessionData_2))
    {
      sessionData_2->decrease (); sessionData_2 = NULL;
    } // end IF
    sessionDataLock_ = NULL;

    if (aggregate_)
      goto continue_;
  } // end IF

  linked_ = 0;

continue_:
  allocator_ = allocator_in;
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
//  stream_ = configuration_in.stream;

  isInitialized_ = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
const Stream_IStream_T<ACE_SYNCH_USE, TimePolicyType>* const
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::getP () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::getP"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  IGET_T* iget_p = dynamic_cast<IGET_T*> (inherited::mod_);
  if (unlikely (!iget_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Common_IGetR_T<ACE_Stream>>(0x%@) failed --> check implementation !, aborting\n"),
                inherited::mod_->name (),
                inherited::mod_));
    return NULL;
  } // end IF
  STREAM_T& stream_r = const_cast<STREAM_T&> (iget_p->getR ());
  ISTREAM_T* istream_p = NULL;
  try {
    istream_p = dynamic_cast<ISTREAM_T*> (&stream_r);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in dynamic_cast<Stream_IStream_T>(0x%@), aborting\n"),
                inherited::mod_->name (),
                &stream_r));
  }
  if (unlikely (!istream_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStream_T>(0x%@) failed --> check implementation !, aborting\n"),
                inherited::mod_->name (),
                &stream_r));
    return NULL;
  } // end IF

  return istream_p;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleSessionMessage"));

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
      ++linked_;

      int result = -1;
      bool release_upstream_lock_b = false;
      const typename SessionMessageType::DATA_T::DATA_T* session_data_local_p = NULL;
      typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      typename SessionMessageType::DATA_T::DATA_T* session_data_upstream_p = NULL;

      // *IMPORTANT NOTE*: in case the session has been aborted asynchronously,
      //                   the 'session end' message may already have been
      //                   processed at this point ('concurrent' scenario)

      // sanity check(s)
      if (unlikely (!sessionData_))
        goto continue_;

      session_data_local_p = &sessionData_->getR ();

      // *TODO*: avoid race condition here; get() should add a reference
      session_data_container_p =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->getR ());
      session_data_container_p->increase ();
      // *IMPORTANT NOTE*: although reuse of the upstream session data is
      //                   warranted, it may not be safe (e.g. connection might
      //                   close unexpectedly, ...)
      //                   --> use downstreams' session data lock instead
      // *TODO*: this precaution may be completely unnecessary. Points to
      //         consider:
      //         - linking/unlinking code may have to be synchronized
      //         - upstream session resources (e.g. connection handles, ...)
      //           must not be allocated/used/freed until the streams have been
      //           un/linked
      //         - ...
      session_data_upstream_p =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());

      // 'upstream' ? --> nothing to do
      // *TODO*: writing this from an 'upstream' perspective may result in
      //         better code
      if (session_data_local_p == session_data_upstream_p)
      {
        session_data_container_p->decrease (); session_data_container_p = NULL;
        goto continue_; // <-- 'upstream'
      } // end IF

      // --> 'downstream'
      ACE_ASSERT (session_data_local_p->lock && session_data_upstream_p->lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_local_p->lock);
        if (likely (session_data_local_p->lock != session_data_upstream_p->lock))
        {
          result = session_data_upstream_p->lock->acquire ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          release_upstream_lock_b = true;
        } // end IF
        sessionData_2 = sessionData_;
        sessionDataLock_ = session_data_upstream_p->lock; // retain handle to originals
        const_cast<typename SessionMessageType::DATA_T::DATA_T*> (session_data_upstream_p)->lock =
          session_data_local_p->lock;
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("%s: stream has been linked, using downstream session data lock (is: 0x%@)\n"),
        //            inherited::mod_->name (),
        //            session_data_local_p->lock));

        // *NOTE*: the idea here is to 'merge' the two datasets
        *session_data_upstream_p += *session_data_local_p;

        if (likely (release_upstream_lock_b))
        {
          result = sessionDataLock_->release ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX_T::release(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
        } // end IF
      } // end lock scope

      // switch session data
      sessionData_ = session_data_container_p; session_data_container_p = NULL;

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
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // update session data
      if (likely (freeSessionData_ && sessionData_))
        sessionData_->decrease ();
      sessionData_ = &const_cast<typename SessionMessageType::DATA_T&> (message_inout->getR ());
      sessionData_->increase ();
      freeSessionData_ = true;

      const typename SessionMessageType::DATA_T::DATA_T* session_data_p =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_->getR ());
      // *NOTE*: retain a handle to the original lock
      sessionDataLock_ = session_data_p->lock;

      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      // sanity check(s)
      if (unlikely (!linked_))
        break;
      --linked_;

      int result = -1;
      const typename SessionMessageType::DATA_T::DATA_T* session_data_upstream_p = NULL;
      typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      typename SessionMessageType::DATA_T::DATA_T* session_data_message_p = NULL;
      bool release_saved_upstream_lock_b = false;

      // *IMPORTANT NOTE*: in case the session has been aborted asynchronously,
      //                   the 'session end' message may already have been
      //                   processed at this point ('concurrent' scenario)

      // sanity check(s)
      if (!sessionData_)
        goto continue_2;

      session_data_upstream_p = &sessionData_->getR ();
      // *TODO*: avoid race condition here; get() should add a reference
      session_data_container_p =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->getR ());
      session_data_container_p->increase ();
      session_data_message_p =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());

      // 'upstream' ? --> nothing to do
      // *TODO*: writing this from an 'upstream' perspective may result in
      //         better code
      if (session_data_upstream_p == session_data_message_p)
      {
        session_data_container_p->decrease (); session_data_container_p = NULL;
        goto continue_2; // <-- 'upstream'
      } // end IF

      // --> 'downstream'
      ACE_ASSERT (session_data_upstream_p->lock && session_data_message_p->lock);
      if (session_data_upstream_p->lock != session_data_message_p->lock)
          goto continue_2; // <-- already 'reset' by head module ?
      ACE_ASSERT (sessionDataLock_);
      //ACE_ASSERT (session_data_upstream_p->lock != sessionDataLock_);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_upstream_p->lock);
        if (likely (session_data_upstream_p->lock != sessionDataLock_))
        {
          result = sessionDataLock_->acquire ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          release_saved_upstream_lock_b = true;
        } // end IF
  
        const_cast<typename SessionMessageType::DATA_T::DATA_T*> (session_data_message_p)->lock =
          sessionDataLock_;

        if (likely (release_saved_upstream_lock_b))
        {
          result = sessionDataLock_->release ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX_T::release(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
        } // end IF
      } // end lock scope
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stream has been unlinked, reset upstream session data lock (is: %@)\n"),
                  inherited::mod_->name (),
                  sessionDataLock_));
      sessionDataLock_ = NULL;
      session_data_container_p->decrease (); session_data_container_p = NULL;

      // switch session data
      // *NOTE*: the distributor is one notable example where this fails (the nth (n > 1) time around)
      if (likely (sessionData_2))
      {
        sessionData_->decrease (); sessionData_ = NULL;
        sessionData_ = sessionData_2;
        sessionData_2 = NULL;
      } // end IF

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
      const typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;

      if (unlikely (aggregate_))
      {
        if (freeSessionData_ &&
            sessionData_)
          sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      // sanity check(s)
      if (unlikely (sessionData_)) // --> head modules initialize this in open()
        goto continue_3;

      sessionData_ =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->getR ());
      sessionData_->increase ();
      freeSessionData_ = true;

continue_3:
      // sanity check(s)
      ACE_ASSERT (sessionData_);

      session_data_p =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_->getR ());
      // *NOTE*: retain a handle to the original lock
      sessionDataLock_ = session_data_p->lock;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      try {
        this->dump_state ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Comon_IDumpState::dump_state(), continuing\n"),
                    inherited::mod_->name ()));
      }

      if (!linked_         &&
          freeSessionData_) // --> head modules finalize this in close()
      { ACE_ASSERT (sessionData_);
        sessionData_->decrease (); sessionData_ = NULL;
        freeSessionData_ = false;
        sessionDataLock_ = NULL;
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
      break;
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
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::handleProcessingError (const ACE_Message_Block* const messageBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleProcessingError"));

  // *TODO*: spruce this up a little more
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
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
DataMessageType*
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::allocateMessage (size_t requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_p = NULL;

  // sanity check(s)
  ACE_ASSERT (requestedSize_in);

  const typename SessionMessageType::DATA_T::DATA_T* session_data_p =
    (sessionData_ ? &sessionData_->getR () : NULL);
  if (unlikely (!session_data_p))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no session data, cannot set session id, continuing\n"),
                inherited::mod_->name ()));

  // *TODO*: remove type inference
  if (likely (allocator_))
  {
retry:
    try {
      // *TODO*: remove type inference
      ACE_ALLOCATOR_NORETURN (message_p,
                              static_cast<DataMessageType*> (allocator_->malloc (requestedSize_in)));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%Q), continuing\n"),
                  inherited::mod_->name (),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (unlikely (!message_p && !allocator_->block ()))
      goto retry;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      DataMessageType ((session_data_p ? session_data_p->sessionId : 0), // session id
                                       requestedSize_in));                               // size
  if (unlikely (!message_p))
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
  // *TODO*: remove type inference
  if (session_data_p)
    message_p->initialize (session_data_p->sessionId, // session id
                           NULL);                     // data block [NULL --> do not change]

  return message_p;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::handleMessage (ACE_Message_Block* messageBlock_in,
                                                bool& stopProcessing_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleMessage"));

  // sanity check
  ACE_ASSERT (messageBlock_in);

  bool forward_b = true;
  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_CONTROL:
    {
      ControlMessageType* control_message_p =
        static_cast<ControlMessageType*> (messageBlock_in);

      try {
        handleControlMessage (*control_message_p);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleControlMessage(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      }

      break;

error:
      messageBlock_in->release ();
      stopProcessing_out = true;
      forward_b = false;
      break;
    }
    case STREAM_MESSAGE_SESSION:
    {
      SessionMessageType* session_message_p =
        static_cast<SessionMessageType*> (messageBlock_in);

      enum Stream_SessionMessageType session_message_type =
        session_message_p->type ();

      // *IMPORTANT NOTE*: if linked, do not deliver session end messages; this
      //                   ensures that only one session end message is
      //                   delivered per session
      if (unlikely (linked_ &&
                    (session_message_type == STREAM_SESSION_MESSAGE_END)))
        break;

      bool post_process_b = false;
      // post-process UNLINK, END messages, pre-process all others
      if (unlikely ((session_message_type == STREAM_SESSION_MESSAGE_UNLINK) ||
                    (session_message_type == STREAM_SESSION_MESSAGE_END)))
        post_process_b = true;
      else
        OWN_TYPE_T::handleSessionMessage (session_message_p,
                                          forward_b);
      ACE_ASSERT (session_message_p && forward_b);
      // process message
      try {
        handleSessionMessage (session_message_p,
                              forward_b);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in handleSessionMessage(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      }
      // post-process UNLINK/END messages
      if (unlikely (post_process_b))
      {
        // *TODO*: currently, the session data will not be released (see below)
        //         if the module forwards the session end message itself
        //         --> memory leakage, resolve ASAP
        if (unlikely (!forward_b))
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: cannot post-process session message (type was: %d), continuing\n"),
                      inherited::mod_->name (),
                      session_message_type));
        else
        { ACE_ASSERT (session_message_p);
          OWN_TYPE_T::handleSessionMessage (session_message_p,
                                            forward_b);
          ACE_ASSERT (session_message_p && forward_b);
        } // end ELSE
      } // end IF

      // stop processing ?
      if (unlikely ((session_message_type == STREAM_SESSION_MESSAGE_ABORT) ||
                    (session_message_type == STREAM_SESSION_MESSAGE_END)))
        stopProcessing_out = true;

      break;

error_2:
      messageBlock_in->release ();
      stopProcessing_out = true;
      forward_b = false;
      break;
    }
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
    {
      DataMessageType* message_p =
        static_cast<DataMessageType*> (messageBlock_in);

      // *IMPORTANT NOTE*: in certain scenarios (e.g. concurrent configurations
      //                   with a network data source), data may start arriving
      //                   before the corresponding session has finished
      //                   initializing (i.e. before the SESSION_BEGIN message
      //                   has been processed by all modules). Due to
      //                   this race condition, session data may not be
      //                   available at this stage, and the downstream modules
      //                   may not behave as intended
      //                   --> prevent dispatch of data messages in this case
      // *WARNING*: this test does not work reliably, it only mitigates the race
      //            condition described
      // *TODO*: prevent this from occurring altogether (e.g. pre-buffer all
      //         'early' messages in the head module, introduce an intermediate
      //         state machine state 'in_session') to handle these situations
      // if (unlikely ((this == inherited::mod_->writer ()) &&
      //               !sessionData_))
      // {
      //   ACE_DEBUG ((LM_WARNING,
      //               ACE_TEXT ("%s: no session data (yet): dropping early/late data message, continuing\n"),
      //               inherited::mod_->name ()));
      //   goto error_3;
      // } // end IF

      try {
        handleDataMessage (message_p,
                           forward_b);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleDataMessage(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_3;
      }
      //if (unlikely (forward_b &&
      //              !configuration_->passData)) // *TODO*: remove type inference
      //{
      //  messageBlock_in->release ();
      //  forward_b = false;
      //} // end IF

      break;

error_3:
      messageBlock_in->release ();
      stopProcessing_out = true;
      forward_b = false;
      break;
    }
    case ACE_Message_Block::MB_USER:
    {
      try {
        handleUserMessage (messageBlock_in,
                           forward_b);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleUserMessage(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_4;
      }

      break;

error_4:
      messageBlock_in->release ();
      stopProcessing_out = true;
      forward_b = false;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message (type was: \"%s\"), aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
      messageBlock_in->release ();
      stopProcessing_out = true;
      forward_b = false;
      break;
    }
  } // end SWITCH

  // pass message downstream ?
  if (likely (forward_b))
  { // *NOTE*: fire-and-forget messageBlock_in here
    int result = inherited::put_next (messageBlock_in, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      messageBlock_in->release ();
      stopProcessing_out = true;
    } // end IF
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::putControlMessage (Stream_SessionId_t sessionId_in,
                                                    StreamControlType messageType_in,
                                                    bool sendUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::putControlMessage"));

  int result = -1;
  ControlMessageType* control_message_p = NULL;

  if (likely (allocator_))
  {
retry:
    try {
      // *IMPORTANT NOTE*: calloc() --> control message !
      ACE_ALLOCATOR_NORETURN (control_message_p,
                              static_cast<ControlMessageType*> (allocator_->calloc ()));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::calloc(), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    }

    // keep retrying ?
    if (unlikely (!control_message_p &&
                  !allocator_->block ()))
      goto retry;
  } // end IF
  else
    ACE_NEW_NORETURN (control_message_p,
                      ControlMessageType (messageType_in));
  if (unlikely (!control_message_p))
  {
    if (likely (allocator_))
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
  if (likely (allocator_))
    if (unlikely (!control_message_p->initialize (sessionId_in,
                                                  messageType_in)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_ControlMessage_T::initialize(%u,%d), aborting\n"),
                  inherited::mod_->name (),
                  sessionId_in,
                  messageType_in));
      control_message_p->release ();
      return false;
    } // end IF

  result = (sendUpStream_in ? inherited::reply (control_message_p, NULL)
                            : put (control_message_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::%s(): \"%m\", aborting\n"),
                inherited::mod_->name (),
                (sendUpStream_in ? ACE_TEXT ("reply") : ACE_TEXT ("put"))));
    control_message_p->release (); control_message_p = NULL;
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
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::putSessionMessage (SessionEventType eventType_in,
                                                    typename SessionMessageType::DATA_T*& sessionData_inout,
                                                    UserDataType* userData_in,
                                                    bool expedited_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::putSessionMessage"));

  // sanity check(s)
  const typename SessionMessageType::DATA_T::DATA_T* session_data_p =
    (sessionData_inout ? &sessionData_inout->getR () : NULL);
  if (unlikely (!session_data_p))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no session data, cannot set session id, continuing\n"),
                inherited::mod_->name ()));

  int result = -1;
  SessionMessageType* session_message_p = NULL;

  if (likely (allocator_))
  {
retry:
    try {
      // *IMPORTANT NOTE*: 0 --> session message !
      ACE_ALLOCATOR_NORETURN (session_message_p,
                              static_cast<SessionMessageType*> (allocator_->malloc (0)));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(0), aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    }

    // keep retrying ?
    if (unlikely (!session_message_p &&
                  !allocator_->block ()))
      goto retry;
  } // end IF
  else
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType ((session_data_p ? session_data_p->sessionId : -1),
                                          eventType_in,
                                          sessionData_inout,
                                          userData_in,
                                          expedited_in));
  if (unlikely (!session_message_p))
  {
    if (likely (allocator_))
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
  if (likely (allocator_))
    session_message_p->initialize ((session_data_p ? session_data_p->sessionId : -1),
                                   eventType_in,
                                   sessionData_inout,
                                   userData_in,
                                   expedited_in);

  result = put (session_message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::put(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    session_message_p->release (); session_message_p = NULL;
    goto error;
  } // end IF

  return true;

error:
  if (sessionData_inout)
  {
    sessionData_inout->decrease (); sessionData_inout = NULL;
  } // end IF

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::notify (SessionEventType sessionEvent_in,
                                         bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::notify"));

  Stream_SessionId_t session_id = 0;
  if (likely (sessionData_))
  {
    const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      sessionData_->getR ();
    session_id = session_data_r.sessionId;
  } // end IF

  INOTIFY_T* inotify_p = dynamic_cast<INOTIFY_T*> (inherited::mod_);
  if (unlikely (!inotify_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_ISessionNotify_T>(0x%@) failed, returning\n"),
                inherited::mod_->name (),
                inherited::mod_));
    return;
  } // end IF
  try {
    inotify_p->notify (session_id,
                       sessionEvent_in,
                       expedite_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_ISessionNotify_T::notify(%u,%d), continuing\n"),
                inherited::mod_->name (),
                session_id,
                sessionEvent_in));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  StreamControlType,
                  SessionEventType,
                  UserDataType>::control (int messageType_in,
                                          bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::control"));

  ACE_UNUSED_ARG (highPriority_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       messageType_in,                     // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = put (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::put(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
}
