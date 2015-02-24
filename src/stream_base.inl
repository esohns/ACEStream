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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::Stream_Base_T ()
// *TODO*: use default ctor and rely on init/fini() ?
 : inherited (NULL, // argument to module open()
              NULL, // no head module --> allocate !
              NULL) // no tail module --> allocate !
// , availableModules_ ()
 , isInitialized_ (false)
 , allocator_ (NULL)
// , state_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::Stream_Base_T"));

  ACE_OS::memset (&state_, 0, sizeof (state_));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::~Stream_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::~Stream_Base_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::reset"));

  // sanity check: is running ?
  if (isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot reset (currently running), aborting\n")));

    return false;
  } // end IF

  // pop/close all modules
  // *NOTE*: will implicitly (blocking !) wait for any active worker threads
  if (!fini ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to fini(), aborting\n")));

    return false;
  } // end IF

  // - reset reader/writers tasks for ALL modules
  // - reinit head/tail modules
  return init ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::init ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::init"));

  if (isInitialized_)
  {
    // *NOTE*: fini() invokes close() which will reset the writer/reader tasks
    // of the enqueued modules --> reset this !
    Stream_IModule_t* imodule_handle = NULL;
    Stream_Module_t* module = NULL;
    // *NOTE*: cannot write this - it confuses gcc...
    //   for (MODULE_CONTAINER_TYPE::const_iterator iter = myAvailableModules.begin();
    for (ACE_DLList_Iterator<Stream_Module_t> iterator (availableModules_);
         iterator.next (module);
         iterator.advance ())
    {
      // need a downcast...
      imodule_handle = dynamic_cast<Stream_IModule_t*> (module);
      if (!imodule_handle)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                    ACE_TEXT (module->name ())));

        return false;
      } // end IF
      try
      {
        imodule_handle->reset ();
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
                ACE_TEXT ("caught exception in ACE_Stream::open(), aborting\n")));
    result = -1;
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::open(): \"%m\", aborting\n")));

  return (result == 0);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::fini ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::fini"));

  // OK: delegate this to base class close(ACE_Module_Base::M_DELETE_NONE)
  int result = -1;
  try
  {
    // *NOTE*: this will implicitly:
    // - unwind the stream, which pop()s all (pushed) modules
    // --> pop()ing a module will close() it
    // --> close()ing a module will module_closed() and flush() the associated tasks
    // --> flush()ing a task will close() its queue
    // --> close()ing a queue will deactivate() and flush() it
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ACE_Stream::close(M_DELETE_NONE), aborting\n")));
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::close(M_DELETE_NONE): \"%m\", aborting\n")));

  return (result == 0);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::start"));

  // sanity check: is initialized ?
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));

    return;
  } // end IF

  // delegate to the head module
  Stream_Module_t* module = inherited::head ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF

  // skip over ACE_Stream_Head...
  module = module->next ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF

  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
  if (module == inherited::tail ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));

    return;
  } // end IF

  Stream_IStreamControl* control_impl = NULL;
  control_impl = dynamic_cast<Stream_IStreamControl*> (module->writer ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module->name ())));

    return;
  } // end IF

  try
  {
    control_impl->start ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::start (module: \"%s\"), returning\n"),
                ACE_TEXT (module->name ())));

    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::stop (bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check: is running ?
  if (!isRunning ())
  {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("not running --> nothing to do, returning\n")));

    return;
  } // end IF

  // delegate to the head module, skip over ACE_Stream_Head...
  Stream_Module_t* module = inherited::head ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF
  module = module->next ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF

  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
  if (module == inherited::tail ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));

    return;
  } // end IF

  // *WARNING*: cannot flush(), as this deactivates() the queue as well,
  // which causes mayhem for our (blocked) worker...
  // *TODO*: consider optimizing this...
  //module->reader ()->flush ();

  Stream_IStreamControl* control_impl = NULL;
  control_impl = dynamic_cast<Stream_IStreamControl*> (module->writer ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module->name ())));

    return;
  } // end IF

  try
  {
    control_impl->stop ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::stop (module: \"%s\"), returning\n"),
                ACE_TEXT (module->name ())));

    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::pause"));

  // sanity check: is running ?
  if (!isRunning ())
  {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("not running --> nothing to do, returning\n")));

    return;
  } // end IF

  // delegate to the head module
  Stream_Module_t* module = inherited::head ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF

  // skip over ACE_Stream_Head...
  module = module->next ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF

  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
  if (module == inherited::tail ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));

    return;
  } // end IF

  Stream_IStreamControl* control_impl = NULL;
  control_impl = dynamic_cast<Stream_IStreamControl*> (module->writer ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module->name ())));

    return;
  } // end IF

  try
  {
    control_impl->pause ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::pause (module: \"%s\"), returning\n"),
                ACE_TEXT (module->name ())));

    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::rewind"));

  // sanity check: is running ?
  if (isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("currently running, returning\n")));

    return;
  } // end IF

  // delegate to the head module
  Stream_Module_t* module = inherited::head ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    // *NOTE*: what else can we do ?
    return;
  } // end IF

  // skip over ACE_Stream_Head...
  module = module->next ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    // *NOTE*: what else can we do ?
    return;
  } // end IF

  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
  if (module == inherited::tail ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));

    return;
  } // end IF

  Stream_IStreamControl* control_impl = NULL;
  control_impl = dynamic_cast<Stream_IStreamControl*> (module->writer ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<RPG_Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module->name ())));

    return;
  } // end IF

  try
  {
    control_impl->rewind ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in RPG_Stream_IStreamControl::rewind (module: \"%s\"), returning\n"),
                ACE_TEXT (module->name ())));

    return;
  }
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::waitForCompletion ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::waitForCompletion"));

  // OK: the logic here is this...
  // step1: wait for processing to finish
  // step2: wait for any pipelined messages to flush...

  // step1: get head module, skip over ACE_Stream_Head
  Stream_StreamIterator_t iterator (*this);
  if (iterator.advance () == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF
  const Stream_Module_t* module = NULL;
  if (iterator.next (module) == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));

    return;
  } // end IF

  // sanity check: head == tail ?
  // --> reason: no modules have been push()ed (yet) !
  // --> stream hasn't been intialized (at all: too many connections ?)
  // --> nothing to do !
  if (module == inherited::tail ())
    return;

  Stream_Modules_t modules;
  modules.push_front (const_cast<Stream_Module_t*> (module));
  // need to downcast
  Stream_HeadModuleTask_t* head_impl = NULL;
  head_impl = dynamic_cast<Stream_HeadModuleTask_t*> (const_cast<Stream_Module_t*> (module)->writer ());
  if (!head_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_HeadModuleTask_t> failed, returning\n"),
                ACE_TEXT (module->name ())));

    return;
  } // end IF

  try
  {
    // wait for state switch (xxx --> FINISHED) / any worker(s)
    head_impl->waitForCompletion ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::waitForCompletion (module: \"%s\"), returning\n"),
                ACE_TEXT (module->name ())));

    return;
  }

  for (iterator.advance ();
       (iterator.next (module) != 0);
       iterator.advance ())
  {
    // skip stream tail (last module)
    if (module == inherited::tail ())
      continue;

    modules.push_front (const_cast<Stream_Module_t*> (module));
    // OK: got a handle... wait
    if (const_cast<Stream_Module_t*> (module)->writer ()->wait () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));

    module = NULL;
  } // end FOR

  // step2: wait for any pipelined messages to flush...
  for (Stream_ModulesIterator_t iterator2 = modules.begin ();
       iterator2 != modules.end ();
       iterator2++)
    if ((*iterator2)->reader ()->wait () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isRunning"));

  // delegate to the head module, skip over ACE_Stream_Head...
  Stream_Module_t* module = const_cast<own_type*> (this)->head ();
  if (!module)
  {
    // *IMPORTANT NOTE*: this happens when no modules have been pushed onto the
    // stream yet
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found, aborting\n")));

    return false;
  } // end IF
  module = module->next ();
  if (!module)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, aborting\n")));

    return false;
  } // end IF

  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
  if (module == const_cast<own_type*> (this)->tail ())
  {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));

    return false;
  } // end IF

  Stream_IStreamControl* control_impl = NULL;
  control_impl = dynamic_cast<Stream_IStreamControl*> (module->writer ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module->name ())));

    return false;
  } // end IF

  try
  {
    return control_impl->isRunning ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::isRunning (module: \"%s\"), aborting\n"),
                ACE_TEXT (module->name ())));
  }

  return false;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::dump_state"));

  std::string stream_layout;

  const Stream_Module_t* module = NULL;
  for (Stream_StreamIterator_t iterator (*this);
       (iterator.next (module) != 0);
       iterator.advance ())
  {
    // silently ignore ACE head/tail modules...
    if ((module == const_cast<own_type*> (this)->tail ()) ||
        (module == const_cast<own_type*> (this)->head ()))
      continue;

    stream_layout.append (ACE_TEXT_ALWAYS_CHAR (module->name ()));

    // avoid trailing "-->"...
    if (const_cast<Stream_Module_t*> (module)->next () !=
        const_cast<own_type*> (this)->tail ())
      stream_layout += ACE_TEXT_ALWAYS_CHAR (" --> ");

    module = NULL;
  } // end FOR

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stream layout: \"%s\"\n"),
              ACE_TEXT (stream_layout.c_str ())));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<TaskSynchType,
              TimePolicyType,
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
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
  Stream_Module_t* module = NULL;
  if (!isInitialized_)
  {
    // sanity check: successfully pushed() ANY modules ?
    module = inherited::head ();
    if (module)
    {
      module = module->next ();
      if (module && (module != inherited::tail ()))
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

  for (ACE_DLList_Iterator<Stream_Module_t> iterator (availableModules_);
       (iterator.next (module) != 0);
       iterator.advance ())
  {
    // sanity check: on the stream ?
    if (module->next () == NULL)
    {
      //ACE_DEBUG ((LM_WARNING,
      //            ACE_TEXT ("manually closing module: \"%s\"\n"),
      //            ACE_TEXT (module->name ())));

      try
      {
        module->close (ACE_Module_Base::M_DELETE_NONE);
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n")));
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
  if (!fini ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::fini(): \"%m\", continuing\n")));

  // *NOTE*: every ACTIVE module will join with its worker thread in close()
  // --> ALL stream-related threads should have returned by now !
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<TaskSynchType,
              TimePolicyType,
              SessionDataType,
              SessionDataContainerType,
              SessionMessageType,
              ProtocolMessageType>::deactivateModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::deactivateModules"));

  // allocate session data
  SessionDataContainerType* session_data_container_p = NULL;
  ACE_NEW_NORETURN (session_data_container_p,
                    SessionDataContainerType (NULL,
                                              false,
                                              &state_,
                                              ACE_Time_Value::zero,
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
    {
      message_p = static_cast<SessionMessageType*> (allocator_->malloc (0)); // want a session message !
    }
    catch (...)
    {
      ACE_DEBUG ((LM_CRITICAL,
                 ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));

      // clean up
      session_data_container_p->decrease ();

      return;
    }
  } // end IF
  else
  { // *NOTE*: session message assumes responsibility for session_data_container_p !
    ACE_NEW_NORETURN (message_p,
                      SessionMessageType (SESSION_END,
                                          session_data_container_p));
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
  { // *NOTE*: session message assumes responsibility for session_data_container_p !
    message_p->init (SESSION_END,
                     session_data_container_p);
  } // end IF

  // send message downstream...
  if (inherited::put (message_p, NULL) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF
}
