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

#include "stream_data_base.h"
#include "stream_iallocator.h"
#include "stream_imessagequeue.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::Stream_Base_T (const std::string& name_in)
// *TODO*: use default ctor and rely on init/fini() ?
 : inherited (NULL, // argument to module open()
              NULL, // no head module --> allocate !
              NULL) // no tail module --> allocate !
 , configuration_ (NULL)
 , isInitialized_ (false)
 , lock_ ()
 , modules_ ()
 , sessionData_ (NULL)
 , state_ ()
 , upStream_ (NULL)
 ////////////////////////////////////////
 , hasFinal_ (false)
 , name_ (name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::Stream_Base_T"));

  int result = inherited::open (NULL,
                                NULL,
                                NULL);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::open(): \"%m\", continuing\n")));
  // *TODO*: this really shouldn't be necessary
  inherited::head ()->next (inherited::tail ());
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::~Stream_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::~Stream_Base_T"));

  int result = -1;

  if (state_.module)
  {
    MODULE_T* module_p = inherited::find (state_.module->name ());
    if (module_p)
    {
      result = inherited::remove (state_.module->name (),
                                  ACE_Module_Base::M_DELETE_NONE);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
                    state_.module->name ()));
    } // end IF

    if (state_.deleteModule)
      delete state_.module;
  } // end IF

  if (sessionData_)
    sessionData_->decrease ();
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::reset"));

  bool result = false;

  // pop/close all modules
  // *NOTE*: will implicitly (blocking !) wait for any active worker threads
  result = finalize ();
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::finalize(), continuing\n")));

  // - reset reader/writers tasks for ALL modules
  // - re-initialize head/tail modules
  initialize (true, true);

  return result;
}
template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::setup (ACE_Notification_Strategy* notificationStrategy_in)
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
                ACE_TEXT ("caught exception in ACE_Stream::open(), aborting\n")));
    result = -1;
  }
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::open(): \"%m\", aborting\n")));
    return false;
  } // end IF
  // *TODO*: this really shouldn't be necessary
  inherited::head ()->next (inherited::tail ());

  // step2: set notification strategy ?
  MODULE_T* module_p = NULL;
  if (notificationStrategy_in)
  {
    module_p = inherited::head ();
    if (!module_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module found, aborting\n")));
      return false;
    } // end IF
    TASK_T* task_p = module_p->reader ();
    if (!task_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module reader task found, aborting\n")));
      return false;
    } // end IF
    QUEUE_T* queue_p = task_p->msg_queue ();
    if (!queue_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module reader task queue found, aborting\n")));
      return false;
    } // end IF
    queue_p->notification_strategy (notificationStrategy_in);
  } // end IF

  // step3: push all available modules
  for (Stream_ModuleListIterator_t iterator = modules_.begin ();
       iterator != modules_.end ();
       iterator++)
  {
    result = inherited::push (*iterator);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                  (*iterator)->name ()));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("pushed \"%s\"...\n"),
                (*iterator)->name ()));
  } // end IF
#if defined (_DEBUG)
  dump_state ();
#endif

  return true;
}

//template <typename LockType,
//          typename TaskSynchType,
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
//bool
//Stream_Base_T<LockType,
//              TaskSynchType,
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
//              ProtocolMessageType>::putSessionMessage (Stream_SessionMessageType sessionMessageType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::putSessionMessage"));
//
//  int result = -1;
//
//  // delegate to the head module
//  MODULE_T* module_p = NULL;
//  result = inherited::top (module_p);
//  if ((result == -1) || !module_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("no head module found: \"%m\", aborting\n")));
//    return false;
//  } // end IF
//
//  ISTREAM_CONTROL_T* control_impl_p =
//    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
//  if (!control_impl_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, aborting\n"),
//                module_p->name ()));
//    return;
//  } // end IF
//
//  try {
//    control_impl_p->control ();
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::control(), aborting\n"),
//                module_p->name ()));
//    return;
//  }
//}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::initialize (bool setupPipeline_in,
                                                bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isInitialized_);

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
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
      return;
    } // end IF
    // *TODO*: remove type inferences
    session_data_p->lock = &lock_;
    state_.currentSessionData = session_data_p;

    // *IMPORTANT NOTE*: fire-and-forget API (session_data_p)
    ACE_NEW_NORETURN (sessionData_,
                      SessionDataContainerType (session_data_p));
    if (!sessionData_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

      // clean up
      state_.currentSessionData = NULL;
      delete session_data_p;

      return;
    } // end IF
  } // end IF

  if (!load (modules_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStreamControlBase::load(), returning\n")));
    return;
  } // end IF
  // *TODO* fix ACE bug: modules should initialize their 'next_' member
  for (Stream_ModuleListIterator_t iterator = modules_.begin ();
       iterator != modules_.end ();
       iterator++)
    (*iterator)->next (NULL);

  // step2: setup pipeline ?
  if (setupPipeline_in)
  {
    // sanity check(s)
    ACE_ASSERT (configuration_);
    ACE_ASSERT (configuration_->moduleHandlerConfiguration);

    IMODULE_T* imodule_p = NULL;
    Stream_Task_t* task_p = NULL;
    MODULEHANDLER_IINITIALIZE_T* iinitialize_p = NULL;
    for (Stream_ModuleListIterator_t iterator = modules_.begin ();
         iterator != modules_.end ();
         iterator++)
    {
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, returning\n"),
                    (*iterator)->name ()));
        return;
      } // end IF
      if (!imodule_p->initialize (*configuration_->moduleConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_IInitialize_T::initialize(), aborting\n"),
                    (*iterator)->name ()));
        return;
      } // end IF

      task_p = (*iterator)->writer ();
      ACE_ASSERT (task_p);
      iinitialize_p = dynamic_cast<MODULEHANDLER_IINITIALIZE_T*> (task_p);
      if (!iinitialize_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Common_IInitialize_T<HandlerConfigurationType>> failed, returning\n"),
                    (*iterator)->name ()));
        return;
      } // end IF
      if (!iinitialize_p->initialize (*configuration_->moduleHandlerConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_IInitialize_T::initialize(), aborting\n"),
                    (*iterator)->name ()));
        return;
      } // end IF

      //// *TODO* fix ACE bug: modules should initialize their 'next_' member
      //(*iterator)->next (NULL);
    } // end IF

    if (!setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Base_T::setup(), returning\n")));
      return;
    } // end IF
  } // end IF

  //// step3: initialize state machine
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

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::finalize ()
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
                ACE_TEXT ("caught exception in ACE_Stream::close(M_DELETE_NONE), continuing\n")));
    result = -1;
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::close(M_DELETE_NONE): \"%m\", aborting\n")));

  return (result == 0);
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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

  try {
    control_impl_p->start ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::start(), returning\n"),
                module_p->name ()));
    return;
  }
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::stop (bool waitForCompletion_in,
                                          bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::stop"));

  int result = -1;
  MODULE_T* module_p = NULL;
  ISTREAM_CONTROL_T* control_impl_p = NULL;

  // has upstream ? --> (try to) stop that instead
  if (upStream_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: stopping upstream...\n"),
    //            ACE_TEXT (name ().c_str ())));

    // delegate to the head module, skip over ACE_Stream_Head...
    result = upStream_->top (module_p);
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
    control_impl_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
    if (!control_impl_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl_T> failed, returning\n"),
                  module_p->name ()));
      return;
    } // end IF

    try {
      control_impl_p->stop (waitForCompletion_in,
                            lockedAccess_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl::stop(), returning\n"),
                  module_p->name ()));
      return;
    }

    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: stopping upstream...done\n"),
    //            ACE_TEXT (name ().c_str ())));
  } // end IF

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
  control_impl_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!control_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try {
    control_impl_p->stop (waitForCompletion_in,
                          lockedAccess_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::stop(), returning\n"),
                module_p->name ()));
    return;
  }

wait:
  if (waitForCompletion_in)
    waitForCompletion ();
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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

  try {
    return control_impl_p->isRunning ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::isRunning(), aborting\n"),
                module_p->name ()));
  }

  return false;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::control (ControlType control_in,
                                             bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::control"));

  int result = -1;
  ISTREAM_CONTROL_T* istream_control_p = NULL;

  // *IMPORTANT NOTE*: if this stream has been linked (e.g. connection is part
  //                   of another stream), forward upstream ?
  if (upStream_ && forwardUpstream_in)
  {
    istream_control_p = dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  upStream_));
      return;
    } // end IF
    istream_control_p->control (control_in,
                                forwardUpstream_in);
    return;
  } // end IF

  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (module_p);

  switch (control_in)
  {
    case STREAM_CONTROL_LINK:
    case STREAM_CONTROL_STEP:
    case STREAM_CONTROL_UNLINK:
    {
      istream_control_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
      if (!istream_control_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                    module_p->name ()));
        return;
      } // end IF

      try {
        istream_control_p->control (control_in,
                                    false);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_IStreamControl::control(%d), returning\n"),
                    module_p->name (),
                    control_in));
        return;
      }

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown control (was: %d), returning\n"),
                  control_in));
      return;
    }
  } // end IF
}
template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::notify (NotificationType notification_in,
                                            bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::notify"));

  int result = -1;
  ISTREAM_CONTROL_T* istream_control_p = NULL;

  // *IMPORTANT NOTE*: if this stream has been linked (e.g. connection is part
  //                   of another stream), forward upstream ?
  if (upStream_ && forwardUpstream_in)
  {
    istream_control_p = dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  upStream_));
      return;
    } // end IF
    istream_control_p->notify (notification_in,
                               forwardUpstream_in);
    return;
  } // end IF

  MODULE_T* module_p = NULL;
  result = inherited::top (module_p);
  if ((result == -1) || !module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (module_p);

  istream_control_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (module_p->writer ());
  if (!istream_control_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try {
    istream_control_p->notify (notification_in,
                               false);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::notify(%d), returning\n"),
                module_p->name (),
                notification_in));
    return;
  }
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::load (Stream_ModuleList_t& modules_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::load"));

  ACE_UNUSED_ARG (modules_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::flush (bool flushInbound_in,
                                           bool flushUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::flush"));

  int result = -1;

  // *WARNING*: make sure not to flush any control/session messages !

  // *IMPORTANT NOTE*: if this stream has been linked (e.g. connection is part
  //                   of another stream), flush the whole pipeline ?
  if (upStream_ && flushUpStream_in)
  {
    ISTREAM_CONTROL_T* istream_control_p =
        dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  upStream_));
      return;
    } // end IF
    istream_control_p->flush (flushInbound_in,
                              flushUpStream_in);
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("flushed upstream \"%s\"...\n"),
    //            ACE_TEXT (istream_control_p->name ().c_str ())));
  } // end IF

  Stream_ModuleList_t modules;
  const MODULE_T* head_p = inherited::head ();
  const MODULE_T* tail_p = inherited::tail ();
  const MODULE_T* module_p = NULL;
  Stream_Task_t* task_p = NULL;
  Stream_Queue_t* queue_p = NULL;
  Stream_IMessageQueue* iqueue_p = NULL;
  //  unsigned int number_of_messages = 0;
  const MODULE_T* top_module_p =
    (upStream_ ? NULL
               : const_cast<MODULE_T*> (head_p)->next ());

  if (!flushInbound_in)
    goto continue_;

  // writer (inbound) side
  for (ITERATOR_T iterator (*this);
       (iterator.next (module_p) != 0);
       iterator.advance ())
  {
    // skip stream head/tail
    if ((module_p == head_p)       ||
        (module_p == top_module_p) || // <-- stream generator
        (module_p == tail_p))
      continue;

    modules.push_front (const_cast<MODULE_T*> (module_p));
    task_p = const_cast<MODULE_T*> (module_p)->writer ();
    if (!task_p) // close()d already ?
      continue;
    queue_p = task_p->msg_queue ();
    ACE_ASSERT (queue_p);
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (queue_p);
    if (!iqueue_p)
    {
      // *NOTE*: most probable cause: module is (upstream) head
      // *TODO*: control/session messages are flushed here, this must not
      //         happen
      result = queue_p->flush ();
    } // end IF
    else
      result = static_cast<int> (iqueue_p->flushData ());
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\":\"%s\" writer: failed to Stream_IMessageQueue::flushData(): \"%m\", continuing\n"),
                  ACE_TEXT (name ().c_str ()), module_p->name ()));
    } // end IF
    else if (result)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\":\"%s\" writer: flushed %d message(s)...\n"),
                  ACE_TEXT (name ().c_str ()), module_p->name (),
                  result));
    } // end IF

    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("\"%s\":\"%s\" writer: flushed...\n"),
    //            ACE_TEXT (name ().c_str ()), module_p->name ()));

    module_p = NULL;
  } // end FOR

continue_:
  // reader (outbound) side
  for (Stream_ModuleListIterator_t iterator = modules.begin ();
       iterator != modules.end ();
       iterator++)
  {
    task_p = (*iterator)->reader ();
    if (!task_p) // close()d already ?
      continue;
    queue_p = task_p->msg_queue ();
    ACE_ASSERT (queue_p);
    iqueue_p = dynamic_cast<Stream_IMessageQueue*> (queue_p);
    if (!iqueue_p)
    {
      // *NOTE*: most probable cause: module is (upstream) head
      // *TODO*: control/session messages are flushed here, this must not
      //         happen
      result = queue_p->flush ();
    } // end IF
    else
      result = static_cast<int> (iqueue_p->flushData ());
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\":\"%s\" reader: failed to Stream_IMessageQueue::flushData(): \"%m\", continuing\n"),
                  ACE_TEXT (name ().c_str ()), (*iterator)->name ()));
    } // end IF
    else if (result)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\":\"%s\" reader: flushed %d message(s)...\n"),
                  ACE_TEXT (name ().c_str ()), (*iterator)->name (),
                  result));
    } // end IF

    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("\"%s\":\"%s\" reader: flushed...\n"),
    //            ACE_TEXT (name ().c_str ()), (*iterator)->name ()));
  } // end FOR
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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

  try {
    control_impl_p->pause ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::pause(), returning\n"),
                module_p->name ()));
    return;
  }
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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

  try {
    control_impl_p->rewind ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::rewind(), returning\n"),
                module_p->name ()));
    return;
  }
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
StatusType
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::status () const
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
  MODULE_T* module_p = NULL;
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
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl*> failed, aborting\n"),
                module_p->name ()));
    return result;
  } // end IF

  try {
    result = control_impl_p->status ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::status(), aborting\n"),
                module_p->name ()));
    return result;
  }

  return result;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::waitForCompletion (bool waitForThreads_in,
                                                       bool waitForUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::waitForCompletion"));

  int result = -1;

  // *NOTE*: if this stream has been linked (e.g. connection is part of another
  //         stream), make sure to wait for the whole pipeline
  if (upStream_ && waitForUpStream_in)
  {
    ISTREAM_CONTROL_T* istream_control_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
    if (!istream_control_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Stream_IStreamControl_T>(0x%@), returning\n"),
                  upStream_));
      return;
    } // end IF
    istream_control_p->waitForCompletion (waitForThreads_in,
                                          waitForUpStream_in);
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("upstream \"%s\" complete...\n"),
    //            ACE_TEXT (istream_control_p->name ().c_str ())));
  } // end IF

  // *NOTE*: the logic here is this:
  //         step1: wait for (message source) processing to finish
  //         step2: wait for any upstreamed messages to 'flush' (message sink)
  Stream_ModuleList_t modules;
  modules.push_front (inherited::head ());

  // step1a: get head module, skip over ACE_Stream_Head
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

  try {
    // wait for state switch (xxx --> FINISHED) (/ any head module thread(s))
    control_impl_p->waitForCompletion (waitForThreads_in,
                                       waitForUpStream_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IStreamControl::waitForCompletion (), returning\n"),
                module_p->name ()));
    return;
  }

  // step1b: wait for (inbound) processing pipeline to flush
  Stream_Task_t* task_p = NULL;
  Stream_Queue_t* queue_p = NULL;
  ACE_Time_Value one_second (1, 0);
  size_t message_count = 0;
  for (iterator.advance ();
       (iterator.next (module_p) != 0);
       iterator.advance ())
  {
    // skip stream tail (last module)
    if (module_p == inherited::tail ())
      continue;

    modules.push_front (const_cast<MODULE_T*> (module_p));

    task_p = const_cast<MODULE_T*> (module_p)->writer ();
    if (!task_p) continue; // close()d already ?
    queue_p = task_p->msg_queue ();
    ACE_ASSERT (queue_p);
    do
    {
      //result = queue_p->wait ();
      message_count = queue_p->message_count ();
      if (!message_count) break;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s writer: waiting to process ~%d byte(s) (in %u message(s))...\n"),
      //            module_p->name (),
      //            queue_p->message_bytes (), message_count));
      result = ACE_OS::sleep (one_second);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s writer: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    module_p->name (),
                    &one_second));
    } while (true);

    if (waitForThreads_in)
    {
      result = task_p->wait ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s writer: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                    module_p->name ()));
    } // end IF

    module_p = NULL;
  } // end FOR

  // step2: wait for any upstreamed workers and messages to flush
  for (Stream_ModuleListIterator_t iterator2 = modules.begin ();
       iterator2 != modules.end ();
       iterator2++)
  {
    task_p = (*iterator2)->reader ();
    if (!task_p) continue; // close()d already ?
    queue_p = task_p->msg_queue ();
    ACE_ASSERT (queue_p);
    do
    {
      //result = queue_p->wait ();
      message_count = queue_p->message_count ();
      if (!message_count) break;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s reader: waiting to process ~%d byte(s) (in %u message(s))...\n"),
      //            (*iterator2)->name (),
      //            queue_p->message_bytes (), message_count));
      result = ACE_OS::sleep (one_second);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s reader: failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    (*iterator2)->name (),
                    &one_second));
    } while (true);

    if (waitForThreads_in)
    {
      result = task_p->wait ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s reader: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                    (*iterator2)->name ()));
    } // end IF
  } // end FOR
}

//template <typename LockType,
//          typename TaskSynchType,
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
//              TaskSynchType,
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
//  Stream_Task_t* task_p = NULL;
//  Stream_Queue_t* queue_p = NULL;
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

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Module_t*
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::find"));

  Stream_Module_t* result = NULL;
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);

  // step1: search for the module on the stream
  const ACE_TCHAR* name_p = ACE_TEXT_CHAR_TO_TCHAR (name_in.c_str ());
  result = this_p->inherited::find (name_p);
  if (!result)
  {
    // step2: search loaded modules
    for (Stream_ModuleListIterator_t iterator = modules_.begin ();
         iterator != modules_.end ();
         iterator++)
      if (ACE_TEXT_ALWAYS_CHAR ((*iterator)->name ()) == name_in)
        return *iterator;

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module (name was: \"%s\") not found, aborting\n"),
                ACE_TEXT (name_in.c_str ())));
  } // end IF

  return result;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
std::string
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::name () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::name"));

  return name_;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
const StateType&
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::state"));

  return state_;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::upStream (Stream_Base_t* upStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::upStream"));

  // sanity check(s)
  ACE_ASSERT (!upStream_);

  upStream_ = upStream_in;
}
template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Base_t*
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::upStream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::upStream"));

  return upStream_;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
const SessionDataContainerType*
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::get"));

  return sessionData_;
}
template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::set (const SessionDataContainerType* sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::set"));

  // clean up
  if (sessionData_)
    sessionData_->decrease ();

  sessionData_ = const_cast<SessionDataContainerType*> (sessionData_in);
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::initialize (const ConfigurationType& configuration_inout,
                                                bool setupPipeline_in,
                                                bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::initialize"));

  IMODULE_T* imodule_p = NULL;
  MODULE_T* module_p = NULL;
  int result = -1;

  if (isInitialized_)
  {
    if (!finalize ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Base_T::finalize(), aborting\n")));
      return false;
    } // end IF

    configuration_ = NULL;

    // *NOTE*: finalize() calls close(), resetting the task handles
    //         of all enqueued modules --> reset them manually
    for (Stream_ModuleListIterator_t iterator = modules_.begin ();
         iterator != modules_.end ();
         iterator++)
    {
      imodule_p = dynamic_cast<IMODULE_T*> (*iterator);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                    (*iterator)->name ()));
        return false;
      } // end IF
      try {
        imodule_p->reset ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_IModule::reset(), continuing\n")));
      }
    } // end FOR

    if (hasFinal_)
    {
      if (state_.module)
      {
        module_p = inherited::find (state_.module->name ());
        if (module_p)
        {
          result = inherited::remove (state_.module->name (),
                                      ACE_Module_Base::M_DELETE_NONE);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", continuing\n"),
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

      modules_.pop_front ();
      hasFinal_ = false;
    } // end IF

    if (resetSessionData_in &&
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
                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
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
                    ACE_TEXT ("\"%s\": caught exception in Stream_IModule_T::clone(), aborting\n"),
                    configuration_inout.module->name ()));
        module_p = NULL;
      }
      if (!module_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Stream_IModule_T::clone(), aborting\n"),
                    configuration_inout.module->name ()));
        return false;
      }
      state_.module = module_p;
      state_.deleteModule = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: cloned final module 0x%@ (handle is: 0x%@)\n"),
                  configuration_inout.module->name (),
                  configuration_inout.module,
                  state_.module));

      imodule_p = dynamic_cast<IMODULE_T*> (module_p);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule_T> failed, aborting\n"),
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
      // *NOTE*: if the module is ever reused, the reader/writer handles may need
      //         to be reset
      imodule_p->reset ();

      state_.module = configuration_inout.module;
      state_.deleteModule = configuration_inout.deleteModule;
    } // end ELSE
    ACE_ASSERT (state_.module);

    modules_.push_front (state_.module);
    hasFinal_ = true;

    if (!imodule_p->initialize (*configuration_inout.moduleConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize module, aborting\n"),
                  state_.module->name ()));
      return false;
    } // end IF
    Stream_Task_t* task_p = state_.module->writer ();
    ACE_ASSERT (task_p);
    MODULEHANDLER_IINITIALIZE_T* iinitialize_p =
        dynamic_cast<MODULEHANDLER_IINITIALIZE_T*> (task_p);
    if (!iinitialize_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Common_IInitialize_T<HandlerConfigurationType>> failed, aborting\n"),
                  state_.module->name ()));
      return false;
    } // end IF
    if (!iinitialize_p->initialize (*configuration_inout.moduleHandlerConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_IInitialize_T::initialize(), aborting\n"),
                  state_.module->name ()));
      return false;
    } // end IF
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_inout);
  // sanity check(s)
  ACE_ASSERT (configuration_->moduleHandlerConfiguration);
  configuration_->moduleHandlerConfiguration->stateMachineLock =
    &state_.stateMachineLock;

  initialize (setupPipeline_in,
              resetSessionData_in);

  return true;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
//              ProtocolMessageType>::get () const
              ProtocolMessageType>::get (ACE_Message_Block*& messageBlock_inout,
                                         ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::get"));

  return (upStream_ ? upStream_->get (messageBlock_inout, timeout_in)
                    : inherited::get (messageBlock_inout, timeout_in));
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
//              ProtocolMessageType>::get () const
              ProtocolMessageType>::link (Stream_Base_t& upStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::link"));

  // *WARNING*: cannot reach the base class lock --> not thread-safe !
  // *TODO*: submit change request to the ACE people

  ISTREAM_CONTROL_T* istreamcontrol_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (&upStream_in);
  std::string upstream_name_string;
  if (istreamcontrol_p)
    upstream_name_string = istreamcontrol_p->name ();

  // sanity check(s)
  ACE_Module<TaskSynchType, TimePolicyType>* upstream_tailing_module_p =
    upStream_in.head ();
  if (!upstream_tailing_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF

  // locate the module just above the upstreams' tail
  ACE_Module<TaskSynchType, TimePolicyType>* upstream_tail_module_p =
      upStream_in.tail ();
  if (!upstream_tail_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::tail(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF
  while (upstream_tailing_module_p->next () != upstream_tail_module_p)
    upstream_tailing_module_p = upstream_tailing_module_p->next ();

  //int result = inherited::link (upStream_in);
  ACE_Module<TaskSynchType, TimePolicyType>* head_module_p =
      inherited::head ();
  if (!head_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF
  ACE_Module<TaskSynchType, TimePolicyType>* heading_module_p =
      head_module_p->next ();
  if (!heading_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:%s: failed to ACE_Module::next(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ()),
                head_module_p->name ()));
    return -1;
  } // end IF
  heading_module_p->reader ()->next (upstream_tailing_module_p->reader ());
  upstream_tailing_module_p->next (heading_module_p);
  upstream_tailing_module_p->writer ()->next (heading_module_p->writer ());

  ///////////////////////////////////////

  // merge upstream session data
  OWN_TYPE_T* stream_p = dynamic_cast<OWN_TYPE_T*> (&upStream_in);
  SessionDataContainerType* session_data_container_p = NULL;
  if (!stream_p)
    goto done;
  session_data_container_p =
      const_cast<SessionDataContainerType*> (stream_p->get ());
  if (!session_data_container_p)
    goto done;
  if (sessionData_)
  {
    SessionDataType& session_data_r =
      const_cast<SessionDataType&> (session_data_container_p->get ());
    SessionDataType& session_data_2 =
      const_cast<SessionDataType&> (sessionData_->get ());
    // *NOTE*: the idea here is to 'merge' the two datasets
    session_data_r += session_data_2;

    // clean up
    sessionData_->decrease ();
  } // end IF
  session_data_container_p->increase ();
  sessionData_ = session_data_container_p;

done:
  // *NOTE*: ACE_Stream::linked_us_ is currently private
  //         --> retain another handle
  // *TODO*: modify ACE to make this a protected member
  upStream_ = &upStream_in;

  // notify pipeline modules
  control (STREAM_CONTROL_LINK,
           true);

  return 0;
}
template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
//              ProtocolMessageType>::get () const
              ProtocolMessageType>::unlink (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlink"));

  // *WARNING*: cannot reach the base class lock --> not thread-safe !
  // *TODO*: submit change request to the ACE people

  ISTREAM_CONTROL_T* istreamcontrol_p =
      dynamic_cast<ISTREAM_CONTROL_T*> (upStream_);
  std::string upstream_name_string;
  if (istreamcontrol_p)
    upstream_name_string = istreamcontrol_p->name ();

  // sanity check(s)
  if (!upStream_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no upstream, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF
  ACE_Module<TaskSynchType, TimePolicyType>* upstream_tailing_module_p =
    upStream_->head ();
  if (!upstream_tailing_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF

  // locate the module just above the upstreams' tail
  ACE_Module<TaskSynchType, TimePolicyType>* module_p = NULL;
  ACE_Module<TaskSynchType, TimePolicyType>* head_module_p = inherited::head ();
  if (!head_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::head(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return -1;
  } // end IF
  ACE_Module<TaskSynchType, TimePolicyType>* heading_module_p =
      head_module_p->next ();
  if (!heading_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s:%s: failed to ACE_Module::next(): \"%m\", aborting\n"),
                ACE_TEXT (name_.c_str ()),
                head_module_p->name ()));
    return -1;
  } // end IF
  do
  {
    module_p = upstream_tailing_module_p->next ();
    if (!module_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Module::next(): \"%m\", aborting\n"),
                  ACE_TEXT (upstream_tailing_module_p->name ())));
      return -1;
    } // end IF

    //if (module_p == head_p)
    if (ACE_OS::strcmp (module_p->name (), heading_module_p->name ()) == 0)
      break;

    upstream_tailing_module_p = module_p;
  } while (true);
  ACE_ASSERT (upstream_tailing_module_p);

  //int result = inherited::link (upStream_in);
  heading_module_p->reader ()->next (head_module_p->reader ());
  ACE_Module<TaskSynchType, TimePolicyType>* upstream_tail_module_p =
      upStream_->tail ();
  if (!upstream_tail_module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::tail(): \"%m\", aborting\n"),
                ACE_TEXT (upstream_name_string.c_str ())));
    return -1;
  } // end IF
  upstream_tailing_module_p->next (upstream_tail_module_p);
  upstream_tailing_module_p->writer ()->next (upstream_tail_module_p->writer ());

  upStream_ = NULL;

  // notify pipeline modules
  control (STREAM_CONTROL_UNLINK,
           false);

  //return inherited::unlink ();
  return 0;
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

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::remove (MODULE_T* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::remove"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (module_in);

  // *NOTE*: start with any trailing module(s) and work forwards

  // step1: find chain tail module (if any)
  MODULE_T* module_p = module_in;
  MODULE_T* tail_p = NULL;
  while (module_p->next () != NULL)
    module_p = module_p->next ();
  tail_p = module_p;
  ACE_ASSERT (tail_p &&
              (ACE_OS::strcmp (module_p->name (),
                               ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Tail")) == 0));

  // step2: remove chain (back-to-front)
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

    // *NOTE*: remove()ing a module close()s it, resetting the task handles
    //         --> call reset() so it can be reused
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

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::isInitialized () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::isInitialized"));

  return isInitialized_;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::finished (bool finishUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::finished"));

  int result = -1;

  MODULE_T* module_p = NULL;
  STATEMACHINE_ICONTROL_T* control_impl_p = NULL;

  // *NOTE*: if this stream has been linked (e.g. connection is part of another
  //         stream), make sure to finished() the whole pipeline
  if (upStream_ && finishUpStream_in)
  {
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
                  ACE_TEXT ("%s: dynamic_cast<Stream_StateMachine_IControl_T> failed, continuing\n"),
                  module_p->name ()));
      goto _continue;
    } // end IF

    try {
      control_impl_p->finished ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_StateMachine_IControl_T::finished(), continuing\n"),
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
                ACE_TEXT ("%s: dynamic_cast<Stream_StateMachine_IControl_T> failed, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  try {
    control_impl_p->finished ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_StateMachine_IControl_T::finished(), returning\n"),
                module_p->name ()));
    return;
  }
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
  //        --> close() these manually before they do so in their dtors
  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...\n")));

  //for (ACE_DLList_Iterator<MODULE_T> iterator (availableModules_);
  //     (iterator.next (module_p) != 0);
  //     iterator.advance ())
  for (Stream_ModuleListIterator_t iterator = modules_.begin ();
       iterator != modules_.end ();
       iterator++)
  {
    // sanity check: on the stream ?
    //if (module_p->next () == NULL)
    if ((*iterator)->next () == NULL)
    {
      //ACE_DEBUG ((LM_WARNING,
      //            ACE_TEXT ("closing module: \"%s\"\n"),
      //            module->name ()));

      try {
        //module_p->close (ACE_Module_Base::M_DELETE_NONE);
        result = (*iterator)->close (ACE_Module_Base::M_DELETE_NONE);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                    (*iterator)->name ()));
        result = -1;
      }
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%m\", continuing\n"),
                    (*iterator)->name ()));
    } // end IF
  } // end FOR

  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("deactivating offline module(s)...DONE\n")));

  // step2: shutdown stream
  // check the ACE documentation on ACE_Stream to see why this is needed
  // *TODO*: ONLY do this if stream_head != 0 !!! (warning: obsolete ?)
  // *NOTE*: will NOT destroy all modules in the current stream, as this leads to
  //         exceptions in debug builds under MS Windows (can't delete object
  //         in a different DLL than where it was created...)
  //         --> do this manually !
  //         all this does is call close() on each module (waits for any worker
  //         thread(s) to return)
  if (!finalize ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::finalize(): \"%m\", continuing\n")));

  // *NOTE*: every ACTIVE module will join with its worker thread in close()
  //         --> ALL stream-related threads should have returned by now
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::deactivateModules ()
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
      message_p = static_cast<SessionMessageType*> (configuration_->messageAllocator->malloc (0));
    } catch (...) {
      ACE_DEBUG ((LM_CRITICAL,
                 ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), returning\n")));

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
                ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", returning\n")));

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
                ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF
}

template <typename LockType,
          typename TaskSynchType,
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
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Base_T<LockType,
              TaskSynchType,
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
              SessionMessageType,
              ProtocolMessageType>::unlinkModules ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Base_T::unlinkModules"));

  MODULE_T* head_p = inherited::head ();
  for (MODULE_T* module_p = head_p;
       module_p;
       module_p = module_p->next ())
    module_p->next (NULL);
  head_p->next (inherited::tail ());
}
