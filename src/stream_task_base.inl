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
          typename SessionEventType>
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::Stream_TaskBase_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_DEFAULT_HEAD_THREAD_NAME), // thread name
              STREAM_MODULE_TASK_GROUP_ID,                                   // group id
              1,                                                             // # thread(s)
              false,                                                         // auto-start ?
              ////////////////////////////
              //NULL)                                                          // queue handle
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)                                                       // queue handle
 , configuration_ (NULL)
 , isInitialized_ (false)
 //, lock_ ()
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::Stream_TaskBase_T"));

  //inherited::msg_queue (&queue_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::~Stream_TaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::~Stream_TaskBase_T"));

  int result = -1;

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
                  ACE_TEXT ("\"%s\": flushed %d message(s)...\n"),
                  inherited::mod_->name (),
                  result));
    else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("flushed %d message(s)...\n"),
                  result));
  } // end ELSE IF

  //inherited::msg_queue (NULL);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
const ConfigurationType&
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
bool
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
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
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleControlMessage"));

  ACE_UNUSED_ARG (message_in);

  // *NOTE*: a stub
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // *NOTE*: a stub
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleSessionMessage"));

  // initialize return value(s)
  passMessageDownstream_out = true;

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // *NOTE*: the default behavior is to simply dump the module state at the end
  //         of a session
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    case STREAM_SESSION_MESSAGE_LINK:
    case STREAM_SESSION_MESSAGE_STEP:
      break;
    case STREAM_SESSION_MESSAGE_END:
    {
      try {
        dump_state ();
      } catch (...) {
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("module \"%s\": caught exception in dump_state(), continuing\n"),
                      inherited::mod_->name ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in dump_state(), continuing\n")));
      }

      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
      break;
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown session message type (was: %d)\n"),
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
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::handleProcessingError (const ACE_Message_Block* const messageBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleProcessingError"));

  if (inherited::mod_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module: \"%s\": failed to process message %@, continuing\n"),
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
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::dump_state"));

//   if (inherited::mod_)
//     ACE_DEBUG ((LM_WARNING,
//                 ACE_TEXT (" ***** MODULE: \"%s\" has not implemented the dump_state() API *****\n"),
//                 inherited::mod_->name ()));
//   else
//     ACE_DEBUG ((LM_WARNING,
//                 ACE_TEXT ("dump_state() API not implemented\n")));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::handleMessage (ACE_Message_Block* messageBlock_in,
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
                      ACE_TEXT ("module \"%s\": caught an exception in handleDataMessage() (ID was: %u), continuing\n"),
                      inherited::mod_->name (),
                      message_p->getID ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleDataMessage() (ID was: %u), continuing\n"),
                      message_p->getID ()));
      }

      break;
    }
    case ACE_Message_Block::MB_PROTO:
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
                      ACE_TEXT ("module \"%s\": caught an exception in handleUserMessage() (type was: \"%s\"), continuing\n"),
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
                    ACE_TEXT ("module \"%s\": received an unknown message (type was: %d), continuing\n"),
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
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::handleUserMessage (ACE_Message_Block* messageBlock_in,
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
        stopProcessing_out = true;

      break;
    }
    default:
    {
      if (inherited::mod_)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("module \"%s\": received an unknown user message (type was: %d), continuing\n"),
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
          typename SessionEventType>
void
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::notify (SessionEventType sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::notify"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  INOTIFY_T* inotify_p = dynamic_cast<INOTIFY_T*> (inherited::mod_);
  if (!inotify_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_ISessionNotify_T*>(%@) failed, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
  try { // *TODO*: retrieve session id
    inotify_p->notify (-1,
                       sessionEvent_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module \"%s\": caught exception in Stream_ISessionNotify_T::notify(), continuing\n"),
                inherited::mod_->name ()));
  }
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
int
Stream_TaskBase_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                          ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::put"));

  return inherited::put_next (messageBlock_in, timeValue_in);
}
