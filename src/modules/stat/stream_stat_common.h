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

#ifndef STREAM_STAT_COMMON_H
#define STREAM_STAT_COMMON_H

#include "common_statistic_handler.h"

#include "stream_common.h"

//enum Stream_Statistic_AnalysisModeType : int
//{
//  STREAM_STATISTIC_ANALYSIS_MODE_SLIDINGAVERAGE = 0,
//  ////////////////////////////////////////
//  STREAM_STATISTIC_ANALYSIS_MODE_MAX,
//  STREAM_STATISTIC_ANALYSIS_MODE_INVALID
//};

enum Stream_Statistic_AnalysisEventType
{
  STREAM_STATISTIC_ANALYSIS_EVENT_ACTIVITY = 0,
  STREAM_STATISTIC_ANALYSIS_EVENT_PEAK,
  ////////////////////////////////////////
  STREAM_STATISTIC_ANALYSIS_EVENT_MAX,
  STREAM_STATISTIC_ANALYSIS_EVENT_INVALID
};

//////////////////////////////////////////

typedef Common_StatisticHandler_T<struct Stream_Statistic> Stream_StatisticHandler_t;

#endif
