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

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_net_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
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
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::Stream_Module_Net_IO_Stream_T ()
 : inherited ()
 , handle_ (ACE_INVALID_HANDLE)
 , name_ (ACE_TEXT_ALWAYS_CHAR (StreamName))
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::Stream_Module_Net_IO_Stream_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
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
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::load (Stream_ILayout* layout_in,
                                                   bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::load"));

  // sanity check(s)
  ACE_ASSERT (layout_in);

  typename inherited::MODULE_T* module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  IO_MODULE_T (this,
                               ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
                  false);
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return false;
  } // end IF
  layout_in->append (module_p, NULL, 0);

  delete_out = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
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
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              UserDataType>::initialize (const CONFIGURATION_T& configuration_in,
#else
                              UserDataType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
#endif
                                                         ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);

  if (inherited::isInitialized_)
  {
    handle_ = ACE_INVALID_HANDLE;
//    name_ = StreamName;
  } // end IF

  handle_ = handle_in;

  // update module handle configuration(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  for (typename CONFIGURATION_T::ITERATOR_T iterator = const_cast<CONFIGURATION_T&> (configuration_in).begin ();
       iterator != const_cast<CONFIGURATION_T&> (configuration_in).end ();
#else
  for (typename inherited::CONFIGURATION_T::ITERATOR_T iterator = const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).begin ();
       iterator != const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).end ();
#endif
       ++iterator)
  { //ACE_ASSERT ((*iterator).second.second.finishOnDisconnect);
    //ACE_ASSERT ((*iterator).second.second.socketHandle == ACE_INVALID_HANDLE);
    (*iterator).second.second.socketHandle = handle_in;
  } // end FOR

  return initialize (configuration_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
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
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              UserDataType>::initialize (const CONFIGURATION_T& configuration_in)
#else
                              UserDataType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
//  HandlerConfigurationType* configuration_p = NULL;
//  bool reset_configuration = false;
  typename inherited::MODULE_T* module_p = NULL;
  WRITER_T* IOWriter_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      false;
  reset_setup_pipeline = true;
  if (unlikely (!inherited::initialize (configuration_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (name_.c_str ())));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  if (configuration_in.configuration_->resetSessionData)
  { ACE_ASSERT (inherited::sessionData_);
//    SessionDataType* session_data_p =
//        &const_cast<SessionDataType&> (inherited::sessionData_->get ());
    // *TODO*: remove type inferences
    //session_data_p->sessionId = configuration_in.configuration->sessionId;
    //inherited::sessionData_->state =
    //  &const_cast<StateType&> (inherited::state ());
  } // end IF

//  // *IMPORTANT NOTE*: a connection data processing stream may either be
//  //                   appended ('outbound' scenario) or prepended ('inbound'
//  //                   (e.g. listener-based) scenario) to another stream. In the
//  //                   first case, the net io (head) module behaves in a
//  //                   somewhat particular manner, as it may be neither 'active'
//  //                   (run a dedicated thread) nor 'passive' (borrow calling
//  //                   thread in start()). Instead, it can behave as a regular
//  //                   synchronous (i.e. passive) module; this reduces the
//  //                   thread-count and generally improves efficiency
//  // *TODO*: remove type inferences
//  iterator =
//      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING));
//  ACE_ASSERT (iterator != configuration_in.end ());
//  configuration_p =
//      dynamic_cast<HandlerConfigurationType*> (&(*iterator).second.second);
//  // sanity check(s)
//  ACE_ASSERT (configuration_p);
//  enum Stream_HeadModuleConcurrency concurrency_mode_e =
//      configuration_p->concurrency;
//  configuration_p->concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;
//  reset_configuration = true;

  // ---------------------------------------------------------------------------
  // sanity check(s)
//  ACE_ASSERT (configuration_in.moduleConfiguration);

  // ******************* IO ************************
  module_p =
    const_cast<typename inherited::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING),
                                               true,    // sanitize module names ?
                                               false)); // recurse upstream ?
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (name_.c_str ()),
                ACE_TEXT (MODULE_NET_IO_DEFAULT_NAME_STRING)));
    goto error;
  } // end IF
  IOWriter_impl_p = dynamic_cast<WRITER_T*> (module_p->writer ());
  if (unlikely (!IOWriter_impl_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s writer: dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n"),
                ACE_TEXT (name_.c_str ()),
                module_p->name ()));
    goto error;
  } // end IF
  IOWriter_impl_p->setP (&(inherited::state_));
  IOWriter_impl_p->initialize ();
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

  if (configuration_in.configuration_->setupPipeline)
    if (unlikely (!inherited::setup (configuration_in.configuration_->notificationStrategy)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (name_.c_str ())));
      goto error;
    } // end IF

  // -------------------------------------

  inherited::isInitialized_ = true;

  result = true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
//  if (reset_configuration)
//  { ACE_ASSERT (configuration_p);
//    configuration_p->concurrency = concurrency_mode_e;
//  } // end IF

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::stop (bool wait_in,
                                                   bool recurseUpstream_in,
                                                   bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::stop"));

  // *IMPORTANT NOTE*: downstream modules may hold 'volatile' references to the
  //                   connection: when the stream is finished(), the
  //                   connection may get deleted. 'Inbound' (!) connections
  //                   could thus invalidate the i/o modules' callstack state,
  //                   corrupting the heap
  //                   --> maintain a temporary reference to prevent this
  typename ConnectionManagerType::ICONNECTION_T* connection_p = NULL;
  if (likely (handle_ != ACE_INVALID_HANDLE))
  {
    ConnectionManagerType* connection_manager_p =
      ConnectionManagerType::SINGLETON_T::instance ();
    ACE_ASSERT (connection_manager_p);
    connection_p = connection_manager_p->get (handle_);
    ACE_ASSERT (connection_p);
  } // end IF

  inherited::stop (wait_in,
                   recurseUpstream_in,
                   lockedAccess_in);

  // clean up
  if (likely (connection_p))
    connection_p->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::finished (bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::finished"));

  // *IMPORTANT NOTE*: downstream modules may hold 'volatile' references to the
  //                   connection: when the stream is finished(), the
  //                   connection may get deleted. 'Inbound' (!) connections
  //                   could thus invalidate the i/o modules' callstack state,
  //                   corrupting the heap
  //                   --> maintain a temporary reference to prevent this
  typename ConnectionManagerType::ICONNECTION_T* connection_p = NULL;
  ConnectionManagerType* connection_manager_p =
    ConnectionManagerType::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (connection_manager_p);

  if (likely (handle_ != ACE_INVALID_HANDLE))
    connection_p = connection_manager_p->get (handle_);

  inherited::finished (recurseUpstream_in);

  // clean up
  if (likely (connection_p))
    connection_p->decrease ();
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          const char* StreamName,
//          typename ControlType,
//          typename NotificationType,
//          typename StatusType,
//          typename StateType,
//          typename ConfigurationType,
//          typename StatisticContainerType,
//          typename StatisticHandlerType,
//          typename HandlerConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename AddressType,
//          typename ConnectionManagerType,
//          typename UserDataType>
//bool
//Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
//                              TimePolicyType,
//                              StreamName,
//                              ControlType,
//                              NotificationType,
//                              StatusType,
//                              StateType,
//                              ConfigurationType,
//                              StatisticContainerType,
//                              StatisticHandlerType,
//                              HandlerConfigurationType,
//                              SessionDataType,
//                              SessionDataContainerType,
//                              ControlMessageType,
//                              DataMessageType,
//                              SessionMessageType,
//                              AddressType,
//                              ConnectionManagerType,
//                              UserDataType>::collect (StatisticContainerType& data_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::collect"));
//
//  // sanity check(s)
//  ACE_UNUSED_ARG (data_out); // *TODO*
//  ACE_ASSERT (inherited::sessionData_);
//
//  int result = -1;
//  SessionDataType& session_data_r =
//      const_cast<SessionDataType&> (inherited::sessionData_->getR ());
//
//  // synch access
//  if (session_data_r.lock)
//  {
//    result = session_data_r.lock->acquire ();
//    if (unlikely (result == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
//                  ACE_TEXT (name_.c_str ())));
//      return false;
//    } // end IF
//  } // end IF
//
//  session_data_r.statistic.timeStamp = COMMON_TIME_NOW;
//
//  if (session_data_r.lock)
//  {
//    result = session_data_r.lock->release ();
//    if (unlikely (result == -1))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
//                  ACE_TEXT (name_.c_str ())));
//  } // end IF
//
//  return true;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
const ACE_Notification_Strategy* const
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::getP (bool recurseUpstream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::getP"));

  const typename inherited::MODULE_T* module_p = NULL;
  typename inherited::TASK_T* task_p = NULL;
  std::string module_name_string = ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head");

  if (recurseUpstream_in)
  {
    typename inherited::ISTREAM_T::STREAM_T* stream_p =
        inherited::upstream (recurseUpstream_in);
    if (stream_p)
    {
      Stream_IOutboundDataNotify* inotify =
          dynamic_cast<Stream_IOutboundDataNotify*> (stream_p);
      if (!inotify)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: dynamic_cast<Stream_IOutboundDataNotify>(0x%@) failed, continuing\n"),
                    ACE_TEXT (name_.c_str ()),
                    stream_p));
      else
        return inotify->getP (false);
    } // end IF
  } // end IF

  module_p = const_cast<OWN_TYPE_T*> (this)->inherited::head ();
  ACE_ASSERT (module_p);
  task_p =
      const_cast<typename inherited::ISTREAM_T::MODULE_T*> (module_p)->reader ();
  ACE_ASSERT (task_p);
  ACE_ASSERT (task_p->module ());
retry:
  while (ACE_OS::strcmp (task_p->module ()->name (),
                         ACE_TEXT (module_name_string.c_str ())))
  {
    task_p = task_p->next ();
    if (!task_p)
      break;
  } // end WHILE
  if (unlikely (!task_p))
  {
    if (!ACE_OS::strcmp (module_name_string.c_str (),
                         ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head")))
    {
      module_name_string = ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_HEAD_NAME);
      task_p =
          const_cast<typename inherited::MODULE_T*> (module_p)->reader ();
      ACE_ASSERT (task_p);
      ACE_ASSERT (task_p->module ());
      goto retry;
    } // end IF

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no head module reader task found, aborting\n"),
                ACE_TEXT (name_.c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (task_p->msg_queue_);
  
  return task_p->msg_queue_->notification_strategy ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
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
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::initialize_2 (ACE_Notification_Strategy* strategyHandle_in,
                                                           const std::string& moduleName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::initialize_2"));

  // sanity check(s)
  ACE_ASSERT (strategyHandle_in);

  bool is_head_b =
      (moduleName_in.empty () ||
       (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"),
                         moduleName_in.c_str ()) ||
        !ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_HEAD_NAME),
                         moduleName_in.c_str ())));
  std::string module_name_string =
      (moduleName_in.empty () ? ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head")
                              : moduleName_in);
  const typename inherited::MODULE_T* module_p = NULL;
  typename inherited::TASK_T* task_p = NULL;

  // sanity check(s)
  ACE_ASSERT (!module_name_string.empty ());

  module_p = inherited::tail ();
  ACE_ASSERT (module_p);
  task_p =
      const_cast<typename inherited::MODULE_T*> (module_p)->reader ();
  ACE_ASSERT (task_p);
  ACE_ASSERT (task_p->module ());
retry:
  while (ACE_OS::strcmp (task_p->module ()->name (),
                         ACE_TEXT (module_name_string.c_str ())))
  {
    task_p = task_p->next ();
    if (!task_p)
      break;
  } // end WHILE
  if (unlikely (!task_p))
  {
    if (is_head_b)
    {
      module_name_string = ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_HEAD_NAME);
      task_p =
          const_cast<typename inherited::MODULE_T*> (module_p)->reader ();
      ACE_ASSERT (task_p);
      ACE_ASSERT (task_p->module ());
      goto retry;
    } // end IF

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no module (name was: \"%s\") reader task found, aborting\n"),
                ACE_TEXT (name_.c_str ()),
                ACE_TEXT (moduleName_in.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (task_p->msg_queue_);
  task_p->msg_queue_->notification_strategy (strategyHandle_in);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              StreamName,
                              ControlType,
                              NotificationType,
                              StatusType,
                              StateType,
                              ConfigurationType,
                              StatisticContainerType,
                              StatisticHandlerType,
                              HandlerConfigurationType,
                              SessionDataType,
                              SessionDataContainerType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              AddressType,
                              ConnectionManagerType,
                              UserDataType>::onLink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IO_Stream_T::onLink"));

  // inbound/outbound ?
  if (inherited::upstream ())
  { // --> outbound
    ACE_Notification_Strategy* notification_strategy_p =
      const_cast<ACE_Notification_Strategy*> (getP (false));
    if (likely (notification_strategy_p))
    {
      if (unlikely (!initialize_2 (notification_strategy_p,
                                   ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"))))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_IOutboundDataNotify::initialize_2(%@), returning\n"),
                    ACE_TEXT (name_.c_str ()),
                    notification_strategy_p));
        return;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: reset upstream notification\n"),
                  ACE_TEXT (name_.c_str ())));
    } // end IF
  } // end IF
  else
  { // --> inbound
    // make sure 'this' auto-finished()-es on disconnect
    if (!inherited::finishOnDisconnect_)
    {
      inherited::finishOnDisconnect_ = true;
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: reset finish-on-disconnect\n"),
                  ACE_TEXT (name_.c_str ())));
    } // end ELSE

    // initiate read ?
    // sanity check(s)
    ACE_ASSERT (handle_ != ACE_INVALID_HANDLE);
    ConnectionManagerType* connection_manager_p =
      ConnectionManagerType::SINGLETON_T::instance ();
    ACE_ASSERT (connection_manager_p);
    typename ConnectionManagerType::ICONNECTION_T* connection_p =
      connection_manager_p->get (handle_);
    ACE_ASSERT (connection_p);
    //const typename ConnectionManagerType::CONFIGURATION_T& configuration_r =
      //connection_p->getR (); // *TODO*: cannot get configuration from
                               //         Net_IConnection_T anymore ! :-(
    // *WORKAROUND*: there may be a race condition here in some scenarios !
    typename ConnectionManagerType::CONFIGURATION_T* configuration_p = NULL;
    typename ConnectionManagerType::USERDATA_T* user_data_p = NULL;
    connection_manager_p->get (configuration_p, user_data_p);
    ACE_ASSERT (configuration_p);
    //if (unlikely (configuration_r.delayRead))
    if (unlikely (configuration_p->delayRead))
      if (!connection_p->initiate_read ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Net_ISocketHandler::initiate_read(), aborting\n"),
                    ACE_TEXT (name_.c_str ())));
        connection_p->close ();
      } // end IF
    connection_p->decrease ();
  } // end IF
}
