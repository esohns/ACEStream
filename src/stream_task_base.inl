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

#include "ace/Reactor.h"
#include "ace/Message_Block.h"
#include "ace/Time_Value.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_defines.h"
#include "stream_session_message_base.h"

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::Stream_TaskBase_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_DEFAULT_HEAD_THREAD_NAME), // thread name
              STREAM_MODULE_TASK_GROUP_ID,                                   // group id
              1,                                                             // # thread(s)
              false,                                                         // auto-start ?
              ///////////////////////////
              &queue_)                                                       // queue handle
 , lock_ ()
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::Stream_TaskBase_T"));

}

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::~Stream_TaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::~Stream_TaskBase_T"));

  int result = 0;
  result = queue_.flush ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
  else if (result)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("flushed %d message(s)...\n"),
                result));

//   // *TODO*: check if this sequence actually works...
//   queue_.deactivate ();
//   queue_.wait ();
}

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::initialize (const void* configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::initialize"));

  ACE_UNUSED_ARG (configuration_in);

  return true;
}

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleSessionMessage"));

  // initialize return value(s)
  passMessageDownstream_out = true;

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // *NOTE*: the default behavior is to simply dump the state at the
  // end of a session...
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    case STREAM_SESSION_STEP:
      break;
    case STREAM_SESSION_END:
    {
      try
      {
        // OK: dump some status info...
        dump_state ();
      }
      catch (...)
      {
        if (inherited::module ())
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("module \"%s\": caught exception in dump_state(), continuing\n"),
                      inherited::name ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in dump_state(), continuing\n")));
      }

      break;
    }
    case STREAM_SESSION_STATISTIC:
      break;
    case STREAM_SESSION_MESSAGE_MAP:
    case STREAM_SESSION_INVALID:
    case STREAM_SESSION_MAX:
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("invalid/unknown session message type (was: %d)\n"),
                  message_inout->type ()));
      break;
    }
  } // end SWITCH
}

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::handleProcessingError (const ACE_Message_Block* const message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleProcessingError"));

  if (inherited::module ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module: \"%s\": failed to process message %@, continuing\n"),
                inherited::name (),
                message_in));
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to process message %@, continuing\n"),
                message_in));
}

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::dump_state"));

//   if (inherited::module ())
//     ACE_DEBUG ((LM_WARNING,
//                 ACE_TEXT (" ***** MODULE: \"%s\" has not implemented the dump_state() API *****\n"),
//                 inherited::name ()));
//   else
//     ACE_DEBUG ((LM_WARNING,
//                 ACE_TEXT ("dump_state() API not implemented\n")));
}

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::handleMessage (ACE_Message_Block* messageBlock_in,
                                                       bool& stopProcessing_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleMessage"));

  int result = -1;

  // sanity check
  ACE_ASSERT (messageBlock_in);

  // initialize return value(s)
  stopProcessing_out = false;

  // default behavior is to pass EVERYTHING downstream...
  bool passMessageDownstream = true;
  switch (messageBlock_in->msg_type ())
  {
    // DATA handling
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
    {
      ProtocolMessageType* message_p =
        dynamic_cast<ProtocolMessageType*> (messageBlock_in);
//       // *OPTIMIZATION*: not as safe, but (arguably) a lot faster !...
//       message = static_cast<RPG_Stream_MessageBase*>(mb_in);
      if (!message_p)
      {
        std::string type_string;
        Stream_MessageBase::MessageType2String (messageBlock_in->msg_type (),
                                                type_string);
        Stream_Module_t* module_p = inherited::module ();
        if (module_p)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("module \"%s\": dynamic_cast<ProtocolMessageType*>(%@) failed (type was: \"%s\"), aborting\n"),
                      module_p->name (),
                      messageBlock_in,
                      ACE_TEXT (type_string.c_str ())));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("dynamic_cast<ProtocolMessageType*>(%@) failed (type was: \"%s\"), aborting\n"),
                      messageBlock_in,
                      ACE_TEXT (type_string.c_str ())));

        // clean up
        messageBlock_in->release ();

        return;
      } // end IF

      // OK: process data...
      try
      {
        // invoke specific implementation...
        // *IMPORTANT NOTE*: invoke OWN implementation here, otherwise, the (ld)
        //                   linker complains about a missing reference to
        //                   StreamITaskBase::handleDataMessage...
        this->handleDataMessage (message_p,
                                 passMessageDownstream);
      }
      catch (...)
      {
        if (inherited::module ())
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("module \"%s\": caught an exception in handleDataMessage() (message ID: %u), continuing\n"),
                      ACE_TEXT (inherited::name ()),
                      message_p->getID ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleDataMessage() (message ID: %u), continuing\n"),
                      message_p->getID ()));
      }

      break;
    }
    // *NOTE*: anything that is NOT data is defined as a control message...
    default:
    {
      // OK: process control message...
      try
      {
        // invoke specific implementation...
        handleControlMessage (messageBlock_in,
                              stopProcessing_out,
                              passMessageDownstream);
      }
      catch (...)
      {
        std::string type_string;
        Stream_MessageBase::MessageType2String (messageBlock_in->msg_type (),
                                                type_string);

        if (inherited::module ())
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("module \"%s\": caught an exception in handleControlMessage() (type was: \"%s\"), continuing\n"),
                      inherited::name (),
                      ACE_TEXT (type_string.c_str ())));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleControlMessage() (type: \"%s\"), continuing\n"),
                      ACE_TEXT (type_string.c_str ())));
      }

      break;
    }
  } // end SWITCH

  // pass message downstream (if there is a stream)
  if (passMessageDownstream)
  {
    // *NOTE*: tasks that are not part of a stream have no notion of the concept
    if (!inherited::module ())
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("cannot put_next(): not a module, continuing\n")));

      // clean up
      messageBlock_in->release ();
    } // end IF
    else
    {
      result = inherited::put_next (messageBlock_in, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to put_next(): \"%m\", continuing\n")));

        // clean up
        messageBlock_in->release ();
      } // end IF
    } // end IF
  } // end IF
}

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_TaskBase_T<TaskSynchStrategyType,
                  TimePolicyType,
                  SessionMessageType,
                  ProtocolMessageType>::handleControlMessage (ACE_Message_Block* controlMessage_in,
                                                              bool& stopProcessing_out,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBase_T::handleControlMessage"));

  // initialize return value(s)
  stopProcessing_out = false;
  passMessageDownstream_out = true;

  switch (controlMessage_in->msg_type ())
  {
    case STREAM_MESSAGE_SESSION:
    {
      SessionMessageType* session_message_p = NULL;
      // downcast message
      session_message_p = dynamic_cast<SessionMessageType*> (controlMessage_in);
      if (!session_message_p)
      {
        if (inherited::module ())
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("module \"%s\": dynamic_cast<SessionMessageType*>(%@) failed (type was: %d) ,aborting\n"),
                      ACE_TEXT (inherited::name ()),
                      controlMessage_in,
                      controlMessage_in->msg_type ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("dynamic_cast<SessionMessageType*>(%@) failed (type was: %d) ,aborting\n"),
                      controlMessage_in,
                      controlMessage_in->msg_type ()));

        // clean up
        passMessageDownstream_out = false;
        controlMessage_in->release ();

        break;
      } // end IF

      // OK: process session message...
      Stream_SessionMessageType session_message_type =
        session_message_p->type ();
      try
      {
        // invoke specific implementation...
        handleSessionMessage (session_message_p,
                              passMessageDownstream_out);
      }
      catch (...)
      {
        if (inherited::module ())
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught an exception in handleSessionMessage(), continuing\n"),
                      inherited::name ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught an exception in handleSessionMessage(), continuing\n")));
      }

      // *NOTE*: if this was a Stream_SessionMessage::SESSION_END, stop
      //         processing (see above) !
      if (session_message_type == STREAM_SESSION_END)
        stopProcessing_out = true;

      break;
    }
    default:
    {
      // *NOTE*: if someone defines their own control message type and enqueues
      //         it on the stream, it will land here (this is just a sanity
      //         check warning...)
      if (inherited::module ())
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("module \"%s\": received an unknown control message (type: %d), continuing\n"),
                    inherited::name (),
                    controlMessage_in->msg_type ()));
      else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("received an unknown control message (type: %d), continuing\n"),
                    controlMessage_in->msg_type ()));
      break;
    }
  } // end SWITCH
}
