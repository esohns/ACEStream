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

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS.h"
#include "ace/Time_Value.h"

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
 , threadIDs_ ()
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_hthread_t handle = ACE_INVALID_HANDLE;
  for (THREAD_IDS_ITERATOR_T iterator = threadIDs_.begin ();
       iterator != threadIDs_.end ();
       ++iterator)
  {
    handle = (*iterator).handle ();
    if (handle != ACE_INVALID_HANDLE)
      if (!::CloseHandle (handle))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                    handle,
                    ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
  } // end FOR
#endif
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
  ACE_thread_t* thread_ids_p = NULL;
  ACE_hthread_t* thread_handles_p = NULL;
  const char** thread_names_p = NULL;
  char* thread_name_p = NULL;
  std::string buffer;
  std::ostringstream converter;
  std::ostringstream string_stream;
  ACE_Thread_ID thread_id;

  ACE_NEW_NORETURN (thread_ids_p,
                    ACE_thread_t[inherited::threadCount_]);
  if (!thread_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                (sizeof (ACE_thread_t) * inherited::threadCount_)));
    goto error;
  } // end IF
  ACE_OS::memset (thread_ids_p, 0, sizeof (thread_ids_p));
  ACE_NEW_NORETURN (thread_handles_p,
                    ACE_hthread_t[inherited::threadCount_]);
  if (!thread_handles_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                (sizeof (ACE_hthread_t) * inherited::threadCount_)));

    // clean up
    delete [] thread_ids_p;

    goto error;
  } // end IF
  ACE_OS::memset (thread_handles_p, 0, sizeof (thread_handles_p));
  ACE_NEW_NORETURN (thread_names_p,
                    const char*[inherited::threadCount_]);
  if (!thread_names_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                (sizeof (const char*) * inherited::threadCount_)));

    // clean up
    delete [] thread_ids_p;
    delete [] thread_handles_p;

    goto error;
  } // end IF
  ACE_OS::memset (thread_names_p, 0, sizeof (thread_names_p));
  for (unsigned int i = 0;
       i < inherited::threadCount_;
       i++)
  {
    thread_name_p = NULL;
    ACE_NEW_NORETURN (thread_name_p,
                      char[BUFSIZ]);
    if (!thread_name_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                  (sizeof (char) * BUFSIZ)));

      // clean up
      delete [] thread_ids_p;
      delete [] thread_handles_p;
      for (unsigned int j = 0; j < i; j++)
        delete [] thread_names_p[j];
      delete [] thread_names_p;

      goto error;
    } // end IF
    ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << (i + 1);
    buffer = inherited::threadName_;
    buffer += ACE_TEXT_ALWAYS_CHAR (" #");
    buffer += converter.str ();
    ACE_OS::strcpy (thread_name_p,
                    buffer.c_str ());
    thread_names_p[i] = thread_name_p;
  } // end FOR
  result =
    ACE_Task_Base::activate ((THR_NEW_LWP      |
                              THR_JOINABLE     |
                              THR_INHERIT_SCHED),                        // flags
                             static_cast<int> (inherited::threadCount_), // # threads
                             0,                                          // force active ?
                             ACE_DEFAULT_THREAD_PRIORITY,                // priority
                             inherited::grp_id (),                       // group id (see above)
                             NULL,                                       // task base
                             thread_handles_p,                           // thread handle(s)
                             NULL,                                       // stack(s)
                             NULL,                                       // stack size(s)
                             thread_ids_p,                               // thread id(s)
                             thread_names_p);                            // thread name(s)
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task_Base::activate(%u): \"%m\", aborting\n"),
                inherited::threadCount_));

    // clean up
    delete [] thread_ids_p;
    delete [] thread_handles_p;
    for (unsigned int i = 0; i < inherited::threadCount_; i++)
      delete [] thread_names_p[i];
    delete [] thread_names_p;

    goto error;
  } // end IF

  for (unsigned int i = 0;
       i < inherited::threadCount_;
       ++i)
  {
    string_stream << ACE_TEXT_ALWAYS_CHAR ("#") << (i + 1)
                  << ACE_TEXT_ALWAYS_CHAR (" ")
                  << thread_ids_p[i]
                  << ACE_TEXT_ALWAYS_CHAR ("\n");

    // clean up
    delete [] thread_names_p[i];

    thread_id.handle (thread_handles_p[i]);
    thread_id.id (thread_ids_p[i]);
    threadIDs_.push_back (thread_id);
  } // end FOR
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

  // clean up
  delete [] thread_ids_p;
  delete [] thread_handles_p;
  delete [] thread_names_p;

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
        //// *NOTE*: deactivate the queue so it does not accept new data
        //int result = inherited::msg_queue_->deactivate ();
        //if (result == -1)
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));
        result = inherited::msg_queue_->flush ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
        //else if (inherited::mod_)
        //  ACE_DEBUG ((LM_DEBUG,
        //              ACE_TEXT ("\"%s\": flushed %d message(s)...\n"),
        //              inherited::mod_->name (),
        //              result));
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
    if (stop_processing) // *NOTE*: message_block_p has already been processed
      inherited::stop (false); // wait ?

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
      inherited::stop (false); // wait ?
      break;
    }
    default:
      break;
  } // end SWITCH
}
