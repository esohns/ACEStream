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

#ifndef STREAM_STAT_STATISTIC_HANDLER_H
#define STREAM_STAT_STATISTIC_HANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"

#include "common_istatistic.h"

#include "common_itimerhandler.h"
#include "common_timer_handler.h"

enum Stream_StatisticActionType : int
{
  ACTION_COLLECT = 0,
  ACTION_REPORT,
  ////////////////////////////////////////
  ACTION_MAX,
  ACTION_INVALID
};

template <typename StatisticContainerType>
class Stream_StatisticHandler_T
 : public Common_TimerHandler
 , public Common_ITimerHandler
{
  typedef Common_TimerHandler inherited;

 public:
  // convenient types
  typedef Common_IStatistic_T<StatisticContainerType> ISTATISTIC_T;

  Stream_StatisticHandler_T (enum Stream_StatisticActionType,              // handler action
                             Common_IStatistic_T<StatisticContainerType>*, // interface handle
                             bool = false);                                // report on collect ?
  inline virtual ~Stream_StatisticHandler_T () {};

  // implement Common_ITimerHandler
  virtual void handle (const void*); // asynchronous completion token

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_T (const Stream_StatisticHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StatisticHandler_T& operator= (const Stream_StatisticHandler_T&))

  enum Stream_StatisticActionType action_;
  ISTATISTIC_T*                   interfaceHandle_;
  bool                            reportOnCollect_;
};

// include template definition
#include "stream_stat_statistic_handler.inl"

#endif
