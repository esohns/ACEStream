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

#include <ace/Log_Msg.h>

#include "stream_data_base.h"
#include "stream_iallocator.h"
#include "stream_imessagequeue.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::Stream_Base_T (const std::string& name_in,
                                                  bool finishOnDisconnect_in)
 : inherited (NULL, // default argument to module open()
              NULL, // --> allocate head module
              NULL) // --> allocate tail module
 , configuration_ (NULL)
 , delete_ (false)
 , finishOnDisconnect_ (finishOnDisconnect_in)
 , isInitialized_ (false)
 , messageQueue_ (STREAM_QUEUE_MAX_SLOTS)
 , modules_ ()
 , name_ (name_in)
 , sessionData_ (NULL)
 , sessionDataLock_ ()
 , state_ ()
 , upStream_ (NULL)
 /////////////////////////////////////////
 , hasFinal_ (false)
 , lock_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::Stream_Base_T"));

  // *NOTE*: pass a handle to the message queue member to the head module reader
  //         task; this will become the outbound queue

  int result = -1;
  HEAD_T* writer_p = NULL;
  ACE_NEW_NORETURN (writer_p,
                    HEAD_T (NULL));
  HEAD_T* reader_p = NULL;
  ACE_NEW_NORETURN (reader_p,
                    HEAD_T (&messageQueue_));
  if (!writer_p || !reader_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  typename inherited2::MODULE_T* module_p = NULL;
  ACE_NEW_NORETURN (module_p,
                    typename inherited2::MODULE_T (ACE_TEXT (STREAM_MODULE_HEAD_NAME),
                                                   writer_p, reader_p,
                                                   NULL,
                                                   ACE_Module_Base::M_DELETE_NONE));
  if (!module_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    delete writer_p;
    delete reader_p;

    return;
  } // end IF

  result = inherited::close ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::close(): \"%m\", returning\n")));

    // clean up
    delete module_p;

    return;
  } // end IF
  result = inherited::open (NULL,     // argument passed to module open()
                            module_p, // head module handle
                            NULL);    // tail module handle --> allocate
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::open(): \"%m\", returning\n")));

    // clean up
    delete module_p;

    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::~Stream_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::~Stream_Base_T"));

  if (state_.module)
  {
    typename inherited2::MODULE_T* module_p =
      inherited::find (state_.module->name ());
    if (module_p)
    {
      if (!remove (module_p,
                   true))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                    state_.module->name ()));
    } // end IF

    if (state_.deleteModule)
      delete state_.module;
  } // end IF

  if (sessionData_)
    sessionData_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::reset"));

  bool result = false;
//  int result_2 = -1;

  // pop/close all modules
  // *NOTE*: will implicitly (blocking !) wait for any active worker threads

  // handle final module (if any)
  // *NOTE*: this module may be shared by multiple stream instances, so it
  //         must not be close()d here
  if (hasFinal_)
  {
    if (state_.module)
    {
      typename inherited2::MODULE_T* module_p =
        inherited::find (state_.module->name ());
      if (module_p)
      {
        if (!remove (module_p,
                     true))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                      state_.module->name ()));
      } // end IF
    } // end IF

//    if (state_.deleteModule)
//    {
//      ACE_ASSERT (state_.module);
//      delete state_.module;

//      state_.deleteModule = false;
//    } // end IF
//    state_.module = NULL;

    { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
      modules_.pop_front ();
    } // end lock scope
    hasFinal_ = false;
  } // end IF

  result = finalize ();
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::finalize(), continuing\n"),
                ACE_TEXT (name_.c_str ())));

  // - reset reader/writers tasks for ALL modules
  // - re-initialize head/tail modules
  initialize (true,
              true);

  return result;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::setup (ACE_Notification_Strategy* notificationStrategy_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::setup"));

  int result = -1;

  // step1: reset pipeline
  try {
    result = inherited::open (NULL,                // argument to push()
                              inherited::head (),  // head
                              inherited::tail ()); // tail
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Stream::open(), aborting\n"),
                ACE_TEXT (name_.c_str ())));
    result = -1;
  }
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::open(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return false;
  } // end IF
  // *TODO*: this really shouldn't be necessary
  //inherited::head ()->next (inherited::tail ());

  // step2: set notification strategy ?
  typename inherited2::MODULE_T* module_p = NULL;
  if (notificationStrategy_in)
  {
    module_p = inherited::head ();
    if (!module_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module found, aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    } // end IF
    TASK_T* task_p = module_p->reader ();
    if (!task_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module reader task found, aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    } // end IF
    QUEUE_T* queue_p = task_p->msg_queue ();
    if (!queue_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module reader task queue found, aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    } // end IF
    queue_p->notification_strategy (notificationStrategy_in);
  } // end IF

  // step3: push all available modules
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);

    for (typename inherited2::MODULE_LIST_ITERATOR_T iterator = modules_.begin ();
         iterator != modules_.end ();
         iterator++)
    {
      result = inherited::push (*iterator);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator)->name ()));
        return false;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: pushed \"%s\"...\n"),
                  ACE_TEXT (name_.c_str ()),
                  (*iterator)->name ()));
    } // end FOR
  } // end lock scope
//#if defined (_DEBUG)
//  dump_state ();
//#endif

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (bool setupPipeline_in,
                                               bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  // step1: allocate session data ?
  if (resetSessionData_in)
  {
    // sanity check(s)
    ACE_ASSERT (!sessionData_);

    SessionDataType* session_data_p = NULL;
    ACE_NEW_NORETURN (session_data_p,
                      SessionDataType ());
    if (!session_data_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                  ACE_TEXT (name_.c_str ())));
      return;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: allocated %u byte(s) of session data: %@ (lock: %@)...\n"),
                ACE_TEXT (name_.c_str ()),
                sizeof (SessionDataType),
                session_data_p,
                &sessionDataLock_));

    // *TODO*: remove type inferences
    session_data_p->lock = &sessionDataLock_;
    state_.sessionData = session_data_p;

    // *IMPORTANT NOTE*: fire-and-forget API (session_data_p)
    ACE_NEW_NORETURN (sessionData_,
                      SessionDataContainerType (session_data_p));
    if (!sessionData_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                  ACE_TEXT (name_.c_str ())));

      // clean up
      state_.sessionData = NULL;
      delete session_data_p;

      return;
    } // end IF
  } // end IF

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    if (!load (modules_,
               delete_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_IStreamControlBase::load(), returning\n"),
                  ACE_TEXT (name_.c_str ())));
      return;
    } // end IF
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_->moduleConfiguration);

  // step2: initialize modules
  IMODULE_T* imodule_p = NULL;
  TASK_T* task_p = NULL;
  IMODULE_HANDLER_T* imodule_handler_p = NULL;
  CONFIGURATION_ITERATOR_T iterator_2;
  const HandlerConfigurationType* configuration_p = NULL;

  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);
    for (typename inherited2::MODULE_LIST_ITERATOR_T iterator = modules_.begin ();
         iterator != modules_.end ();
         iterator++)
    {
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator)->name ()));
        return;
      } // end IF
      if (!imodule_p->isFinal ())
        imodule_p->reset ();
      if (!imodule_p->initialize (*configuration_->moduleConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Common_IInitialize_T::initialize(), returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator)->name ()));
        return;
      } // end IF

      task_p = (*iterator)->writer ();
      ACE_ASSERT (task_p);
      imodule_handler_p = dynamic_cast<IMODULE_HANDLER_T*> (task_p);
      if (!imodule_handler_p)
      { // *TODO*: determine the 'active' side of the module by some
        //         member/function
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("%s: dynamic_cast<Stream_IModuleHandler_T> failed, continuing\n"),
        //            (*iterator)->name ()));
        task_p = (*iterator)->reader ();
        ACE_ASSERT (task_p);
        imodule_handler_p = dynamic_cast<IMODULE_HANDLER_T*> (task_p);
        if (!imodule_handler_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModuleHandler_T> failed, continuing\n"),
                      ACE_TEXT (name_.c_str ()),
                      (*iterator)->name ()));
          continue;
        } // end IF
      } // end IF
      // *TODO*: remove type inference
      iterator_2 =
          configuration_->moduleHandlerConfigurations.find ((*iterator)->name ());
      if (iterator_2 == configuration_->moduleHandlerConfigurations.end ())
        iterator_2 =
            configuration_->moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s: applying propietary configuration...\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator)->name ()));
      ACE_ASSERT (iterator_2 != configuration_->moduleHandlerConfigurations.end ());
      // *TODO*: use a dynamic cast here
      configuration_p =
          dynamic_cast<const HandlerConfigurationType*> ((*iterator_2).second);
      ACE_ASSERT (configuration_p);
      if (!imodule_handler_p->initialize (*configuration_p,
                                          configuration_->messageAllocator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Stream_IModuleHandler_T::initialize(), continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator)->name ()));

      //// *TODO* fix ACE bug: modules should initialize their 'next_' member
      //(*iterator)->next (NULL);
    } // end FOR
  } // end lock scope

  // step3: setup pipeline ?
  if (setupPipeline_in)
    if (!setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::setup(), returning\n"),
                  ACE_TEXT (name_.c_str ())));
      return;
    } // end IF

  //// step4: initialize state machine
  //// delegate to the head module
  //MODULE_T* module_p = NULL;
  //result = inherited::top (module_p);
  //if ((result == -1) || !module_p)
  //{
  //  //ACE_DEBUG ((LM_ERROR,
  //  //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
  //  return;
  //} // end IF

  //STATEMACHINE_ICONTROL_T* statemachine_icontrol_p =
  //  dynamic_cast<STATEMACHINE_ICONTROL_T*> (module_p->writer ());
  //if (!statemachine_icontrol_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: dynamic_cast<Stream_StateMachine_IControl_T*> failed, returning\n"),
  //              module_p->name ()));
  //  return;
  //} // end IF

  //try {
  //  statemachine_icontrol_p->initialize ();
  //} catch (...) {
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: caught exception in Stream_StateMachine_IControl_T::initialize(), returning\n"),
  //              module_p->name ()));
  //  return;
  //}

  isInitialized_ = true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::finalize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finalize"));

  int result = -1;

  // delegate this to base class close(ACE_Module_Base::M_DELETE_NONE)
  try {
    // *NOTE*: unwinds the stream, pop()ing all push()ed modules
    //         --> pop()ing a module will close() it
    //         --> close()ing a module will module_closed() and flush() its
    //             tasks
    //         --> flush()ing a task will close() its queue
    //         --> close()ing a queue will deactivate() and flush() it
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Stream::close(M_DELETE_NONE), continuing\n"),
                ACE_TEXT (name_.c_str ())));
    result = -1;
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::close(M_DELETE_NONE): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));

  if (delete_)
  {
    { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
      for (typename inherited2::MODULE_LIST_ITERATOR_T iterator = modules_.begin ();
           iterator != modules_.end ();
           ++iterator)
        delete *iterator;
    } // end lock scope
  } // end IF
  modules_.clear ();

//  if (state_.sessionData)
//  {
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("%s: stream has shut down, resetting session data lock\n"),
//                ACE_TEXT (name_.c_str ())));

//    state_.sessionData->lock = NULL;
//  } // end IF

  return (result == 0);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::start"));

  int result = -1;

  // sanity check(s)
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: not initialized, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  if (isRunning ())
    return; // nothing to do

  // delegate to the head module
  typename inherited2::MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found: \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  ISTREAM_CONTROL_T* control_impl_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    control_impl_p->start ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::start(), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::stop (bool wait_in,
                                         bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::stop"));

  int result = -1;
  typename inherited2::MODULE_T* module_p = NULL;
  ISTREAM_CONTROL_T* control_impl_p = NULL;

  // has upstream ? --> (try to) stop that instead
  if (upStream_)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);

    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: stopping upstream...\n"),
    //            ACE_TEXT (istream_p->name ().c_str ())));

    // delegate to the head module, skip over ACE_Stream_Head...
    result = upStream_->top (module_p);
    if ((result == -1) || !module_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: no head module found: \"%m\", returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      return;
    } // end IF

    // *WARNING*: cannot flush(), as this deactivates() the queue as well, which
    //            causes mayhem for any (blocked) worker(s)
    // *TODO*: consider optimizing this
    //module->writer ()->flush ();
    control_impl_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
    if (!control_impl_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name ()));
      return;
    } // end IF

    try {
      control_impl_p->stop (wait_in,
                            lockedAccess_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl::stop(), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name ()));
      return;
    }

    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: stopping upstream...done\n"),
    //            ACE_TEXT (name_.c_str ())));
  } // end IF

  if (!isRunning ())
    goto wait;

  // delegate to the head module, skip over ACE_Stream_Head...
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found: \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  // *WARNING*: cannot flush(), as this deactivates() the queue as well, which
  //            causes mayhem for any (blocked) worker(s)
  // *TODO*: consider optimizing this
  //module->writer ()->flush ();
  control_impl_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    control_impl_p->stop (wait_in,
                          lockedAccess_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::stop(), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  }

wait:
  if (wait_in)
    wait (true,   // wait for any worker thread(s) ?
          false,  // wait for upstream (if any) ?
          false); // wait for downstream (if any) ?
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isRunning"));

  int result = -1;

  // delegate to the head module
  typename inherited2::MODULE_T* module_p = NULL;
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
//                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl_T> failed, aborting\n"),
//                module_p->name ()));
    return false;
  } // end IF

  try {
    return control_impl_p->isRunning ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::isRunning(), aborting\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
  }

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::control (ControlType control_in,
                                            bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::control"));

  int result = -1;
  ISTREAM_CONTROL_T* istream_control_p = NULL;

  // *IMPORTANT NOTE*: if this stream has been linked (e.g. connection is part
  //                   of another stream), forward upstream ?
  if (upStream_ && forwardUpstream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);
    istream_control_p = dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upStream_));
      return;
    } // end IF
    istream_control_p->control (control_in,
                                forwardUpstream_in);
    return;
  } // end IF

  typename inherited2::MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (module_p);

  switch (control_in)
  {
    case STREAM_CONTROL_END:
    case STREAM_CONTROL_LINK:
    case STREAM_CONTROL_STEP:
    case STREAM_CONTROL_UNLINK:
    {
      istream_control_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
      if (!istream_control_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
        return;
      } // end IF

      try {
        istream_control_p->control (control_in,
                                    false);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::control(%d), returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name (),
                    control_in));
        return;
      }

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown control (was: %d), returning\n"),
                  ACE_TEXT (name_.c_str ()),
                  control_in));
      return;
    }
  } // end IF
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::notify (NotificationType notification_in,
                                           bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::notify"));

  int result = -1;
  ISTREAM_CONTROL_T* istream_control_p = NULL;

  // *IMPORTANT NOTE*: if this stream has been linked (e.g. connection is part
  //                   of another stream), forward upstream ?
  if (upStream_ && forwardUpstream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);
    istream_control_p = dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upStream_));
      return;
    } // end IF
    try {
      istream_control_p->notify (notification_in,
                                 forwardUpstream_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::notify(%d), continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  notification_in));
    }
    return;
  } // end IF

  // send control ?
  // *TODO*: this is strategy and needs to be traited ASAP
//  bool send_control = false;
  ControlType control_type = STREAM_CONTROL_INVALID;
  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      control_type = STREAM_CONTROL_FLUSH;
      break;
    }
    default:
      break;
  } // end SWITCH
  ACE_UNUSED_ARG (control_type);

  typename inherited2::MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (module_p);

  istream_control_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!istream_control_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    istream_control_p->notify (notification_in,
                               false);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::notify(%d), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name (),
                notification_in));
    return;
  }

  // finished ?
  // *TODO*: this is strategy and needs to be traited ASAP
  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      if (finishOnDisconnect_)
        finished ();
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::flush (bool flushInbound_in,
                                          bool flushSessionMessages_in,
                                          bool flushUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::flush"));

  int result = -1;

  // *IMPORTANT NOTE*: if this stream has been linked (e.g. connection is part
  //                   of another stream), flush the whole pipeline ?
  if (upStream_ && flushUpStream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);
    ISTREAM_CONTROL_T* istream_control_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upStream_));
      return;
    } // end IF
    istream_control_p->flush (flushInbound_in,
                              flushSessionMessages_in,
                              flushUpStream_in);
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("flushed upstream \"%s\"...\n"),
    //            ACE_TEXT (istream_control_p->name ().c_str ())));
  } // end IF

  typename inherited2::MODULE_LIST_T modules;
  const typename inherited2::MODULE_T* module_p = NULL;
  TASK_T* task_p = NULL;
  Stream_IMessageQueue* iqueue_p = NULL;

  for (ITERATOR_T iterator (*this);
       (iterator.next (module_p) != 0);
       iterator.advance ())
    modules.push_front (const_cast<typename inherited2::MODULE_T*> (module_p));
  modules.pop_back (); // discard head
  modules.pop_front (); // discard tail

  // *TODO*: implement a dedicated control message to push this functionality
  //         into the task object
  if (!flushInbound_in)
    goto continue_;

  // writer (inbound) side
  for (typename inherited2::MODULE_LIST_REVERSE_ITERATOR_T iterator = modules.rbegin ();
       iterator != modules.rend ();
       ++iterator)
  {
    task_p = const_cast<typename inherited2::MODULE_T*> (*iterator)->writer ();
    if (!task_p) continue; // close()d already ?

    ACE_ASSERT (task_p->msg_queue_);
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
    if (!iqueue_p)
    {
      // *NOTE*: most probable cause: module is (upstream) head
      // *TODO*: all messages are flushed here, this must not happen
      result = task_p->msg_queue_->flush ();
    } // end IF
    else
      result = static_cast<int> (iqueue_p->flush (flushSessionMessages_in));
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s writer: failed to Stream_IMessageQueue::flushData(): \"%m\", continuing\n"),
                  ACE_TEXT (name_.c_str ()),
                  (*iterator)->name ()));
    } // end IF
    else if (result)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s writer: flushed %d message(s)\n"),
                  ACE_TEXT (name_.c_str ()),
                  (*iterator)->name (),
                  result));
    } // end IF
  } // end FOR

continue_:
  // reader (outbound) side
  modules.push_back (const_cast<typename inherited2::MODULE_T*> (inherited::head ())); // append head
  for (typename inherited2::MODULE_LIST_ITERATOR_T iterator = modules.begin ();
       iterator != modules.end ();
       iterator++)
  {
    task_p = (*iterator)->reader ();
    if (!task_p) continue; // close()d already ?

    ACE_ASSERT (task_p->msg_queue_);
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (task_p->msg_queue_);
    if (!iqueue_p)
    {
      // *NOTE*: most probable cause: stream head, or module does not have a
      //         reader task
      result = task_p->msg_queue_->flush ();
    } // end IF
    else
      result = static_cast<int> (iqueue_p->flush (flushSessionMessages_in));
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s reader: failed to Stream_IMessageQueue::flushData(): \"%m\", continuing\n"),
                  ACE_TEXT (name_.c_str ()),
                  (*iterator)->name ()));
    } // end IF
    else if (result)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s reader: flushed %d message(s)...\n"),
                  ACE_TEXT (name_.c_str ()),
                  (*iterator)->name (),
                  result));
    } // end ELSE IF
  } // end FOR
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::pause"));

  int result = -1;

  //// sanity check
  //ACE_ASSERT (isRunning ());

  // delegate to the head module
  typename inherited2::MODULE_T* module_p = NULL;
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
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    control_impl_p->pause ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl::pause(), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::rewind"));

  int result = -1;

  // sanity check
  // *TODO*
  if (isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: currently running, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  // delegate to the head module
  typename inherited2::MODULE_T* module_p = NULL;
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
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    control_impl_p->rewind ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::rewind(), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
StatusType
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::status () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::status"));

  StatusType result = static_cast<StatusType> (-1);
  int result_2 = -1;

  // sanity check(s)
  // *NOTE*: top() uses inherited::stream_head_. If the stream has not been
  //         open()ed yet, or failed to initialize(), this will crash
  if (!const_cast<OWN_TYPE_T*> (this)->head ())
    return result;

  // delegate to the head module
  typename inherited2::MODULE_T* module_p = NULL;
  result_2 = const_cast<OWN_TYPE_T*> (this)->top (module_p);
  if ((result_2 == -1) || !module_p)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return result;
  } // end IF

  ISTREAM_CONTROL_T* control_impl_p = NULL;
  control_impl_p = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, aborting\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return result;
  } // end IF

  try {
    result = control_impl_p->status ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl_T::status(), aborting\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return result;
  }

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::wait (bool waitForThreads_in,
                                         bool waitForUpStream_in,
                                         bool waitForDownStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::wait"));

  int result = -1;

  // if this stream has been linked (e.g. 'this' is part of another stream),
  // wait for the whole pipeline ?
  if (upStream_ && waitForUpStream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);
    ISTREAM_CONTROL_T* istream_control_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  upStream_));
      return;
    } // end IF
    try {
      istream_control_p->wait (waitForThreads_in,
                               waitForUpStream_in,
                               waitForDownStream_in);
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("upstream \"%s\" complete...\n"),
      //            ACE_TEXT (istream_control_p->name ().c_str ())));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::wait(), returning\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""))));
      return;
    }
  } // end IF

  // *NOTE*: the procedure here is this:
  //         step1: wait for (message source) processing to finish
  //         step2: wait for any upstreamed messages to 'flush' (message sink)
  typename inherited2::MODULE_LIST_T modules;
  modules.push_front (inherited::head ());

  // step1a: get head module (skip over ACE_Stream_Head)
  ITERATOR_T iterator (*this);
  if (iterator.done ())
    return;
  result = iterator.advance ();
  if (result == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  const typename inherited2::MODULE_T* module_p = NULL;
  result = iterator.next (module_p);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module found, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF
  // sanity check: head == tail ? possible reasons:
  // - no modules have been push()ed (yet)
  // - stream hasn't been intialized (at all)
  // --> nothing to do
  if (module_p == inherited::tail ())
    return;

  // ... and wait for the state switch (xxx --> FINISHED) (/ any head module
  // thread(s))
  ISTREAM_CONTROL_T* control_impl_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (const_cast<typename inherited2::MODULE_T*> (module_p)->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF
  try {
    control_impl_p->wait (false,
                          waitForUpStream_in,
                          waitForDownStream_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_IStreamControl::wait(), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  }

  // --> no new messages will be dispatched from the head module
  // *IMPORTANT NOTE*: in 'concurrent' scenarios, messages are dispatched by
  //                   'third parties' (i.e. external threads); the FINISHED
  //                   state must prevent more data from 'leaking' in beyond
  //                   this point

  // step1b: wait for inbound processing (i.e. the 'writer' side) pipeline to
  //         flush (/ any worker(s) to idle)
  // *NOTE*: 'final' trailing modules are remove()d from the stream by the
  //         shutdown logic (see: shutdown()).
  //         --> grab the lock to freeze the pipeline layout and prevent a race
  //             condition
  //         Note that this also removes them from the module configuration
  //         stack 'modules_'; alas, this fact effectively prevents proper
  //         flushing of messages here
  // *TODO*: handle this scenario by e.g. holding on to the stream lock while
  //         waiting (see: line 1766)
  ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);

  typename inherited2::MODULE_LIST_ITERATOR_T iterator_2 = modules_.begin ();
  TASK_T* task_p = NULL;
  ACE_Time_Value one_second (1, 0);
  size_t message_count = 0;
  ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> reverse_lock (lock_);
  do
  {
    // *WARNING*: if the stream is link()ed, and 'this' is 'upstream', the
    //            tail() module will not be hit

    // sanity check(s)
    ACE_ASSERT (module_p);

    // skip stream tail (i.e. last last module)
    if (ACE_OS::strcmp (module_p->name (), ACE_TEXT ("ACE_Stream_Tail")) == 0)
      break;

    modules.push_front (const_cast<typename inherited2::MODULE_T*> (module_p));

    task_p = const_cast<typename inherited2::MODULE_T*> (module_p)->writer ();
    if (!task_p)
      continue; // close()d already ?
    ACE_ASSERT (task_p->msg_queue_);
    do
    {
      //result = task_p->msg_queue_->wait ();
      message_count = task_p->msg_queue_->message_count ();
      if (!message_count)
        break;

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s/%s writer: waiting to process ~%d byte(s) in %u message(s)...\n"),
      //            ACE_TEXT (name_.c_str ()),
      //            module_p->name (),
      //            task_p->msg_queue_->message_bytes (), message_count));
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
        result = ACE_OS::sleep (one_second);
      } // end lock scope
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name (),
                    &one_second));
    } while (true);

    if (waitForThreads_in)
    {
      result = task_p->wait ();
      if (result == -1)
      {
        int error = ACE_OS::last_error ();
        ACE_UNUSED_ARG (error);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if (error != ENXIO) // *NOTE*: see also: common_task_base.inl:350
#endif
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s writer: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                      ACE_TEXT (name_.c_str ()),
                      module_p->name ()));
      } // end IF
    } // end IF

    if (!waitForDownStream_in &&
        (module_p == *iterator_2))
      break;

    iterator.advance ();
    module_p = NULL;
    result = iterator.next (module_p);
    if (!result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Stream_Iterator::next(): \"%m\", returning\n"),
                  ACE_TEXT (name_.c_str ())));
      return;
    } // end IF
  } while (true);

  // step2: wait for outbound/upstream processing (i.e. 'reader') pipeline to
  //        flush (/ any worker(s) to idle)
  for (typename inherited2::MODULE_LIST_ITERATOR_T iterator2 = modules.begin ();
       iterator2 != modules.end ();
       iterator2++)
  {
    task_p = (*iterator2)->reader ();
    if (!task_p) continue; // close()d already ?
    ACE_ASSERT (task_p->msg_queue_);
    do
    {
      //result = task_p->msg_queue_->wait ();
      message_count = task_p->msg_queue_->message_count ();
      if (!message_count) break;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s/%s reader: waiting to process ~%d byte(s) in %u message(s)...\n"),
      //            ACE_TEXT (name_.c_str ()),
      //            (*iterator2)->name (),
      //            task_p->msg_queue_->message_bytes (), message_count));
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX>, aGuard2, reverse_lock);
        result = ACE_OS::sleep (one_second);
      } // end lock scope
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator2)->name (),
                    &one_second));
    } while (true);

    if (waitForThreads_in)
    {
      result = task_p->wait ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s reader: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    (*iterator2)->name ()));
    } // end IF
  } // end FOR
}

//template <typename LockType,
//          ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename StatusType,
//          typename StateType,
//          typename ConfigurationType,
//          typename StatisticContainerType,
//          typename ModuleConfigurationType,
//          typename HandlerConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename SessionMessageType,
//          typename ProtocolMessageType>
//void
//Stream_Base_T<LockType,
//              ACE_SYNCH_USE,
//              TimePolicyType,
//              StatusType,
//              StateType,
//              ConfigurationType,
//              StatisticContainerType,
//              ModuleConfigurationType,
//              HandlerConfigurationType,
//              SessionDataType,
//              SessionDataContainerType,
//              SessionMessageType,
//              ProtocolMessageType>::waitForIdleState () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::waitForIdleState"));
//
//  int result = -1;
//
//  // *NOTE*: if this stream has been linked (e.g. connection is part of another
//  //         stream), make sure to wait for the whole pipeline
//  if (upStream_)
//  {
//    ISTREAM_CONTROL_T* istream_control_p =
//      dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
//    if (!istream_control_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
//                  upStream_));
//      return;
//    } // end IF
//    istream_control_p->waitForIdleState ();
//    //ACE_DEBUG ((LM_DEBUG,
//    //            ACE_TEXT ("upstream \"%s\" idle...\n"),
//    //            ACE_TEXT (istream_control_p->name ().c_str ())));
//  } // end IF
//
//  MODULE_CONTAINER_T modules;
//  MODULE_T* head_module_p = const_cast<OWN_TYPE_T*> (this)->head ();
//  modules.push_front (head_module_p);
//
//  // step1a: get head module, skip over ACE_Stream_Head
//  ITERATOR_T iterator (*this);
//  result = iterator.advance ();
//  if (result == 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("no head module found, returning\n")));
//    return;
//  } // end IF
//  const MODULE_T* module_p = NULL;
//  result = iterator.next (module_p);
//  if (result == 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("no head module found, returning\n")));
//    return;
//  } // end IF
//
//  modules.push_front (const_cast<MODULE_T*> (module_p));
//
//  // step1b: wait for (inbound) processing pipeline to flush
//  ITASK_T* itask_p = NULL;
//  MODULE_T* tail_module_p = const_cast<OWN_TYPE_T*> (this)->tail ();
//  TASK_T* task_p = NULL;
//  QUEUE_T* queue_p = NULL;
//  ACE_Time_Value one_second (1, 0);
//  size_t message_count = 0;
//  for (;
//       (iterator.next (module_p) != 0);
//       iterator.advance ())
//  {
//    // skip stream tail (last module)
//    if (module_p == tail_module_p)
//      continue; // done
//
//    modules.push_front (const_cast<MODULE_T*> (module_p));
//
//    if (module_p == head_module_p)
//    {
//      task_p = const_cast<MODULE_T*> (module_p)->writer ();
//      if (!task_p) continue; // close()d already ?
//      queue_p = task_p->msg_queue ();
//      if (!queue_p) continue;
//      do
//      {
//        //result = queue_p->wait ();
//        message_count = queue_p->message_count ();
//        if (!message_count) break;
//        //ACE_DEBUG ((LM_DEBUG,
//        //            ACE_TEXT ("%s writer: waiting to process ~%d byte(s) (in %u message(s))...\n"),
//        //            module_p->name (),
//        //            queue_p->message_bytes (), message_count));
//        result = ACE_OS::sleep (one_second);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
//                      module_p->name (),
//                      &one_second));
//      } while (true);
//      continue; // done
//    } // end IF
//
//    itask_p =
//        dynamic_cast<ITASK_T*> (const_cast<MODULE_T*> (module_p)->writer ());
//    if (!itask_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s writer: failed to dynamic_cast<Stream_ITask_T>(0x%@), continuing\n"),
//                  module_p->name (),
//                  const_cast<MODULE_T*> (module_p)->writer ()));
//      continue;
//    } // end IF
//    try {
//      itask_p->waitForIdleState ();
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: caught exception in Stream_ITask_T::waitForIdleState(), continuing\n"),
//                  module_p->name ()));
//      continue;
//    }
//  } // end FOR
//
//  // step2: wait for any upstreamed workers and messages to flush
//  for (MODULE_CONTAINER_ITERATOR_T iterator2 = modules.begin ();
//       iterator2 != modules.end ();
//       iterator2++)
//  {
//    if (*iterator2 == head_module_p)
//    {
//      task_p = const_cast<MODULE_T*> (*iterator2)->reader ();
//      if (!task_p) continue; // close()d already ?
//      queue_p = task_p->msg_queue ();
//      if (!queue_p) continue;
//      do
//      {
//        //result = queue_p->wait ();
//        message_count = queue_p->message_count ();
//        if (!message_count) break;
//        //ACE_DEBUG ((LM_DEBUG,
//        //            ACE_TEXT ("%s writer: waiting to process ~%d byte(s) (in %u message(s))...\n"),
//        //            module_p->name (),
//        //            queue_p->message_bytes (), message_count));
//        result = ACE_OS::sleep (one_second);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
//                      module_p->name (),
//                      &one_second));
//      } while (true);
//      continue; // done
//    } // end IF
//
//    itask_p =
//        dynamic_cast<ITASK_T*> (const_cast<MODULE_T*> (*iterator2)->reader ());
//    if (!itask_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s reader: failed to dynamic_cast<Stream_ITask_T>(0x%@), continuing\n"),
//                  (*iterator2)->name (),
//                  const_cast<MODULE_T*> (*iterator2)->reader ()));
//      continue;
//    } // end IF
//    try {
//      itask_p->waitForIdleState ();
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: caught exception in Stream_ITask_T::waitForIdleState(), continuing\n"),
//                  (*iterator2)->name ()));
//      continue;
//    }
//  } // end FOR
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
const typename Stream_IStream_T<ACE_SYNCH_USE, TimePolicyType>::MODULE_T*
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::find"));

  typename inherited2::STREAM_T* stream_p = const_cast<OWN_TYPE_T*> (this);

  // step1: search for the module on the current stream
  const ACE_TCHAR* name_p = ACE_TEXT_CHAR_TO_TCHAR (name_in.c_str ());
  const typename inherited2::MODULE_T* module_p = NULL;
  for (ITERATOR_T iterator (*stream_p);
       iterator.next (module_p);
       iterator.advance ())
    if (ACE_OS::strcmp (module_p->name (), name_p) == 0)
      return module_p;
  //result = stream_p->inherited::find (name_p);

  // step2: search (loaded) module configuration stack
  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, NULL);

    for (typename inherited2::MODULE_LIST_ITERATOR_T iterator = modules_.begin ();
         iterator != modules_.end ();
         iterator++)
      if (ACE_TEXT_ALWAYS_CHAR ((*iterator)->name ()) == name_in)
        return *iterator;
  } // end lock scope

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: module not found (name was: \"%s\"), aborting\n"),
              ACE_TEXT (name_.c_str ()),
              ACE_TEXT (name_in.c_str ())));

  return NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::link (typename inherited2::STREAM_T* stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // sanity check(s)
  ACE_ASSERT (stream_in);

  if (upStream_)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);
    if (!istream_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": failed to dynamic_cast<Stream_IStream_T>(0x%@), aborting\n"),
                  upStream_));
      return false;
    } // end IF
    try {
      return istream_p->link (stream_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStream_T::link(), aborting\n"),
                  ACE_TEXT (istream_p->name ().c_str ())));
      return false;
    }
  } // end IF

  int result = link (*stream_in);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::link(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));

  return (result == 0);
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::_unlink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::_unlink"));

  // sanity check(s)
  if (!upStream_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: no upstream; cannot unlink, returning\n"),
                ACE_TEXT (name_.c_str ())));
    return;
  } // end IF

  int result = unlink ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::unlink(): \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
ACE_Stream<ACE_SYNCH_USE, TimePolicyType>*
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::downStream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::downStream"));

  // sanity check(s)
  typename inherited2::MODULE_T* module_p =
    const_cast<OWN_TYPE_T*> (this)->inherited::head ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return NULL;
  } // end IF

  // step1: locate the second (!) downstream head module
  Common_IGetP_2_T<inherited2>* iget_p = NULL;
  bool is_first = true;
  do
  {
    if (!module_p->next ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to ACE_Module::next(), aborting\n"),
                  ACE_TEXT (name_.c_str ()),
                  module_p->name ()));
      return NULL;
    } // end IF
    iget_p = dynamic_cast<Common_IGetP_2_T<inherited2>*> (module_p->writer ());
    if (iget_p)
    {
      if (!is_first)
        break;
      is_first = false;
    } // end IF
    module_p = module_p->next ();
  } while (true);
  if (!iget_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to locate downstream head module, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return NULL;
  } // end IF
  inherited2* istream_p = const_cast<inherited2*> (iget_p->get_2 ());
  if (!istream_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve downstream handle, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return NULL;
  } // end IF
  inherited* stream_p = dynamic_cast<inherited*> (istream_p);
  if (!istream_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<ACE_Stream>(0x%@), aborting\n"),
                ACE_TEXT (name_.c_str ()),
                istream_p));
    return NULL;
  } // end IF

  return stream_p;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::lock (bool block_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::lock"));

  int result = -1;

  if (upStream_)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (upStream_);
    if (!ilock_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  upStream_));
      return false;
    } // end IF
    try {
      return ilock_p->lock (block_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": caught exception in Stream_ILock_T::lock(), aborting\n")));
    }
    return false;
  } // end IF

  // *IMPORTANT NOTE*: currently,
  //                   ACE_Recursive_Thread_Mutex::get_nesting_level() is not
  //                   supported on non-Win32 platforms (returns -1, see:
  //                   Recursive_Thread_Mutex.cpp:96)
  //                   --> use result of the locking operation instead
  int previous_nesting_level = lock_.get_nesting_level ();
  //ACE_recursive_thread_mutex_t& mutex_r = lock_.lock ();

  result = (block_in ? lock_.acquire () : lock_.tryacquire ());
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error == EBUSY)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[%T][%t]: lock %sheld by %d --> false\n"),
//                  (block_in ? ACE_TEXT ("(block) ") : ACE_TEXT ("")),
//                  mutex_r.OwningThread));
#endif
      return false;
    } // end IF
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("[%T][%t]: lock %s%d --> unlock ? %s\n"),
  //            (block_in ? ACE_TEXT ("(block) ") : ACE_TEXT ("")),
  //            lock_.get_nesting_level (),
  //            ((lock_.get_nesting_level () != nesting_level) ? ACE_TEXT ("true") : ACE_TEXT ("false"))));

  int current_nesting_level = lock_.get_nesting_level ();
  return ((current_nesting_level > 0) ? (current_nesting_level != previous_nesting_level)
                                      : !result);
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlock (bool unlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlock"));

  int result = -1;

  if (upStream_)
  {
    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (upStream_);
    if (!ilock_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  upStream_));
      return -1;
    } // end IF
    try {
      return ilock_p->unlock (unlock_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": caught exception in Stream_ILock_T::unlock(), aborting\n")));
    }
    return -1;
  } // end IF

  // *TODO*: on Win32 platforms, the ACE implementation does not currently
  //         support ACE_Recursive_Thread_Mutex::get_thread_id(), although the
  //         data type 'struct _RTL_CRITICAL_SECTION' contains the necessary
  //         information ('OwningThread')
  //         --> clean up these inconsistencies and submit a bug report/patch
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_recursive_thread_mutex_t& mutex_r = lock_.lock ();
  if (!ACE_OS::thr_equal (reinterpret_cast<ACE_thread_t> (mutex_r.OwningThread),
                          ACE_OS::thr_self ()))
#elif !defined (ACE_HAS_RECURSIVE_MUTEXES)
  // IMPORTANT NOTE*: currently, ACE_Recursive_Thread_Mutex::get_thread_id() is
  //                  not supported on non-Win32 platforms (returns
  //                  ACE_OS::NULL_thread), see: Recursive_Thread_Mutex.cpp:70)
  ACE_thread_t thread_id = lock_.get_thread_id ();
  if (!ACE_OS::thr_equal (thread_id, ACE_OS::NULL_thread) &&
      !ACE_OS::thr_equal (thread_id, ACE_OS::thr_self ()))
#else
  goto continue_;
#endif
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("[%T][%t]: unlock held by %d --> -1\n"),
    //            mutex_r.OwningThread));
#endif
    return -1;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif !defined (ACE_HAS_RECURSIVE_MUTEXES)
#else
continue_:
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = (mutex_r.RecursionCount > 0 ? mutex_r.RecursionCount - 1 : 0);
  int result_2 = -1;
  do
  {
    result_2 = lock_.release ();
    if (!unlock_in) break;
  } while (mutex_r.RecursionCount > 0);
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                ACE_TEXT (name_.c_str ())));
#else
  // *IMPORTANT NOTE*: currently,
  //                   ACE_Recursive_Thread_Mutex::get_nesting_level() is not
  //                   supported on non-Win32 platforms (returns -1, see:
  //                   Recursive_Thread_Mutex.cpp:96)
  int previous_nesting_level = lock_.get_nesting_level ();
  result = ((previous_nesting_level > 0) ? (previous_nesting_level - 1) : 0);

  int result_2 = -1;
  bool is_first_iteration = true;
  do
  {
    result_2 = lock_.release ();
    if (result_2 == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EPERM) // not locked ?
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", returning\n"),
                    ACE_TEXT (name_.c_str ())));
        break;
      } // end IF
      else if (!is_first_iteration)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", returning\n"),
                    ACE_TEXT (name_.c_str ())));
        break;
      } // end ELSE IF
      else
      { // --> failed on first iteration: was not locked
        result = -1;
        break;
      } // end ELSE
    } // end IF
    if (!unlock_in) break;
    is_first_iteration = false;
  } while (lock_.get_nesting_level () > 0);
#endif
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("[%T][%t]: unlock %s%d --> %d\n"),
  //            (unlock_in ? ACE_TEXT ("(full) ") : ACE_TEXT ("")),
  //            lock_.get_nesting_level (),
  //            result));

  return result;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
ACE_SYNCH_RECURSIVE_MUTEX&
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::getLock ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::getLock"));

  if (upStream_)
  {
    ACE_SYNCH_RECURSIVE_MUTEX dummy;

    ILOCK_T* ilock_p = dynamic_cast<ILOCK_T*> (upStream_);
    if (!ilock_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  upStream_));
      return dummy;
    } // end IF
    try {
      return ilock_p->getLock ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": caught exception in Stream_ILock_T::getLock(), aborting\n")));
    }
  } // end IF

  return lock_;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::hasLock ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::hasLock"));

  if (upStream_)
  {
    int result = -1;
    typename inherited2::MODULE_T* module_p = NULL;
    TASK_T* task_p = NULL;
    ILOCK_T* ilock_p = NULL;

    result = upStream_->top (module_p);
    if ((result == -1) || !module_p)
    {
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("%s: no head module found: \"%m\", aborting\n"),
      //            ACE_TEXT (upStream_->name ().c_str ())));
      return false; // *WARNING*: false negative
    } // end IF
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);

    ilock_p = dynamic_cast<ILOCK_T*> (task_p);
    if (!ilock_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("/%s: failed to dynamic_cast<Stream_ILock_T>(0x%@), aborting\n"),
                  module_p->name (),
                  task_p));
      return false; // *WARNING*: false negative
    } // end IF
    return ilock_p->hasLock ();
  } // end IF

  // *TODO*: on Windows platforms, the current ACE implementation does not
  //         support ACE_Recursive_Thread_Mutex::get_thread_id(), although the
  //         data type 'struct _RTL_CRITICAL_SECTION' contains the necessary
  //         information ('OwningThread')
  //         --> submit a bug report
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_recursive_thread_mutex_t& mutex_r = lock_.lock ();
  return (ACE_OS::thr_equal (reinterpret_cast<ACE_thread_t> (mutex_r.OwningThread),
                             ACE_OS::thr_self ()) &&
          (lock_.get_nesting_level () > 0));
#else
  return (ACE_OS::thr_equal (lock_.get_thread_id (), ACE_OS::thr_self ()) &&
          (lock_.get_nesting_level () > 0));
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::dump_state"));

  if (upStream_)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);
    ACE_ASSERT (istream_p);
    try {
      istream_p->dump_state ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT (": caught exception in Common_IDumpState(), continuing\n")));
    }
    return;
  } // end IF

  std::string stream_layout_string;

  const typename inherited2::MODULE_T* module_p = NULL;
  //const MODULE_T* tail_p = const_cast<OWN_TYPE_T*> (this)->tail ();
  //ACE_ASSERT (tail_p);
  for (ITERATOR_T iterator (*this);
       iterator.next (module_p);
       iterator.advance ())
  {
    stream_layout_string.append (ACE_TEXT_ALWAYS_CHAR (module_p->name ()));

    // avoid trailing "-->"
    if (ACE_OS::strcmp (module_p->name (), ACE_TEXT ("ACE_Stream_Tail")))
      stream_layout_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

    module_p = NULL;
  } // end FOR

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: \"%s\"\n"),
              ACE_TEXT (name_.c_str ()),
              ACE_TEXT (stream_layout_string.c_str ())));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::set (SessionDataContainerType*& sessionData_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::set"));

  // sanity check(s)
  ACE_ASSERT (sessionData_inout);

  // clean up
  if (sessionData_)
    sessionData_->decrease ();

  sessionData_ = sessionData_inout;
  sessionData_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::initialize (const ConfigurationType& configuration_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  IMODULE_T* imodule_p = NULL;
  typename inherited2::MODULE_T* module_p = NULL;
//  int result = -1;

  if (isInitialized_)
  {
    // handle final module (if any)
    // *NOTE*: this module may be shared by multiple stream instances, so it
    //         must not be close()d here
    if (hasFinal_)
    {
      if (state_.module)
      {
        module_p = inherited::find (state_.module->name ());
        if (module_p)
        {
          if (!remove (module_p,
                       true))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                        ACE_TEXT (name_.c_str ()),
                        state_.module->name ()));
        } // end IF
      } // end IF

      if (state_.deleteModule)
      {
        ACE_ASSERT (state_.module);
        delete state_.module;

        state_.deleteModule = false;
      } // end IF
      state_.module = NULL;

      { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
        modules_.pop_front ();
      } // end lock scope
      hasFinal_ = false;
    } // end IF

    if (!finalize ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::finalize(), aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      return false;
    } // end IF

    configuration_ = NULL;

    // *NOTE*: finalize() calls close(), resetting the task handles
    //         of all (enqueued) modules --> reset them manually
    { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);

      for (typename inherited2::MODULE_LIST_ITERATOR_T iterator = modules_.begin ();
           iterator != modules_.end ();
           iterator++)
      {
        imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
        if (!imodule_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                      ACE_TEXT (name_.c_str ()),
                      (*iterator)->name ()));
          return false;
        } // end IF
        try {
          imodule_p->reset ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: caught exception in Stream_IModule_T::reset(), continuing\n"),
                      ACE_TEXT (name_.c_str ()),
                      (*iterator)->name ()));
        }
      } // end FOR
    } // end lock scope

    if (configuration_inout.resetSessionData &&
        sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  // *TODO*: remove type inferences
  if (configuration_inout.module)
  {
    // sanity check(s)
    ACE_ASSERT (!hasFinal_);
    ACE_ASSERT (!state_.module);

    imodule_p = dynamic_cast<IMODULE_T*> (configuration_inout.module);
    if (!imodule_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                  ACE_TEXT (name_.c_str ()),
                  configuration_inout.module->name ()));
      return false;
    } // end IF

    // step1: clone final module (if any) ?
    if (configuration_inout.cloneModule)
    {
      module_p = NULL;
      try {
        module_p = imodule_p->clone ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: caught exception in Stream_IModule_T::clone(), aborting\n"),
                    ACE_TEXT (name_.c_str ()),
                    configuration_inout.module->name ()));
        module_p = NULL;
      }
      if (!module_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Stream_IModule_T::clone(), aborting\n"),
                    ACE_TEXT (name_.c_str ()),
                    configuration_inout.module->name ()));
        return false;
      } // end IF
      state_.module = module_p;
      state_.deleteModule = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s: cloned final module (handle: 0x%@, clone handle is: 0x%@)\n"),
                  ACE_TEXT (name_.c_str ()),
                  configuration_inout.module->name (),
                  configuration_inout.module,
                  state_.module));

      imodule_p = dynamic_cast<IMODULE_T*> (module_p);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
                    ACE_TEXT (name_.c_str ()),
                    configuration_inout.module->name ()));

        // clean up
        state_.module = NULL;
        state_.deleteModule = false;
        delete module_p;

        return false;
      } // end IF
    } // end IF
    else
    {
      // *NOTE*: if the module is ever reused, the reader/writer handles may
      //         need to be reset
      imodule_p->reset ();

      state_.module = configuration_inout.module;
      state_.deleteModule = configuration_inout.deleteModule;
    } // end ELSE
    ACE_ASSERT (state_.module);

    { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, false);
      modules_.push_front (state_.module);
    } // end lock scope
    hasFinal_ = true;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_inout);
  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_->moduleConfiguration);
  configuration_->moduleConfiguration->notify = this;
  for (CONFIGURATION_ITERATOR_T iterator = configuration_->moduleHandlerConfigurations.begin ();
       iterator != configuration_->moduleHandlerConfigurations.end ();
       iterator++)
  {
    //(*iterator).second->stateMachineLock = state_.stateMachineLock;
    (*iterator).second->stream = this;
  } // end FOR
  state_.userData = configuration_->userData;

  initialize (configuration_inout.setupPipeline,
              configuration_inout.resetSessionData);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::get (ACE_Message_Block*& messageBlock_inout,
                                        ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::get"));

  return (upStream_ ? upStream_->get (messageBlock_inout, timeout_in)
                    : inherited::get (messageBlock_inout, timeout_in));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::link (typename inherited2::STREAM_T& upStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // *WARNING*: cannot reach the base class lock --> not thread-safe !
  // *TODO*: submit change request to the ACE maintainers

  ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (&upStream_in);
  std::string upstream_name_string;
  if (istream_p)
    upstream_name_string = istream_p->name ();

  // sanity check(s)
  typename inherited2::MODULE_T* module_p = upStream_in.head ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF

  // locate the module just above the upstreams' tail
  typename inherited2::MODULE_T* upstream_tail_module_p = upStream_in.tail ();
  if (!upstream_tail_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::tail(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF
  do
  {
    if (!module_p->next ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to ACE_Module::next(), aborting\n"),
                  ACE_TEXT (upstream_name_string.c_str ()),
                  module_p->name ()));
      return -1;
    } // end IF
    if (!ACE_OS::strcmp (module_p->next ()->name (),
                         upstream_tail_module_p->name ()))
      break;
    module_p = module_p->next ();
  } while (true);

  //int result = inherited::link (upStream_in);
  typename inherited2::MODULE_T* head_module_p = inherited::head ();
  if (!head_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF
  typename inherited2::MODULE_T* heading_module_p = head_module_p->next ();
  if (!heading_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to ACE_Module::next(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ()),
                head_module_p->name ()));
    return -1;
  } // end IF
  heading_module_p->reader ()->next (module_p->reader ());
  module_p->next (heading_module_p);
  module_p->writer ()->next (heading_module_p->writer ());

  ////////////////////////////////////////

  SessionDataType* session_data_p = NULL;
  SessionDataType* session_data_2 = NULL;

  // ((re-)lock /) update configuration
  // *IMPORTANT NOTE*: in fully synchronous, or 'concurrent' scenarios, with
  //                   non-reentrant modules, the caller needs to hold the
  //                   stream lock(s) to securely perform this operation.
  //                   These issues need to be considered:
  //                   - data processing may fail when the pipeline layout is
  //                     suddenly changed (see above)
  //                   - downstream threads synchronizing with upstream, for
  //                     whatever reason, will 'desynchronize' (and likely
  //                     deadlock soon after) when the stream lock interface is
  //                     switched (e.g. due to an asynchronous unlink())
  //                   - threads accessing session data must block until it
  //                     has been merged between both streams
  //                   - ...
  // *TODO*: this needs more work
  int nesting_level = unlock (true);

  if (!istream_p)
    goto continue_;
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  for (CONFIGURATION_ITERATOR_T iterator = configuration_->moduleHandlerConfigurations.begin ();
       iterator != configuration_->moduleHandlerConfigurations.end ();
       iterator++)
    (*iterator).second->stream = istream_p;

continue_:
  // merge upstream session data
  OWN_TYPE_T* stream_p = dynamic_cast<OWN_TYPE_T*> (&upStream_in);
  SessionDataContainerType* session_data_container_p = NULL;
  if (!sessionData_ || !stream_p)
    goto done;
  session_data_container_p =
      &const_cast<SessionDataContainerType&> (stream_p->get ());
  if (!session_data_container_p)
    goto done;

  // *TODO*: this next line doesn't work unless the upstream cannot just go
  //         away (see discussion above)
  //         --> make Stream_Base_T::get() return a reference directly
  session_data_container_p->increase ();
  session_data_p =
    &const_cast<SessionDataType&> (session_data_container_p->get ());
  session_data_2 =
    &const_cast<SessionDataType&> (sessionData_->get ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, sessionDataLock_, -1);
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard_2, stream_p->sessionDataLock_, -1);

    // *NOTE*: the idea here is to 'merge' the two datasets
    *session_data_p += *session_data_2;

    // switch session data
    sessionData_->decrease ();
    sessionData_ = session_data_container_p;
  } // end lock scope

done:
  // *NOTE*: ACE_Stream::linked_us_ is currently private
  //         --> retain another handle
  // *TODO*: modify ACE to make this a protected member
  upStream_ = &upStream_in;

  // relock ?
  if (nesting_level >= 0)
    COMMON_ILOCK_ACQUIRE_N (this, nesting_level + 1);

  // notify pipeline modules
  control (STREAM_CONTROL_LINK,
           true);

  return 0;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlink (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlink"));

  // *WARNING*: cannot reach the base class lock --> not thread-safe !
  // *TODO*: submit change request to the ACE people

  ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);
  std::string upstream_name_string;
  if (istream_p)
    upstream_name_string = istream_p->name ();

  // sanity check(s)
  if (!upStream_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: no upstream; cannot unlink, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF
  typename inherited2::MODULE_T* module_p = upStream_->head ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF

  // locate the module just above the upstreams' tail
  typename inherited2::MODULE_T* head_module_p = inherited::head ();
  if (!head_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF
  typename inherited2::MODULE_T* heading_module_p = head_module_p->next ();
  if (!heading_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to ACE_Module::next(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ()),
                head_module_p->name ()));
    return -1;
  } // end IF
  do
  {
    if (!module_p->next ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to ACE_Module::next(): \"%m\", aborting\n"),
                  ACE_TEXT (upstream_name_string.c_str ()),
                  module_p->name ()));
      return -1;
    } // end IF
    if (!ACE_OS::strcmp (module_p->next ()->name (),
                         heading_module_p->name ()))
      break;

    module_p = module_p->next ();
  } while (true);
  ACE_ASSERT (module_p);

  TASK_T* task_p = heading_module_p->reader ();
  ACE_ASSERT (task_p);
  task_p->next (head_module_p->reader ());
  typename inherited2::MODULE_T* upstream_tail_module_p = upStream_->tail ();
  if (!upstream_tail_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::tail(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF
  module_p->next (upstream_tail_module_p);
  task_p = module_p->writer ();
  ACE_ASSERT (task_p);
  task_p->next (upstream_tail_module_p->writer ());

  // ((re-)lock /) update configuration
  int nesting_level = unlock (true);

  ACE_ASSERT (configuration_);
  for (CONFIGURATION_ITERATOR_T iterator = configuration_->moduleHandlerConfigurations.begin ();
       iterator != configuration_->moduleHandlerConfigurations.end ();
       iterator++)
    (*iterator).second->stream = this;

  upStream_ = NULL;

  // relock ?
  if (nesting_level >= 0)
    COMMON_ILOCK_ACQUIRE_N (this, nesting_level + 1);

  // notify pipeline modules
  control (STREAM_CONTROL_UNLINK,
           false);

  return 0;
}

//template <ACE_SYNCH_DECL,
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
//Stream_Base_T<ACE_SYNCH_USE,
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::remove (typename inherited2::MODULE_T* module_in,
                                           bool reset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::remove"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (module_in);

  // *NOTE*: start with any trailing module(s) and work forwards

  // step1: retrieve head/tail modules
  typename inherited2::MODULE_T* head_p = inherited::head ();
  typename inherited2::MODULE_T* tail_p = inherited::tail ();
  ACE_ASSERT ((head_p &&
               ((ACE_OS::strcmp (head_p->name (),
                                 ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"))       == 0) ||
                (ACE_OS::strcmp (head_p->name (),
                                 ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_HEAD_NAME)) == 0))) &&
              (tail_p &&
               (ACE_OS::strcmp (tail_p->name (),
                                ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Tail")) == 0)));
  if ((ACE_OS::strcmp (module_in->name (), head_p->name ()) == 0) ||
      (ACE_OS::strcmp (module_in->name (), tail_p->name ()) == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: cannot remove head or tail module (name was: \"%s\"), aborting\n"),
                ACE_TEXT (name_.c_str ()),
                module_in->name ()));
    return false;
  } // end IF

  // step2: remove chain (back-to-front)
  std::deque<typename inherited2::MODULE_T*> modules;
  typename inherited2::MODULE_T* previous_p = NULL;
  typename inherited2::MODULE_T* module_p = head_p;
  while (ACE_OS::strcmp (module_p->name (),
                         module_in->name ()))
  {
    previous_p = module_p;
    module_p = module_p->next ();
  } // end WHILE
  module_p = module_in;
  while (ACE_OS::strcmp (module_p->name (), tail_p->name ()))
  {
    modules.push_front (module_p);
    module_p = module_p->next ();
  } // end WHILE

  IMODULE_T* imodule_p = NULL;
  while (!modules.empty ())
  {
    module_p = modules.front ();

    // *NOTE*: ACE_Stream::remove()ing a module close()s it, resetting the task
    //         handles
    //         --> call reset() so it can be reused
    // *NOTE*: 'final' modules may be in use by multiple streams, and may
    //         therefore not be close()d/reset()ed
    imodule_p = dynamic_cast<IMODULE_T*> (module_p);
    if (!imodule_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to dynamic_cast<Stream_IModule_T>(0x%@), aborting\n"),
                  ACE_TEXT (name_.c_str ()),
                  module_p->name (),
                  module_p));
      return false;
    } // end IF
    if (reset_in && !imodule_p->isFinal ())
    {
      result = module_p->close (ACE_Module_Base::M_DELETE_NONE);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to ACE_Module::close(M_DELETE_NONE): \"%m\", aborting\n"),
                    ACE_TEXT (name_.c_str ()),
                    module_p->name ()));
        return false;
      } // end IF
      imodule_p->reset ();
    } // end IF

    modules.pop_front ();
  } // end WHILE
  previous_p->link (tail_p);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: removed module \"%s\"...\n"),
              ACE_TEXT (name_.c_str ()),
              module_in->name ()));

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::finished (bool finishUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finished"));

  int result = -1;

  typename inherited2::MODULE_T* module_p = NULL;
  STATEMACHINE_ICONTROL_T* control_impl_p = NULL;

  // *NOTE*: if this stream has been linked (e.g. connection is part of another
  //         stream), make sure to finished() the whole pipeline
  if (upStream_ && finishUpStream_in)
  {
    ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (upStream_);

    // delegate to the head module
    result = upStream_->top (module_p);
    if ((result == -1) || !module_p)
    {
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("no head module found: \"%m\", continuing\n")));
      goto _continue;
    } // end IF

    control_impl_p =
      dynamic_cast<STATEMACHINE_ICONTROL_T*> (module_p->writer ());
    if (!control_impl_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: dynamic_cast<Stream_StateMachine_IControl_T> failed, continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name ()));
      goto _continue;
    } // end IF

    try {
      control_impl_p->finished ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: caught exception in Stream_StateMachine_IControl_T::finished(), continuing\n"),
                  (istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT ("")),
                  module_p->name ()));
      goto _continue;
    }
  } // end IF

_continue:
  // delegate to the head module
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("no head module found: \"%m\", returning\n")));
    return;
  } // end IF

  control_impl_p =
      dynamic_cast<STATEMACHINE_ICONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<Stream_StateMachine_IControl_T> failed, returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  } // end IF

  try {
    control_impl_p->finished ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: caught exception in Stream_StateMachine_IControl_T::finished(), returning\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    return;
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::shutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::shutdown"));

//  int result = -1;

  // step0: if not properly initialized, this needs to deactivate any hitherto
  // enqueued ACTIVE modules, or the stream will wait forever during closure...
  // --> possible scenarios:
  // - (re-)init() failed halfway through (i.e. MAYBE some modules push()ed
  //   correctly)
  typename inherited2::MODULE_T* module_p = NULL;
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
                    ACE_TEXT ("%s: not initialized - deactivating module(s)...\n"),
                    ACE_TEXT (name_.c_str ())));

        deactivateModules ();

        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: not initialized - deactivating module(s)...DONE\n"),
                    ACE_TEXT (name_.c_str ())));
      } // end IF
    } // end IF
  } // end IF
  else
  {
    // handle final module (if any)
    // *NOTE*: this module may be shared by multiple stream instances, so it
    //         must not be close()d here
    if (hasFinal_)
    {
      if (state_.module)
      {
        module_p = inherited::find (state_.module->name ());
        if (module_p)
        {
          if (!remove (module_p,
                       true))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                        ACE_TEXT (name_.c_str ()),
                        state_.module->name ()));
        } // end IF
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: removed module \"%s\"...\n"),
                      ACE_TEXT (name_.c_str ()),
                      state_.module->name ()));
      } // end IF

      if (state_.deleteModule)
      {
        ACE_ASSERT (state_.module);
        delete state_.module;

        state_.deleteModule = false;
      } // end IF
      state_.module = NULL;

      { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_);

        modules_.pop_front ();
      } // end lock scope
      hasFinal_ = false;
    } // end IF
  } // end ELSE

  // step1: iterator over modules which are NOT on the stream
  //        --> close() these manually before they do so in their dtors
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...\n")));

  //for (Stream_ModuleListIterator_t iterator = modules_.begin ();
  //     iterator != modules_.end ();
  //     iterator++)
  //{
  //  // sanity check: on the stream ?
  //  if ((*iterator)->next () == NULL)
  //  {
  //    //ACE_DEBUG ((LM_WARNING,
  //    //            ACE_TEXT ("closing module: \"%s\"\n"),
  //    //            module->name ()));

  //    try {
  //      result = (*iterator)->close (ACE_Module_Base::M_DELETE_NONE);
  //    } catch (...) {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
  //                  (*iterator)->name ()));
  //      result = -1;
  //    }
  //    if (result == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%m\", continuing\n"),
  //                  (*iterator)->name ()));
  //  } // end IF
  //} // end FOR

  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...DONE\n")));

  // step2: shutdown stream
  // check the ACE documentation on ACE_Stream to understand why this is needed
  // *TODO*: ONLY do this if stream_head != 0 !!! (warning: obsolete ?)
  // *NOTE*: will NOT destroy all modules in the current stream, as this leads
  //         to exceptions in debug builds under MS Windows (can't delete object
  //         in a different DLL than where it was created...)
  //         --> do this manually !
  //         this invokes close() on each module (and waits for any worker
  //         thread(s) to return)
  if (!finalize ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::finalize(): \"%m\", continuing\n"),
                ACE_TEXT (name_.c_str ())));

  // *NOTE*: every ACTIVE module will join with its worker thread in close()
  //         --> ALL stream-related threads should have returned by now
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::deactivateModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::deactivateModules"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  // *TODO*: remove type inference
  // *NOTE*: session message assumes responsibility for session data
  //         --> add a reference
  sessionData_->increase ();

  // allocate SESSION_END session message
  SessionMessageType* message_p = NULL;
  // *TODO*: remove type inference
  if (configuration_->messageAllocator)
  {
    try { // *NOTE*: 0 --> session message
      message_p =
        static_cast<SessionMessageType*> (configuration_->messageAllocator->malloc (0));
    } catch (...) {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(0), returning\n"),
                  ACE_TEXT (name_.c_str ())));

      // clean up
      sessionData_->decrease ();

      return;
    }
  } // end IF
  else
  {
    // *TODO*: remove type inference
    ACE_NEW_NORETURN (message_p,
                      SessionMessageType (STREAM_SESSION_MESSAGE_END,
//                                          session_data_container_p,
                                          sessionData_,
                                          state_.userData));
  } // end ELSE
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate SessionMessageType: \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));

    // clean up
    sessionData_->decrease ();

    return;
  } // end IF
  if (configuration_->messageAllocator)
  {
    // *TODO*: remove type inference
    message_p->initialize (STREAM_SESSION_MESSAGE_END,
                           sessionData_,
                           state_.userData);
  } // end IF

  // send message downstream
  int result = inherited::put (message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::put(): \"%m\", returning\n"),
                ACE_TEXT (name_.c_str ())));

    // clean up
    message_p->release ();

    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Base_T<ACE_SYNCH_USE,
              TimePolicyType,
              ControlType,
              NotificationType,
              StatusType,
              StateType,
              ConfigurationType,
              StatisticContainerType,
              ModuleConfigurationType,
              HandlerConfigurationType,
              SessionDataType,
              SessionDataContainerType,
              ControlMessageType,
              DataMessageType,
              SessionMessageType>::unlinkModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlinkModules"));

  typename inherited2::MODULE_T* head_p = inherited::head ();
  for (typename inherited2::MODULE_T* module_p = head_p;
       module_p;
       module_p = module_p->next ())
    module_p->next (NULL);
  head_p->next (inherited::tail ());
}
