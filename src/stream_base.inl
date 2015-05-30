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

#include "stream_macros.h"

#include "stream_session_data_base.h"
#include "stream_session_message_base.h"
#include "stream_iallocator.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
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
 , state_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::Stream_Base_T"));

//  ACE_OS::memset (&state_, 0, sizeof (state_));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::~Stream_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::~Stream_Base_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::reset"));

  // sanity check
  if (isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot reset (currently running), aborting\n")));
    return false;
  } // end IF

  // pop/close all modules
  // *NOTE*: will implicitly (blocking !) wait for any active worker threads
  if (!finalize ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to finalize(), aborting\n")));
    return false;
  } // end IF

  // - reset reader/writers tasks for ALL modules
  // - re-initialize head/tail modules
  return initialize ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  if (isInitialized_)
  {
    // *NOTE*: fini() invokes close() which will reset the writer/reader tasks
    // of the enqueued modules --> reset this !
    MODULE_T* module_p = NULL;
    IMODULE_T* imodule_handle_p = NULL;
    // *NOTE*: cannot write this - it confuses gcc...
    //   for (MODULE_CONTAINER_TYPE::const_iterator iter = availableModules_.begin ();
    for (ACE_DLList_Iterator<MODULE_T> iterator (availableModules_);
         iterator.next (module_p);
         iterator.advance ())
    {
      // need a downcast...
      imodule_handle_p = dynamic_cast<IMODULE_T*> (module_p);
      if (!imodule_handle_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                    ACE_TEXT (module_p->name ())));
        return false;
      } // end IF
      try
      {
        imodule_handle_p->reset ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_IModule::reset(), continuing\n")));
      }
    } // end FOR
  } // end IF

  // delegate this to base class open()
  int result = -1;
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
                ACE_TEXT ("failed to ACE_Stream::open(): \"%m\", aborting\n")));

  return (result == 0);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
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
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
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
                ACE_TEXT (module_p->name ())));
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
                ACE_TEXT (module_p->name ())));
    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::stop (bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  int result = -1;

  if (!isRunning ())
    return;

  // delegate to the head module, skip over ACE_Stream_Head...
  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return;
  } // end IF

  // *WARNING*: cannot flush(), as this deactivates() the queue as well,
  // which causes mayhem for (blocked) worker(s)...
  // *TODO*: consider optimizing this...
  //module->writer ()->flush ();

  ISTREAM_CONTROL_T* control_impl_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, returning\n"),
                ACE_TEXT (module_p->name ())));
    return;
  } // end IF

  try
  {
    control_impl_p->stop ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::stop(), returning\n"),
                ACE_TEXT (module_p->name ())));
    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isRunning"));

  int result = -1;

  // delegate to the head module
  MODULE_T* module_p = NULL;
  result = const_cast<SELF_T*> (this)->top (module_p);
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
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, aborting\n"),
                ACE_TEXT (module_p->name ())));
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
                ACE_TEXT (module_p->name ())));
  }

  return false;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::pause"));

  int result = -1;

  // sanity check
  if (!isRunning ())
  {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("not running --> nothing to do, returning\n")));
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
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module_p->name ())));
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
                ACE_TEXT (module_p->name ())));
    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
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
                ACE_TEXT (module_p->name ())));
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
                ACE_TEXT (module_p->name ())));
    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::waitForCompletion ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::waitForCompletion"));

  int result = -1;

  // *NOTE*: the logic here is this...
  // step1: wait for processing to finish
  // step2: wait for any pipelined messages to flush...

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
  // --> reason: no modules have been push()ed (yet) !
  // --> stream hasn't been intialized (at all: too many connections ?)
  // --> nothing to do !
  if (module_p == inherited::tail ())
    return;

  MODULE_CONTAINER_T modules;
  modules.push_front (const_cast<MODULE_T*> (module_p));
  // need to downcast
  HEADMODULE_TASK_T* head_impl_p = NULL;
  head_impl_p =
    dynamic_cast<HEADMODULE_TASK_T*> (const_cast<MODULE_T*> (module_p)->writer ());
  if (!head_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_HeadModuleTask_t> failed, returning\n"),
                ACE_TEXT (module_p->name ())));
    return;
  } // end IF

  try
  {
    // wait for state switch (xxx --> FINISHED) / any worker(s)
    head_impl_p->waitForCompletion ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::waitForCompletion (), returning\n"),
                ACE_TEXT (module_p->name ())));
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
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));

    module_p = NULL;
  } // end FOR

  // step2: wait for any pipelined messages to flush...
  for (MODULE_CONTAINER_ITERATOR_T iterator2 = modules.begin ();
       iterator2 != modules.end ();
       iterator2++)
  {
    task_p = (*iterator2)->writer ();
    ACE_ASSERT (task_p);
    result = task_p->wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
  } // end FOR
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
const StreamStateType*
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::getState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::getState"));

  return &state_;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::dump_state"));

  std::string stream_layout;

  const MODULE_T* module_p = NULL;
  for (ITERATOR_T iterator (*this);
       (iterator.next (module_p) != 0);
       iterator.advance ())
  {
    // silently ignore ACE head/tail modules...
    if ((module_p == const_cast<SELF_T*> (this)->tail ()) ||
        (module_p == const_cast<SELF_T*> (this)->head ()))
      continue;

    stream_layout.append (ACE_TEXT_ALWAYS_CHAR (module_p->name ()));

    // avoid trailing "-->"...
    if (const_cast<MODULE_T*> (module_p)->next () !=
        const_cast<SELF_T*> (this)->tail ())
      stream_layout += ACE_TEXT_ALWAYS_CHAR (" --> ");

    module_p = NULL;
  } // end FOR

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stream layout: \"%s\"\n"),
              ACE_TEXT (stream_layout.c_str ())));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
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
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::shutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::shutdown"));

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

  // step1: retrieve a list of modules which are NOT on the stream
  // --> need to close() these manually (before they do so in their dtors...)
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...\n")));

  for (ACE_DLList_Iterator<MODULE_T> iterator (availableModules_);
       (iterator.next (module_p) != 0);
       iterator.advance ())
  {
    // sanity check: on the stream ?
    if (module_p->next () == NULL)
    {
      //ACE_DEBUG ((LM_WARNING,
      //            ACE_TEXT ("manually closing module: \"%s\"\n"),
      //            ACE_TEXT (module->name ())));

      try
      {
        module_p->close (ACE_Module_Base::M_DELETE_NONE);
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                    ACE_TEXT (module_p->name ())));
      }
    } // end IF
  } // end FOR

  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...DONE\n")));

  // step2: shutdown stream
  // check the ACE documentation on ACE_Stream to see why this is needed !!!
  // Note: ONLY do this if stream_head != 0 !!! (warning: obsolete ?)
  // *NOTE*: this will NOT destroy all modules in the current stream
  // as this leads to exceptions in debug builds under windows (can't delete
  // objects in a different DLL where it was created...)
  // --> we need to do this ourselves !
  // all this does is call close() on each one (--> wait for any worker
  // thread(s) to return)
  if (!finalize ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::finalize(): \"%m\", continuing\n")));

  // *NOTE*: every ACTIVE module will join with its worker thread in close()
  // --> ALL stream-related threads should have returned by now !
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename StreamStatisticContainerType,
          typename StreamConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              StreamStateType,
              StreamStatisticContainerType,
              StreamConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::deactivateModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::deactivateModules"));

  // allocate session data
  SessionDataContainerType* session_data_p = NULL;
  ACE_NEW_NORETURN (session_data_p,
                    SessionDataContainerType (NULL,
                                              false));
  if (!session_data_p)
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
    {
      message_p =
       static_cast<SessionMessageType*> (allocator_->malloc (0)); // want a session message !
    }
    catch (...)
    {
      ACE_DEBUG ((LM_CRITICAL,
                 ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), returning\n")));

      // clean up
      session_data_p->decrease ();

      return;
    }
  } // end IF
  else
  { // *NOTE*: session message assumes responsibility for session_data_p
    ACE_NEW_NORETURN (message_p,
                      SessionMessageType (SESSION_END,
                                          &state_,
                                          session_data_p));
  } // end ELSE
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", returning\n")));

    // clean up
    session_data_p->decrease ();

    return;
  } // end IF
  if (allocator_)
  { // *NOTE*: session message assumes responsibility for session_data_container_p !
    message_p->initialize (SESSION_END,
                           &state_,
                           session_data_p);
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
