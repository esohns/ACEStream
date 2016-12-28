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

#include <limits>
#include <string>

#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/OS.h>
#include <ace/Time_Value.h>

#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::Stream_TaskBaseAsynch_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::Stream_TaskBaseAsynch_T"));

  // *TODO*: pass this in from outside
  inherited::threadCount_ = 1;

  // set group ID for worker thread(s)
  // *TODO*: pass this in from outside
  inherited::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::~Stream_TaskBaseAsynch_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::~Stream_TaskBaseAsynch_T"));

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
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::open (void* args_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::open"));

  ACE_UNUSED_ARG (args_in);

  int result = -1;

  // step1: (re-)activate() the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state
  //         (hopefully, this is what is intended)
  //         The second time (i.e. the task has been stopped/started, the queue
  //         will have been deactivated in the process, and getq() (see svc()
  //         below) will fail (ESHUTDOWN)
  //         --> (re-)activate() the queue
  result = inherited::queue_.activate ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // step2: spawn worker thread(s)
  result = inherited::open (NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  //for (unsigned int i = 0;
  //     i < inherited::threadIDs_.size ();
  //     ++i)
  //  string_stream << ACE_TEXT_ALWAYS_CHAR ("#") << (i + 1)
  //                << ACE_TEXT_ALWAYS_CHAR (" ")
  //                << inherited::threadIDs_[i].thread_id ()
  //                << ACE_TEXT_ALWAYS_CHAR ("\n");
  //if (inherited::mod_)
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s: spawned worker thread(s) (\"%s\", group: %d):\n%s"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (inherited::threadName_.c_str ()),
  //              inherited::grp_id (),
  //              ACE_TEXT (string_stream.str ().c_str ())));
  //else
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("spawned worker thread(s) (\"%s\", group: %d):\n%s"),
  //              ACE_TEXT (inherited::threadName_.c_str ()),
  //              inherited::grp_id (),
  //              ACE_TEXT (string_stream.str ().c_str ())));

  goto done;

error:
  result = inherited::queue_.deactivate ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", aborting\n")));

done:
  return result;
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
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::close"));

  int result = -1;

  // *IMPORTANT NOTE*: this method may be invoked
  //                   - by an external thread closing down the active object
  //                     --> should NEVER happen as a module !
  //                   - by the worker thread after returning from svc()
  //                     --> in this case, this should be a NOP
  switch (arg_in)
  {
    // called from ACE_Task_Base on clean-up
    case 0:
    {
      if (inherited::mod_)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: worker thread (ID: %t) leaving...\n"),
                    inherited::mod_->name ()));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("worker thread (ID: %t) leaving...\n")));

      if (inherited::thr_count_ == 0) // last thread ?
      {
        // *NOTE*: deactivate the queue so it does not accept new data
        result = inherited::msg_queue_->deactivate ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));
        result = inherited::msg_queue_->flush ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
        else if (result && inherited::mod_)
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: flushed %d message(s)...\n"),
                      inherited::mod_->name (),
                      result));
      } // end IF

      // don't (need to) do anything
      break;
    }
    case 1:
    {
      // *IMPORTANT NOTE*: SHOULD NEVER GET HERE
      // --> module_closed() hook is implemented below !!!
      ACE_ASSERT (false);
      ACE_NOTREACHED (return -1;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument: %u, aborting\n"),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
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
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::module_closed"));

  if (inherited::thr_count_ > 0)
    inherited::stop (true); // wait ?

  return 0;
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
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                                ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::put"));

  if (messageBlock_in->msg_type () == ACE_Message_Block::MB_FLUSH)
  {
    // *TODO*: support selective flushing via ControlMessageType
    int result = inherited::msg_queue_->flush ();
    if (result == -1)
    {
      if (inherited::mod_)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue_T::flush(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue_T::flush(): \"%m\", aborting\n")));
    } // end IF
    else if (result > 0)
    {
      if (inherited::mod_)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: flushed %d messages\n"),
                    inherited::mod_->name ()));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("flushed %d messages\n")));
    } // end ELSE IF

    // clean up
    messageBlock_in->release ();

    return (result > 0 ? 0 : result);
  } // end IF

  // drop the message into the queue
  return inherited::putq (messageBlock_in, timeout_in);
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
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::svc"));

  if (inherited::mod_)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: worker thread (ID: %t) starting...\n"),
                inherited::mod_->name ()));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("(%s): worker thread (ID: %t) starting...\n"),
                ACE_TEXT (inherited::threadName_.c_str ())));

  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block::ACE_Message_Type message_type;
  int result = -1;
  int result_2 = -1;
  int error = -1;
  bool stop_processing = false;

  do
  {
    result = inherited::getq (message_block_p, NULL);
    if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread (ID: %t) failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    message_type = message_block_p->msg_type ();
    if (message_type == ACE_Message_Block::MB_STOP)
    {
      if (inherited::thr_count_ > 1)
      {
        result_2 = inherited::putq (message_block_p, NULL);
        if (result_2 == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
        else
          result = 0;
      } // end IF
      else
        result = 0;

      // clean up
      message_block_p->release ();

      break; // done
    } // end IF

    // process manually
    inherited::handleMessage (message_block_p,
                              stop_processing);
    if (stop_processing &&
        inherited::thr_count_)
      inherited::stop (false,  // wait ?
                       false); // N/A

    // initialize
    message_block_p = NULL;
  } while (true);

  return result;
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
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::waitForIdleState"));

  // delegate this to the queue
  try {
    inherited::queue_.waitForIdleState ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IMessageQueue::waitForIdleState, continuing\n")));
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
void
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionEventType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_END:
    {
      // *IMPORTANT NOTE*: derived classes need to follow this pattern so that
      //                   no data messages get processed after the session end
      //                   has been signalled
      // *NOTE*: that stop() cannot wait, as this is the workers' thread
      //         context (i.e. would deadlock)
      //if (inherited::thr_count_)
      //  inherited::stop (false, // wait for completion ?
      //                   true); // locked access ?

      break;
    }
    default:
      break;
  } // end SWITCH
}
