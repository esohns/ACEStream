/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#include "ace/Guard_T.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::Stream_Session_Manager_T ()
 : resetTimeoutHandler_ (this)
 , resetTimeoutHandlerId_ (-1)
 , resetTimeoutInterval_ (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000)
 , configuration_ (NULL)
 , lock_ ()
 , sessionData_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::Stream_Session_Manager_T"));

}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::~Stream_Session_Manager_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::~Stream_Session_Manager_T"));

  // clean up
  for (typename SESSIONDATA_MAP_T::const_iterator iterator = sessionData_.begin ();
       iterator != sessionData_.end ();
       ++iterator)
    if ((*iterator).second->managed)
      delete (*iterator).second;

  if (unlikely (resetTimeoutHandlerId_ != -1))
  {
    Common_ITimer_Manager_t* timer_interface_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
    const void* act_p = NULL;
    int result = timer_interface_p->cancel_timer (resetTimeoutHandlerId_,
                                                  &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  resetTimeoutHandlerId_));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  // create default (managed) session data
  create ();

  // *TODO*: remove type inferences
  if (likely (configuration_->stream))
  { Common_ISubscribe_T<Stream_IEvent_T<NotificationType> >* isubscribe_p =
      dynamic_cast<Common_ISubscribe_T<Stream_IEvent_T<NotificationType> >*> (configuration_->stream);
    if (likely (isubscribe_p))
      isubscribe_p->subscribe (this);
    else
    { ISTREAM_T* istream_p = dynamic_cast<ISTREAM_T*> (configuration_->stream);
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s%sfailed to dynamic_cast<Common_ISubscribe_T<Stream_IEvent_T<NotificationType> >*>(%@), cannot subscribe to stream events, continuing\n"),
                  istream_p ? ACE_TEXT (istream_p->name ().c_str ()) : ACE_TEXT (""), istream_p ? ACE_TEXT (": ") : ACE_TEXT (""),
                  configuration_->stream));
    } // end ELSE
  } // end IF
  else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("no stream handle supplied, cannot subscribe to stream events, continuing\n")));
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::start"));

  int result = -1;
  Common_ITimer_Manager_t* timer_interface_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  const void* act_p = NULL;

  // (re-)schedule the visitor interval timer
  if (unlikely (resetTimeoutHandlerId_ != -1))
  {
    result = timer_interface_p->cancel_timer (resetTimeoutHandlerId_,
                                              &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  resetTimeoutHandlerId_));
    resetTimeoutHandlerId_ = -1;
  } // end IF
  resetTimeoutHandlerId_ =
    timer_interface_p->schedule_timer (&resetTimeoutHandler_,  // event handler handle
                                       NULL,                   // asynchronous completion token
                                       ACE_Time_Value::zero,   // first wakeup time
                                       resetTimeoutInterval_); // interval
  if (unlikely (resetTimeoutHandlerId_ == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_ITimer::schedule_timer(%#T): \"%m\", returning\n"),
                &resetTimeoutInterval_));
    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("scheduled visitor interval timer (id: %d, interval: %#T)\n"),
              resetTimeoutHandlerId_,
              &resetTimeoutInterval_));
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::stop (bool,
                                              bool,
                                              bool)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::stop"));

  if (unlikely (resetTimeoutHandlerId_ != -1))
  {
    Common_ITimer_Manager_t* timer_interface_p =
        COMMON_TIMERMANAGER_SINGLETON::instance ();
    const void* act_p = NULL;
    int result = timer_interface_p->cancel_timer (resetTimeoutHandlerId_,
                                                  &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  resetTimeoutHandlerId_));
    resetTimeoutHandlerId_ = -1;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::onSessionEnd (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::onSessionEnd"));

  // *NOTE*: look up and reset the session data
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    for (typename SESSIONDATA_MAP_T::iterator iterator = sessionData_.begin ();
         iterator != sessionData_.end ();
         ++iterator)
      if ((*iterator).second->sessionId == sessionId_in)
      {
        (*iterator).second->clear ();
        break; // done
      } // end IF
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
Stream_SessionId_t
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::sessionId (const std::string& streamId_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::sessionId"));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, 0);
    SESSIONDATA_MAP_CONST_ITERATOR_T iterator = sessionData_.find (streamId_in);
    if (likely (iterator != sessionData_.end ()))
      return iterator->second->sessionId;
  } // end lock scope

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("invalid stream id (was: \"%s\"), aborting\n"),
              ACE_TEXT (streamId_in.c_str ())));
  ACE_ASSERT (false);

  return 0;
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
const SessionDataType&
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::getR (const std::string& streamId_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::getR"));

  static SessionDataType dummy;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, dummy);
    SESSIONDATA_MAP_CONST_ITERATOR_T iterator = sessionData_.find (streamId_in);
    if (likely (iterator != sessionData_.end ()))
      return *(iterator->second);
  } // end lock scope

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("invalid stream id (was: \"%s\"), aborting\n"),
              ACE_TEXT (streamId_in.c_str ())));
  ACE_ASSERT (false);

  return dummy;
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::create (const std::string& streamId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::create"));

  SessionDataType* session_data_p = NULL;
  ACE_NEW_NORETURN (session_data_p,
                    SessionDataType ());
  ACE_ASSERT (session_data_p);
  session_data_p->managed = true;

  setR (*session_data_p, streamId_in);
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::destroy (const std::string& streamId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::destroy"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    SESSIONDATA_MAP_ITERATOR_T iterator = sessionData_.find (streamId_in);
    if (iterator != sessionData_.end ())
    {
      if (unlikely ((*iterator).second->managed))
        delete (*iterator).second;

      sessionData_.erase (iterator);
    } // end IF
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid stream id (was: \"%s\"), cannot destroy session data, continuing\n"),
                  ACE_TEXT (streamId_in.c_str ())));
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::setR (SessionDataType& sessionData_in,
                                              const std::string& streamId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::setR"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    // *TODO*: remove type inferences
    if (likely (!sessionData_in.lock))
      sessionData_in.lock = &lock_;

    // sanity check(s)
    SESSIONDATA_MAP_ITERATOR_T iterator = sessionData_.find (streamId_in);
    if (iterator != sessionData_.end ())
    {
      if (unlikely ((*iterator).second->managed))
        delete (*iterator).second;

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("resetting session data (stream id: \"%s\"), continuing\n"),
                  ACE_TEXT (streamId_in.c_str ())));

      // update map entry
      (*iterator).second = &sessionData_in;

      return;
    } // end IF

    // create map entry
    sessionData_.insert (std::make_pair (streamId_in, &sessionData_in));
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::onEvent (const std::string& streamId_in,
                                                 NotificationType notification_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::onEvent"));

  Stream_SessionId_t session_id_i = sessionId (streamId_in);

  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    case STREAM_SESSION_MESSAGE_CONNECT:
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    case STREAM_SESSION_MESSAGE_LINK:
    case STREAM_SESSION_MESSAGE_RESIZE:
    case STREAM_SESSION_MESSAGE_UNLINK:
      break;
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      try {
        onSessionBegin (session_id_i);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_ISessionCB::onSessionBegin(), continuing\n")));
      }

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      try {
        onSessionEnd (session_id_i);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_ISessionCB::onSessionEnd(), continuing\n")));
      }

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    case STREAM_SESSION_MESSAGE_STATISTIC:
      break;
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename NotificationType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         NotificationType,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::reset"));

}
