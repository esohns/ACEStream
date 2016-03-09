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

#include "stream_macros.h"

template <typename LockType,
          typename TaskSynchType,
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
          typename ProtocolMessageType,
          typename AddressType,
          typename ConnectionManagerType>
Stream_Module_Net_IO_Stream_T<LockType,
                              TaskSynchType,
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
                              ProtocolMessageType,
                              AddressType,
                              ConnectionManagerType>::Stream_Module_Net_IO_Stream_T (const std::string& name_in)
 : inherited (name_in)
 , IO_ (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
        NULL,
        false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::Stream_Module_Net_IO_Stream_T"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that all modules which have NOT enqueued onto the
  //         stream (e.g. because initialize() failed...) need to be explicitly
  //         close()d
  inherited::modules_.push_front (&IO_);

  // *TODO* fix ACE bug: modules should initialize their "next" member to NULL
  //inherited::MODULE_T* module_p = NULL;
  //for (ACE_DLList_Iterator<inherited::MODULE_T> iterator (inherited::availableModules_);
  //     iterator.next (module_p);
  //     iterator.advance ())
  //  module_p->next (NULL);
  for (typename inherited::MODULE_CONTAINER_ITERATOR_T iterator = inherited::modules_.begin ();
       iterator != inherited::modules_.end ();
       iterator++)
     (*iterator)->next (NULL);
}

template <typename LockType,
          typename TaskSynchType,
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
          typename ProtocolMessageType,
          typename AddressType,
          typename ConnectionManagerType>
Stream_Module_Net_IO_Stream_T<LockType,
                              TaskSynchType,
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
                              ProtocolMessageType,
                              AddressType,
                              ConnectionManagerType>::~Stream_Module_Net_IO_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::~Stream_Module_Net_IO_Stream_T"));

  inherited::shutdown ();
}

template <typename LockType,
          typename TaskSynchType,
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
          typename ProtocolMessageType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IO_Stream_T<LockType,
                              TaskSynchType,
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
                              ProtocolMessageType,
                              AddressType,
                              ConnectionManagerType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename LockType,
          typename TaskSynchType,
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
          typename ProtocolMessageType,
          typename AddressType,
          typename ConnectionManagerType>
bool
Stream_Module_Net_IO_Stream_T<LockType,
                              TaskSynchType,
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
                              ProtocolMessageType,
                              AddressType,
                              ConnectionManagerType>::initialize (const ConfigurationType& configuration_in,
                                                                  bool setupPipeline_in,
                                                                  bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

  if (inherited::isInitialized_)
  {
    // *TODO*: move this to stream_base.inl ?
    int result = -1;
    const typename inherited::MODULE_T* module_p = NULL;
    typename inherited::IMODULE_T* imodule_p = NULL;
    for (typename inherited::ITERATOR_T iterator (*this);
         (iterator.next (module_p) != 0);
         iterator.advance ())
    {
      if ((module_p == inherited::head ()) ||
          (module_p == inherited::tail ()))
        continue;

      // need a downcast...
      imodule_p =
        dynamic_cast<typename inherited::IMODULE_T*> (const_cast<typename inherited::MODULE_T*> (module_p));
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, aborting\n"),
                    module_p->name ()));
        return false;
      } // end IF
      if (imodule_p->isFinal ())
      {
        //ACE_ASSERT (module_p == configuration_in.module);
        result = inherited::remove (module_p->name (),
                                    ACE_Module_Base::M_DELETE_NONE);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Stream::remove(\"%s\"): \"%m\", aborting\n"),
                      module_p->name ()));
          return false;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("removed final module (was: \"%s\")...\n"),
                    module_p->name ()));
        imodule_p->reset ();

        break; // done
      } // end IF
    } // end FOR

    if (resetSessionData_in &&
        inherited::sessionData_)
    {
      inherited::sessionData_->decrease ();
      inherited::sessionData_ = NULL;
    } // end IF
  } // end IF

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in,
                              false,
                              resetSessionData_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name ().c_str ())));
    return false;
  } // end IF
  if (resetSessionData_in)
  {
    ACE_ASSERT (inherited::sessionData_);
    SessionDataType* session_data_p =
        &const_cast<SessionDataType&> (inherited::sessionData_->get ());
    // *TODO*: remove type inferences
    session_data_p->sessionID = configuration_in.sessionID;
    //inherited::sessionData_->state =
    //  &const_cast<StateType&> (inherited::state ());
  } // end IF

  // things to be done here:
  // [- initialize base class]
  // ------------------------------------
  // - initialize notification strategy (if any)
  // ------------------------------------
  // - push the final module onto the stream (if any)
  // ------------------------------------
  // - initialize modules
  // - push them onto the stream (tail-first) !
  // ------------------------------------

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  // ******************* IO ************************
  IO_.initialize (*configuration_in.moduleConfiguration);
  WRITER_T* IOWriter_impl_p = dynamic_cast<WRITER_T*> (IO_.writer ());
  if (!IOWriter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n")));
    return false;
  } // end IF
  if (!IOWriter_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOWriter_T, aborting\n"),
                IO_.name ()));
    return false;
  } // end IF
  if (!IOWriter_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOWriter_T, aborting\n"),
                IO_.name ()));
    return false;
  } // end IF
//  IOWriter_impl_p->reset ();
  READER_T* IOReader_impl_p = dynamic_cast<READER_T*> (IO_.reader ());
  if (!IOReader_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOReader_T> failed, aborting\n")));
    return false;
  } // end IF
  if (!IOReader_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOReader_T, aborting\n"),
                IO_.name ()));
    return false;
  } // end IF
  //if (!IOReader_impl_p->initialize (inherited::state_))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOReader_T, aborting\n"),
  //              IO_.name ()));
  //  return false;
  //} // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  IO_.arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup (configuration_in.notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  inherited::allocator_ = configuration_in.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename ProtocolMessageType,
          typename AddressType,
          typename ConnectionManagerType>
bool
Stream_Module_Net_IO_Stream_T<LockType,
                              TaskSynchType,
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
                              ProtocolMessageType,
                              AddressType,
                              ConnectionManagerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::collect"));

  // sanity check(s)
  ACE_UNUSED_ARG (data_out); // *TODO*
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());

  // synch access
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return true;
}

template <typename LockType,
          typename TaskSynchType,
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
          typename ProtocolMessageType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IO_Stream_T<LockType,
                              TaskSynchType,
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
                              ProtocolMessageType,
                              AddressType,
                              ConnectionManagerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
