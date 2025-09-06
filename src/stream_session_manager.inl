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
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::Stream_Session_Manager_T ()
 : resetTimeoutHandler_ (this)
 , resetTimeoutHandlerId_ (-1)
 , resetTimeoutInterval_ (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000)
 , configuration_ (NULL)
 , isInitialized_ (false)
 , lock_ ()
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::Stream_Session_Manager_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::~Stream_Session_Manager_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::~Stream_Session_Manager_T"));

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
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
bool
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  return true;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
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
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::stop (bool waitForCompletion_in,
                                              bool recurseUpstream_in,
                                              bool highPriority_in)
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

  if (waitForCompletion_in)
    wait (true); // N/A
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
unsigned int
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::flush (bool flushInbound_in,
                                               bool flushSessionMessages_in,
                                               bool flushUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::flush"));

  return 0;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::idle (bool waitForCompletion_in,
                                              bool recurseUpstream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::idle"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::wait (bool waitForThreads_in,
                                              bool waitForUpstream_in,
                                              bool waitForDownstream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::wait"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::set (const SessionDataType& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::set"));

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

  isInitialized_ = true;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::get (SessionDataType*& sessionData_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::get"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  sessionData_out = sessionData_;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename UserDataType>
void
Stream_Session_Manager_T<ACE_SYNCH_USE,
                         ConfigurationType,
                         SessionDataType,
                         StatisticContainerType,
                         UserDataType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Session_Manager_T::reset"));

}
