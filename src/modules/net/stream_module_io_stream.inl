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

#include <ace/Log_Msg.h>

#include "stream_macros.h"

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
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
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
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::Stream_Module_Net_IO_Stream_T (const std::string& name_in)
 : inherited (name_in)
// , IO_ (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
//        NULL,
//        false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::Stream_Module_Net_IO_Stream_T"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that all modules which have NOT enqueued onto the
  //         stream (e.g. because initialize() failed...) need to be explicitly
  //         close()d
//  inherited::modules_.push_front (&IO_);

  // *TODO* fix ACE bug: modules should initialize their "next" member to NULL
  //inherited::MODULE_T* module_p = NULL;
  //for (ACE_DLList_Iterator<inherited::MODULE_T> iterator (inherited::availableModules_);
  //     iterator.next (module_p);
  //     iterator.advance ())
  //  module_p->next (NULL);
//  for (Stream_ModuleListIterator_t iterator = inherited::modules_.begin ();
//       iterator != inherited::modules_.end ();
//       iterator++)
//     (*iterator)->next (NULL);
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
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
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
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::load (Stream_ModuleList_t& modules_out,
                                                   bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  IO_MODULE_T (ACE_TEXT_ALWAYS_CHAR ("NetIO"),
                               NULL,
                               false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

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
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
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
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                         bool setupPipeline_in,
                                                         bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::initialize"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

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

  // *IMPORTANT NOTE*: a connection data processing stream may either be
  //                   appended ('outbound' scenario) or prepended ('inbound'
  //                   (e.g. listener-based) scenario) to another stream. In the
  //                   first case, the net io (head) module behaves in a
  //                   somewhat particular manner, as it may be neither 'active'
  //                   (run a dedicated thread) nor 'passive' (borrow calling
  //                   thread in start()). Instead, it can behave as a regular
  //                   synchronous (i.e. passive) module; this reduces the
  //                   thread-count and generally improves efficiency
  // *TODO*: remove type inferences
  Stream_ModuleHandlerConfigurationsIterator_t iterator =
      const_cast<ConfigurationType&> (configuration_in).moduleHandlerConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.moduleHandlerConfigurations.end ());
  HandlerConfigurationType* configuration_p =
      dynamic_cast<HandlerConfigurationType*> ((*iterator).second);
  // sanity check(s)
  ACE_ASSERT (configuration_p);
  bool reset_configuration = false;
  enum Stream_HeadModuleConcurrency concurrency_mode;
  bool is_concurrent;
  if (!configuration_p->inbound)
  {
    concurrency_mode = configuration_p->concurrency;
    is_concurrent = configuration_p->concurrent;

    configuration_p->concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;
    configuration_p->concurrent = true;

    reset_configuration = true;
  } // end IF

  // ---------------------------------------------------------------------------
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleConfiguration);

  // ******************* IO ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("NetIO")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("NetIO")));
    return false;
  } // end IF
//  IO_.initialize (*configuration_in.moduleConfiguration);
//  READER_T* IOReader_impl_p = NULL;
  WRITER_T* IOWriter_impl_p = dynamic_cast<WRITER_T*> (module_p->writer ());
  if (!IOWriter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n")));
    goto error;
  } // end IF
  IOWriter_impl_p->set (&(inherited::state_));
//  IOReader_impl_p = dynamic_cast<READER_T*> (module_p->reader ());
//  if (!IOReader_impl_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<Stream_Module_Net_IOReader_T> failed, aborting\n")));
//    goto error;
//  } // end IF
//  if (!IOReader_impl_p->initialize (inherited::state_))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize Stream_Module_Net_IOReader_T, aborting\n"),
//                module_p->name ()));
//    goto error;
//  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup (configuration_in.notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  // OK: all went well
  inherited::isInitialized_ = true;

  result = true;

error:
  if (reset_configuration)
  { ACE_ASSERT (configuration_p);
    configuration_p->concurrency = concurrency_mode;
    configuration_p->concurrent = is_concurrent;
  } // end IF

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
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
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
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::collect (StatisticContainerType& data_out)
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
