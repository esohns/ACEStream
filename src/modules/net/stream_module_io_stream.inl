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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName, // *TODO*: use a variadic character array
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
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
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              AllocatorConfigurationType,
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

  typename inherited::MODULE_T* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  IO_MODULE_T (this,
                               ACE_TEXT_ALWAYS_CHAR ("NetIO"),
                               NULL,
                               false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName, // *TODO*: use a variadic character array
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
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
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              AllocatorConfigurationType,
                              ModuleConfigurationType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
  HandlerConfigurationType* configuration_p = NULL;
  bool reset_configuration = false;
  MODULE_T* module_p = NULL;
  WRITER_T* IOWriter_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  if (configuration_in.configuration_.resetSessionData)
  { ACE_ASSERT (inherited::sessionData_);
    SessionDataType* session_data_p =
        &const_cast<SessionDataType&> (inherited::sessionData_->get ());
    // *TODO*: remove type inferences
    session_data_p->sessionID = configuration_in.configuration_.sessionID;
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
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  configuration_p =
      dynamic_cast<HandlerConfigurationType*> (&(*iterator).second);
  // sanity check(s)
  ACE_ASSERT (configuration_p);
  enum Stream_HeadModuleConcurrency concurrency_mode;
  bool is_concurrent;
  if (configuration_p->inbound)
    inherited::finishOnDisconnect_ = true;
  else
  {
    concurrency_mode = configuration_p->concurrency;
    is_concurrent = configuration_p->concurrent;

    configuration_p->concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;
    configuration_p->concurrent = true;

    reset_configuration = true;
  } // end IF

  // ---------------------------------------------------------------------------
  // sanity check(s)
//  ACE_ASSERT (configuration_in.moduleConfiguration);

  // ******************* IO ************************
  module_p =
    const_cast<MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("NetIO")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve module handle (name was: \"%s\"), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT ("NetIO")));
    goto error;
  } // end IF
//  IO_.initialize (*configuration_in.moduleConfiguration);
//  READER_T* IOReader_impl_p = NULL;
  IOWriter_impl_p = dynamic_cast<WRITER_T*> (module_p->writer ());
  if (!IOWriter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s writer: dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT ("NetIO")));
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

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (configuration_in.configuration_.notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  result = true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;
  if (reset_configuration)
  { ACE_ASSERT (configuration_p);
    configuration_p->concurrency = concurrency_mode;
    configuration_p->concurrent = is_concurrent;
  } // end IF

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName, // *TODO*: use a variadic character array
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename AllocatorConfigurationType,
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
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              AllocatorConfigurationType,
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
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      return false;
    } // end IF
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
  } // end IF

  return true;
}
