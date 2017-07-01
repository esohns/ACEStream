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

template <typename StatisticContainerType>
Stream_StatisticHandler_Reactor_T<StatisticContainerType>::Stream_StatisticHandler_Reactor_T (Stream_StatisticActionType action_in,
                                                                                              Common_IStatistic_T<StatisticContainerType>* interfaceHandle_in,
                                                                                              bool reportOnCollect_in)
 : inherited (NULL,                           // use default reactor
              ACE_Event_Handler::LO_PRIORITY) // priority
 , action_ (action_in)
 , interfaceHandle_ (interfaceHandle_in)
 , reportOnCollect_ (reportOnCollect_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Reactor_T::Stream_StatisticHandler_Reactor_T"));

}

template <typename StatisticContainerType>
Stream_StatisticHandler_Reactor_T<StatisticContainerType>::~Stream_StatisticHandler_Reactor_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Reactor_T::~Stream_StatisticHandler_Reactor_T"));

}

template <typename StatisticContainerType>
int
Stream_StatisticHandler_Reactor_T<StatisticContainerType>::handle_timeout (const ACE_Time_Value& tv_in,
                                                                           const void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Reactor_T::handle_timeout"));

  ACE_UNUSED_ARG (tv_in);
  ACE_UNUSED_ARG (arg_in);

  switch (action_)
  {
    case ACTION_COLLECT:
    {
      StatisticContainerType result;
      ACE_OS::memset (&result, 0, sizeof (StatisticContainerType));

      try
      {
        if (!interfaceHandle_->collect (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_IStatistic::collect(), continuing\n")));
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught an exception in Common_IStatistic::collect(), continuing\n")));
      }

      if (!reportOnCollect_)
        break;
      // *WARNING*: falls through !
    }
    case ACTION_REPORT:
    {
      try
      {
        interfaceHandle_->report ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught an exception in Common_IStatistic::report(), continuing\n")));
      }

      break;
    }
    case ACTION_INVALID:
    case ACTION_MAX:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid action %u, aborting\n"),
                  action_));
      return -1;
    }
  } // end SWITCH

  // reschedule timer...
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

template <typename StatisticContainerType>
Stream_StatisticHandler_Proactor_T<StatisticContainerType>::Stream_StatisticHandler_Proactor_T (Stream_StatisticActionType action_in,
                                                                                                Common_IStatistic_T<StatisticContainerType>* interfaceHandle_in,
                                                                                                bool reportOnCollect_in)
 : inherited ()
 , action_ (action_in)
 , interfaceHandle_ (interfaceHandle_in)
 , reportOnCollect_ (reportOnCollect_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Proactor_T::Stream_StatisticHandler_Proactor_T"));

}

template <typename StatisticContainerType>
Stream_StatisticHandler_Proactor_T<StatisticContainerType>::~Stream_StatisticHandler_Proactor_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Proactor_T::~Stream_StatisticHandler_Proactor_T"));

}

template <typename StatisticContainerType>
void
Stream_StatisticHandler_Proactor_T<StatisticContainerType>::handle_time_out (const ACE_Time_Value& tv_in,
                                                                             const void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Proactor_T::handle_time_out"));

  ACE_UNUSED_ARG (tv_in);
  ACE_UNUSED_ARG (arg_in);

  switch (action_)
  {
    case ACTION_COLLECT:
    {
      StatisticContainerType result;
      ACE_OS::memset (&result, 0, sizeof (StatisticContainerType));

      try
      {
        if (!interfaceHandle_->collect (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_IStatistic::collect(), continuing\n")));
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught an exception in Common_IStatistic::collect(), continuing\n")));
      }

      if (!reportOnCollect_)
        break;
      // *WARNING*: falls through !
    }
    case ACTION_REPORT:
    {
      try
      {
        interfaceHandle_->report ();
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught an exception in Common_IStatistic::report(), continuing\n")));
      }

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid action %u, continuing\n"),
                  action_));
      break;
    }
  } // end SWITCH
}
