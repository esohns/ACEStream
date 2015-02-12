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

#ifndef STREAM_STATISTICHANDLER_H
#define STREAM_STATISTICHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Event_Handler.h"
#include "ace/Asynch_IO.h"

#include "common_istatistic.h"

enum ActionSpecifier_t
{
  ACTION_INVALID = -1,
  ACTION_COLLECT,
  ACTION_REPORT,
  /////////////////////////////////////
  ACTION_MAX
};

template <typename StatisticsContainerType>
class Stream_StatisticHandler_Reactor_T
 : public ACE_Event_Handler
{
 public:
  typedef Common_IStatistic_T<StatisticsContainerType> Stream_IStatistic_t;

  Stream_StatisticHandler_Reactor_T (const Stream_IStatistic_t*, // interface handle
                                     const ActionSpecifier_t&);  // handler action
  virtual ~Stream_StatisticHandler_Reactor_T ();

  // implement specific behaviour
  virtual int handle_timeout (const ACE_Time_Value&, // current time
                              const void*);          // asynchronous completion token

 private:
  typedef ACE_Event_Handler inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_Reactor_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_Reactor_T (const Stream_StatisticHandler_Reactor_T&));
  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_Reactor_T& operator= (const Stream_StatisticHandler_Reactor_T&));

  const Stream_IStatistic_t* interfaceHandle_;
  ActionSpecifier_t          action_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename StatisticsContainerType>
class Stream_StatisticHandler_Proactor_T
 : public ACE_Handler
{
 public:
  typedef Common_IStatistic_T<StatisticsContainerType> Stream_IStatistic_t;

  Stream_StatisticHandler_Proactor_T (const Stream_IStatistic_t*, // interface handle
                                      const ActionSpecifier_t&);  // handler action
  virtual ~Stream_StatisticHandler_Proactor_T ();

  // implement specific behaviour
  virtual void handle_time_out (const ACE_Time_Value&, // current time
                                const void* = NULL);   // asynchronous completion token

 private:
  typedef ACE_Handler inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_Proactor_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_Proactor_T (const Stream_StatisticHandler_Proactor_T&));
  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_Proactor_T& operator=(const Stream_StatisticHandler_Proactor_T&));

  const Stream_IStatistic_t* interfaceHandle_;
  ActionSpecifier_t          action_;
};

// include template implementation
#include "stream_statistichandler.inl"

#endif
