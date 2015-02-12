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

#include "ace/Reactor.h"
#include "ace/Proactor.h"
#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename StatisticsContainerType>
Stream_StatisticHandler_Reactor_T<StatisticsContainerType>::Stream_StatisticHandler_Reactor_T (const Stream_IStatistic_t* interfaceHandle_in,
                                                                                               const ActionSpecifier_t& action_in)
 : inherited (NULL,                           // use default reactor
              ACE_Event_Handler::LO_PRIORITY) // priority
 , interfaceHandle_ (interfaceHandle_in)
 , action_ (action_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Reactor_T::Stream_StatisticHandler_Reactor_T"));

}

template <typename StatisticsContainerType>
Stream_StatisticHandler_Reactor_T<StatisticsContainerType>::~Stream_StatisticHandler_Reactor_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Reactor_T::~Stream_StatisticHandler_Reactor_T"));

}

template <typename StatisticsContainerType>
int
Stream_StatisticHandler_Reactor_T<StatisticsContainerType>::handle_timeout (const ACE_Time_Value& tv_in,
                                                                            const void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Reactor_T::handle_timeout"));

  ACE_UNUSED_ARG (tv_in);
  ACE_UNUSED_ARG (arg_in);

  switch (action_)
  {
    case ACTION_COLLECT:
    {
      StatisticsContainerType result;
      ACE_OS::memset (&result, 0, sizeof (StatisticsContainerType));

      try
      {
        if (!interfaceHandle_->collect (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to RPG_Common_IStatistic::collect(), continuing\n")));
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught an exception in RPG_Common_IStatistic::collect(), continuing\n")));
      }

      // *TODO*: what else can we do, dump the result somehow ?
      break;
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
                    ACE_TEXT ("caught an exception in RPG_Common_IStatistic::report(), continuing\n")));
      }

      break;
    }
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

template <typename StatisticsContainerType>
Stream_StatisticHandler_Proactor_T<StatisticsContainerType>::Stream_StatisticHandler_Proactor_T (const Stream_IStatistic_t* interfaceHandle_in,
                                                                                                 const ActionSpecifier_t& action_in)
 : inherited ()
 , interfaceHandle_ (interfaceHandle_in)
 , action_ (action_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Proactor_T::Stream_StatisticHandler_Proactor_T"));

}

template <typename StatisticsContainerType>
Stream_StatisticHandler_Proactor_T<StatisticsContainerType>::~Stream_StatisticHandler_Proactor_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Proactor_T::~Stream_StatisticHandler_Proactor_T"));

}

template <typename StatisticsContainerType>
void
Stream_StatisticHandler_Proactor_T<StatisticsContainerType>::handle_time_out (const ACE_Time_Value& tv_in,
                                                                              const void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StatisticHandler_Proactor_T::handle_time_out"));

  ACE_UNUSED_ARG (tv_in);
  ACE_UNUSED_ARG (arg_in);

  switch (action_)
  {
    case ACTION_COLLECT:
    {
      StatisticsContainerType result;
      ACE_OS::memset (&result, 0, sizeof (StatisticsContainerType));

      try
      {
        if (!interfaceHandle_->collect (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to RPG_Common_IStatistic::collect(), continuing\n")));
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught an exception in RPG_Common_IStatistic::collect(), continuing\n")));
      }

      // *TODO*: what else can we do, dump the result somehow ?
      break;
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
                    ACE_TEXT ("caught an exception in RPG_Common_IStatistic::report(), continuing\n")));
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
