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

#include <deque>

#include "stream_iallocator.h"
#include "stream_macros.h"
#include "stream_session_data_base.h"
#include "stream_session_message_base.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::Stream_Base_T ()
// *TODO*: use default ctor and rely on init/fini() ?
 : inherited (NULL, // argument to module open()
              NULL, // no head module --> allocate !
              NULL) // no tail module --> allocate !
 , availableModules_ ()
 , isInitialized_ (false)
 , allocator_ (NULL)
 , lock_ ()
 , sessionData_ (NULL)
 , state_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::Stream_Base_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::~Stream_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::~Stream_Base_T"));

  // clean up
  if (sessionData_)
    delete sessionData_;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::reset"));

  // sanity check
  // *TODO*: cannot call isRunning(), as the stream may not be initialized
//  ACE_ASSERT (!isRunning ());

  // pop/close all modules
  // *NOTE*: will implicitly (blocking !) wait for any active worker threads
  if (!finalize ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::finalize(), aborting\n")));
    return false;
  } // end IF

  // - reset reader/writers tasks for ALL modules
  // - re-initialize head/tail modules
  initialize ();

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  int result = -1;

  // sanity check(s)
  // *TODO*: cannot call isRunning(), as the stream may not be initialized
//  ACE_ASSERT (!isRunning ());

  if (isInitialized_)
  {
    // *NOTE*: fini() calls close(), resetting the writer/reader tasks
    //         of all enqueued modules --> reset them !
    IMODULE_T* imodule_p = NULL;
    for (MODULE_CONTAINER_ITERATOR_T iterator = availableModules_.begin ();
         iterator != availableModules_.end ();
         iterator++)
    {
      // need a downcast...
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, returning\n"),
                    (*iterator)->name ()));
        return;
      } // end IF
      try
      {
        imodule_p->reset ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_IModule::reset(), continuing\n")));
      }
    } // end FOR
  } // end IF

  // allocate session data
  if (sessionData_)
  {
    delete sessionData_;
    sessionData_ = NULL;
  } // end IF
  ACE_NEW_NORETURN (sessionData_,
                    SessionDataType ());
  if (!sessionData_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  // *TODO*: remove type inferences
  sessionData_->lock = &lock_;
  state_.currentSessionData = sessionData_;

  try
  {
    result = inherited::open (NULL,  // argument to module open()
                              NULL,  // no head module --> allocate !
                              NULL); // no tail module --> allocate !
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ACE_Stream::open(), continuing\n")));
    result = -1;
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::open(): \"%m\", continuing\n")));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::finalize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finalize"));

  int result = -1;

  // delegate this to base class close(ACE_Module_Base::M_DELETE_NONE)
  try
  {
    // *NOTE*: this will implicitly:
    // - unwind the stream, which pop()s all (pushed) modules
    // --> pop()ing a module will close() it
    // --> close()ing a module will module_closed() and flush() its tasks
    // --> flush()ing a task will close() its queue
    // --> close()ing a queue will deactivate() and flush() it
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ACE_Stream::close(M_DELETE_NONE), continuing\n")));
    result = -1;
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::close(M_DELETE_NONE): \"%m\", aborting\n")));

  return (result == 0);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::start"));

  int result = -1;

  // sanity check(s)
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (isRunning ())
    return; // nothing to do

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return;
  } // end IF

  ISTREAM_CONTROL_T* control_impl_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try
  {
    control_impl_p->start ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::start(), returning\n"),
                module_p->name ()));
    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::stop (bool waitForCompletion_in,
                                          bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  int result = -1;
  MODULE_T* module_p = NULL;
  ISTREAM_CONTROL_T* control_impl_p = NULL;

  if (!isRunning ())
    goto wait;

  // delegate to the head module, skip over ACE_Stream_Head...
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return;
  } // end IF

  // *WARNING*: cannot flush(), as this deactivates() the queue as well, which
  //            causes mayhem for any (blocked) worker(s)
  // *TODO*: consider optimizing this
  //module->writer ()->flush ();
<<<<<<< HEAD

=======
>>>>>>> 73b3dea26f55af37d8c25b7389d4f966f2e2d7a8
  control_impl_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try
  {
    control_impl_p->stop (waitForCompletion_in);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::stop(), returning\n"),
                module_p->name ()));
    return;
  }

wait:
  if (waitForCompletion_in)
    waitForCompletion ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isRunning"));

  int result = -1;

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = const_cast<OWN_TYPE_T*> (this)->top (module_p);
  if ((result == -1) || !module_p)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", aborting\n")));
    return false;
  } // end IF

  ISTREAM_CONTROL_T* control_impl_p = NULL;
  control_impl_p = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    // *NOTE*: perhaps not all modules have been enqueued yet ?
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, aborting\n"),
//                module_p->name ()));
    return false;
  } // end IF

  try
  {
    return control_impl_p->isRunning ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::isRunning(), aborting\n"),
                module_p->name ()));
  }

  return false;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::flush ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::flush"));

  int result = -1;

  // writer (inbound) side
  MODULE_CONTAINER_T modules;
  Stream_Queue_t* queue_p = NULL;
  Stream_Task_t* task_p = NULL;
  const Stream_Module_t* module_p = NULL;
  ACE_Time_Value one_second (1, 0);
  size_t message_count = 0;
  for (ITERATOR_T iterator (*this);
       (iterator.next (module_p) != 0);
       iterator.advance ())
  {
    // skip stream tail (last module)
    if (module_p == inherited::tail ())
      continue;

    modules.push_front (const_cast<MODULE_T*> (module_p));
    task_p = const_cast<MODULE_T*> (module_p)->writer ();
    ACE_ASSERT (task_p);
    queue_p = task_p->msg_queue ();
    ACE_ASSERT (queue_p);
    do
    {
      //result = queue_p->wait ();
      message_count = queue_p->message_count ();
      if (!message_count) break;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s writer: waiting to process ~%d byte(s) (in %u message(s))...\n"),
                  module_p->name (),
                  queue_p->message_bytes (), message_count));
      result = ACE_OS::sleep (one_second);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    module_p->name (),
                    &one_second));
    } while (true);

    module_p = NULL;
  } // end FOR

  // reader (outbound) side
  for (MODULE_CONTAINER_ITERATOR_T iterator = modules.begin ();
       iterator != modules.end ();
       iterator++)
  {
    task_p = (*iterator)->reader ();
    ACE_ASSERT (task_p);
    queue_p = task_p->msg_queue ();
    ACE_ASSERT (queue_p);
    do
    {
      //result = queue_p->wait ();
      message_count = queue_p->message_count ();
      if (!message_count) break;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s reader: waiting to process ~%d byte(s) (in %u message(s))...\n"),
                  module_p->name (),
                  queue_p->message_bytes (), message_count));
      result = ACE_OS::sleep (one_second);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    module_p->name (),
                    &one_second));
    } while (true);
  } // end FOR
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::pause"));

  int result = -1;

  //// sanity check
  //ACE_ASSERT (isRunning ());

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return;
  } // end IF

  ISTREAM_CONTROL_T* control_impl_p = NULL;
  control_impl_p = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try
  {
    control_impl_p->pause ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::pause(), returning\n"),
                module_p->name ()));
    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::rewind"));

  int result = -1;

  // sanity check
  // *TODO*
  if (isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("currently running, returning\n")));
    return;
  } // end IF

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return;
  } // end IF

  ISTREAM_CONTROL_T* control_impl_p = NULL;
  control_impl_p = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try
  {
    control_impl_p->rewind ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::rewind(), returning\n"),
                module_p->name ()));
    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
const StatusType&
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::status () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::status"));

  int result = -1;

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = const_cast<OWN_TYPE_T*> (this)->top (module_p);
  if ((result == -1) || !module_p)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return StatusType ();
  } // end IF

  ISTREAM_CONTROL_T* control_impl_p = NULL;
  control_impl_p = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, returning\n"),
                module_p->name ()));
    return StatusType ();
  } // end IF

  try
  {
    return control_impl_p->status ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::status(), returning\n"),
                module_p->name ()));
    return StatusType ();
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::waitForCompletion ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::waitForCompletion"));

  int result = -1;

  // *NOTE*: the logic here is this:
  //         step1: wait for processing (message generation) to finish
  //         step2: wait for any pipelined messages to 'flush'
  MODULE_CONTAINER_T modules;
  modules.push_front (inherited::head ());

  // step1: get head module, skip over ACE_Stream_Head
  ITERATOR_T iterator (*this);
  result = iterator.advance ();
  if (result == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));
    return;
  } // end IF
  const MODULE_T* module_p = NULL;
  result = iterator.next (module_p);
  if (result == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));
    return;
  } // end IF

  // sanity check: head == tail ?
  // --> reason: no modules have been push()ed (yet)
  // --> stream hasn't been intialized (at all: too many connections ?)
  // --> nothing to do
  if (module_p == inherited::tail ())
    return;

  modules.push_front (const_cast<MODULE_T*> (module_p));
  // need to downcast
  ISTREAM_CONTROL_T* control_impl_p = NULL;
  control_impl_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (const_cast<MODULE_T*> (module_p)->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try
  {
    // wait for state switch (xxx --> FINISHED) / any head module threads
    control_impl_p->waitForCompletion ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::waitForCompletion (), returning\n"),
                module_p->name ()));
    return;
  }

  Stream_Task_t* task_p = NULL;
  for (iterator.advance ();
       (iterator.next (module_p) != 0);
       iterator.advance ())
  {
    // skip stream tail (last module)
    if (module_p == inherited::tail ())
      continue;

    modules.push_front (const_cast<MODULE_T*> (module_p));
    // OK: got a handle... wait
    task_p = const_cast<MODULE_T*> (module_p)->writer ();
    ACE_ASSERT (task_p);
    result = task_p->wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s writer: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                  module_p->name ()));

    module_p = NULL;
  } // end FOR

  // step2: wait for any pipelined messages to flush...
  Stream_Queue_t* queue_p = NULL;
  ACE_Time_Value one_second (1, 0);
  unsigned int message_count = 0;
  for (MODULE_CONTAINER_ITERATOR_T iterator2 = modules.begin ();
       iterator2 != modules.end ();
       iterator2++)
  {
    task_p = (*iterator2)->reader ();
    ACE_ASSERT (task_p);
    queue_p = task_p->msg_queue ();
    ACE_ASSERT (queue_p);
    do
    {
      //result = queue_p->wait ();
      message_count = queue_p->message_count ();
      if (!message_count) break;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s reader: waiting to process ~%d byte(s) (in %u message(s))...\n"),
                  (*iterator2)->name (),
                  queue_p->message_bytes (), message_count));
      result = ACE_OS::sleep (one_second);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    (*iterator2)->name (),
                    &one_second));
    } while (true);
    result = task_p->wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s reader: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                  (*iterator2)->name ()));
  } // end FOR
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
const StateType&
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::state"));

  return state_;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::dump_state"));

  std::string stream_layout;

  const MODULE_T* module_p = NULL;
  const MODULE_T* tail_p = const_cast<OWN_TYPE_T*> (this)->tail ();
  ACE_ASSERT (tail_p);
  for (ITERATOR_T iterator (*this);
       (iterator.next (module_p) != 0);
       iterator.advance ())
  {
    stream_layout.append (ACE_TEXT_ALWAYS_CHAR (module_p->name ()));

    // avoid trailing "-->"
    //if (module_p != tail_p) // <-- does not work for some reason...
    if (ACE_OS::strcmp (module_p->name (), tail_p->name ()) != 0)
      stream_layout += ACE_TEXT_ALWAYS_CHAR (" --> ");

    module_p = NULL;
  } // end FOR

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stream layout: \"%s\"\n"),
              ACE_TEXT (stream_layout.c_str ())));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
const SessionDataType&
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
//              ProtocolMessageType>::get () const
              ProtocolMessageType>::sessionData () const
{
//  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::get"));
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::sessionData"));

  // sanity check(s)
  ACE_ASSERT (sessionData_);

  return *sessionData_;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
//              ProtocolMessageType>::get () const
              ProtocolMessageType>::link (STREAM_T& us)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Stream.cpp:505

  // *WARNING*: cannot reach the base class lock --> not thread-safe !

  //this->linked_us_ = &us;
  //// Make sure the other side is also linked to us!
  //us.linked_us_ = this;

  ACE_Module<TaskSynchType, TimePolicyType> *my_tail = inherited::head ();

  if (my_tail == 0)
    return -1;

  // Locate the module just above our Stream tail.
  while (my_tail->next () != this->tail ())
    my_tail = my_tail->next ();

  ACE_Module<TaskSynchType, TimePolicyType> *other_tail = us.head ();
  ACE_Module<TaskSynchType, TimePolicyType> *other_head = us.head ();
  other_head = other_head->next ();

  if ((other_tail == 0) || (other_head == 0))
    return -1;

  // Locate the module just above the other Stream's tail.
  while (other_tail->next () != us.tail ())
    other_tail = other_tail->next ();

  int result = inherited::link (us);

  //// Reattach the pointers so that the two streams are linked!
  //my_tail->writer ()->next (other_tail->reader ());
  //other_tail->writer ()->next (my_tail->reader ());
  // *EDIT*: reset 'broken' writer link
  other_tail->writer ()->next (us.tail ()->writer ());

  my_tail->writer ()->next (other_head->writer ());
  // *NOTE*: do not link the outbound-side streams together (see header)
  //other_tail->reader ()->next (my_tail->reader ());

  return result;
}

//template <typename TaskSynchType,
//          typename TimePolicyType,
//          typename StateType,
//          typename ConfigurationType,
//          typename StatisticContainerType,
//          typename ModuleConfigurationType,
//          typename HandlerConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename SessionMessageType,
//          typename ProtocolMessageType>
//int
//Stream_Base_T<TaskSynchType,
//              TimePolicyType,
//              StateType,
//              ConfigurationType,
//              StatisticContainerType,
//              ModuleConfigurationType,
//              HandlerConfigurationType,
//              SessionDataType,
//              SessionDataContainerType,
//              SessionMessageType,
//              ProtocolMessageType>::remove (const ACE_TCHAR* name_in,
//                                            int flags_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::remove"));
//
//  MODULE_T* previous_p = NULL;
//  MODULE_T* head_p = inherited::head ();
//  for (MODULE_T* module_p = head_p;
//       module_p;
//       module_p = module_p->next ())
//  {
//    if (!ACE_OS::strcmp (module_p->name (), name_in))
//    {
//      // *NOTE*: (final) modules may be push()ed to several streams
//      //         concurrently. Push()ing a module always sets its "next_"-
//      //         member to the "tail" module. I.e., all traffic is forthwith
//      //         routed to the same "tail" module. This is not really a problem,
//      //         as long as the "tail" is replaced with a valid "tail" (--> the
//      //         available one) on removal (see below)
//      // *TODO*: this is quite a fragile workaround; find a better way to handle
//      //         this (e.g. override push())
//      IMODULE_T* imodule_p = dynamic_cast<IMODULE_T*> (module_p);
//      if (!imodule_p)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to dynamic_cast<Stream_IModule_T*> (%@), aborting\n"),
//                    module_p));
//        return -1;
//      } // end IF
//
//      MODULE_T* tail_p = inherited::tail ();
//      if (!previous_p) // head ?
//        head_p->link (module_p->next ());
//      else
//        previous_p->link (imodule_p->isFinal () ? tail_p
//                                                : module_p->next ());
//
//      // *NOTE*: do NOT close the module; it may be in use elsewhere
//      //module_p->close (flags_in);
//      // *TODO*: this does not work...
//      module_p->next (tail_p);
//
//      if (flags_in != MODULE_T::M_DELETE_NONE)
//        delete module_p;
//
//      return 0;
//    } // end IF
//    else
//      previous_p = module_p;
//  } // end FOR
//
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("%s: not found, aborting\n"),
//              name_in));
//
//  return -1;
//}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::remove (MODULE_T* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::remove"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (module_in);

  // *NOTE*: start with the last module and work backwards
  MODULE_T* module_p = module_in;
  MODULE_T* tail_p = NULL;
  while (module_p->next () != NULL)
    module_p = module_p->next ();
  tail_p = module_p;
  ACE_ASSERT (tail_p &&
              (ACE_OS::strcmp (module_p->name (), ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Tail")) == 0));

  std::deque<MODULE_T*> modules;
  module_p = module_in;
  while (module_p != tail_p)
  {
    modules.push_front (module_p);
    module_p = module_p->next ();
  } // end WHILE
  while (!modules.empty ())
  {
    module_p = modules.front ();

    // *NOTE*: removing a module close()s it; don't want that
    //         --> reset() it manually
    IMODULE_T* imodule_p = dynamic_cast<IMODULE_T*> (module_p);
    if (!imodule_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IModule_T*> (%@), aborting\n"),
                  module_p->name (),
                  module_p));
      return false;
    } // end IF

    result = inherited::remove (module_p->name (),
                                ACE_Module_Base::M_DELETE_NONE);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
                  module_p->name ()));
    } // end IF
    imodule_p->reset ();

    modules.pop_front ();
  } // end WHILE

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::isInitialized () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isInitialized"));

  return isInitialized_;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::shutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::shutdown"));

  int result = -1;

  // step0: if not properly initialized, this needs to deactivate any hitherto
  // enqueued ACTIVE modules, or the stream will wait forever during closure...
  // --> possible scenarios:
  // - (re-)init() failed halfway through (i.e. MAYBE some modules push()ed
  //   correctly)
  MODULE_T* module_p = NULL;
  if (!isInitialized_)
  {
    // sanity check: successfully pushed() ANY modules ?
    module_p = inherited::head ();
    if (module_p)
    {
      module_p = module_p->next ();
      if (module_p && (module_p != inherited::tail ()))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("not initialized - deactivating module(s)...\n")));

        deactivateModules ();

        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("not initialized - deactivating module(s)...DONE\n")));
      } // end IF
    } // end IF
  } // end IF

  // step1: iterator over modules which are NOT on the stream
  //        --> close() these manually (before they do so in their dtors...)
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...\n")));

  //for (ACE_DLList_Iterator<MODULE_T> iterator (availableModules_);
  //     (iterator.next (module_p) != 0);
  //     iterator.advance ())
  for (MODULE_CONTAINER_ITERATOR_T iterator = availableModules_.begin ();
       iterator != availableModules_.end ();
       iterator++)
  {
    // sanity check: on the stream ?
    //if (module_p->next () == NULL)
    if ((*iterator)->next () == NULL)
    {
      //ACE_DEBUG ((LM_WARNING,
      //            ACE_TEXT ("manually closing module: \"%s\"\n"),
      //            ACE_TEXT (module->name ())));

      try
      {
        //module_p->close (ACE_Module_Base::M_DELETE_NONE);
        result = (*iterator)->close (ACE_Module_Base::M_DELETE_NONE);
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                    //ACE_TEXT (module_p->name ())));
                    (*iterator)->name ()));
        result = -1;
      }
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%m\", continuing\n"),
                    //ACE_TEXT (module_p->name ())));
                    (*iterator)->name ()));
    } // end IF
  } // end FOR

  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...DONE\n")));

  // step2: shutdown stream
  // check the ACE documentation on ACE_Stream to see why this is needed !!!
  // *TODO*: ONLY do this if stream_head != 0 !!! (warning: obsolete ?)
  // *NOTE*: will NOT destroy all modules in the current stream as this leads to
  //         exceptions in debug builds under MS Windows (can't delete objects
  //         in a different DLL where it was created...)
  //         --> do this manually !
  //         all this does is call close() on each one (waits for any worker
  //         thread(s) to return)
  if (!finalize ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::finalize(): \"%m\", continuing\n")));

  // *NOTE*: every ACTIVE module will join with its worker thread in close()
  //         --> ALL stream-related threads should have returned by now !
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::deactivateModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::deactivateModules"));

  // *TODO*: remove type inferences

  // allocate session data container
  SessionDataContainerType* session_data_container_p = NULL;
  ACE_NEW_NORETURN (session_data_container_p,
                    SessionDataContainerType (sessionData_,
                                              false));
  if (!session_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", returning\n")));
    return;
  } // end IF

  // allocate SESSION_END session message
  SessionMessageType* message_p = NULL;
  if (allocator_)
  {
    try
    { // *NOTE*: 0 --> session message
      message_p =
       static_cast<SessionMessageType*> (allocator_->malloc (0));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_CRITICAL,
                 ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), returning\n")));

      // clean up
      session_data_container_p->decrease ();

      return;
    }
  } // end IF
  else
  {
    // *NOTE*: session message assumes responsibility for
    //         session_data_container_p
    // *TODO*: remove type inference
    ACE_NEW_NORETURN (message_p,
                      SessionMessageType (STREAM_SESSION_END,
                                          session_data_container_p,
                                          state_.userData));
  } // end ELSE
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", returning\n")));

    // clean up
    session_data_container_p->decrease ();

    return;
  } // end IF
  if (allocator_)
  {
    // *NOTE*: session message assumes responsibility for
    //         session_data_container_p
    // *TODO*: remove type inference
    message_p->initialize (STREAM_SESSION_END,
                           session_data_container_p,
                           state_.userData);
  } // end IF

  // send message downstream...
  int result = inherited::put (message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF
}
