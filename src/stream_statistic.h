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

#ifndef STREAM_STATISTIC_H
#define STREAM_STATISTIC_H

#include "ace/Time_Value.h"

struct Stream_StatisticBase
{
  Stream_StatisticBase ()
   : bytes (0)
   , bytesPerSecond (0.0f)
   , timeStamp (ACE_Time_Value::zero)
  {}

  struct Stream_StatisticBase operator~ ()
  {
    bytes = 0;
    bytesPerSecond = 0.0f;
    timeStamp = ACE_Time_Value::zero;

    return *this;
  }
  struct Stream_StatisticBase operator+= (const struct Stream_StatisticBase& rhs_in)
  {
    bytes += rhs_in.bytes;
    //bytesPerSecond += rhs_in.bytesPerSecond;

    return *this;
  }

  ACE_UINT64     bytes; // amount of processed data

  // (current) runtime performance
  float          bytesPerSecond;

  ACE_Time_Value timeStamp;
};

struct Stream_Statistic
 : Stream_StatisticBase
{
  Stream_Statistic ()
   : Stream_StatisticBase ()
   , dataMessages (0)
   , sessionMessages (0)
   , messagesPerSecond (0.0f)
   ////////////////////////////////////////
   , capturedFrames (0)
   , droppedFrames (0)
   , totalFrames (0)
  {}

  struct Stream_Statistic operator~ ()
  {
    Stream_StatisticBase::operator~ ();

    dataMessages = 0;
    sessionMessages = 0;
    messagesPerSecond = 0.0f;

    capturedFrames = 0;
    droppedFrames = 0;
    totalFrames = 0;

    return *this;
  }
  struct Stream_Statistic operator+= (const struct Stream_Statistic& rhs_in)
  {
    Stream_StatisticBase::operator+= (rhs_in);

    dataMessages += rhs_in.dataMessages;
    sessionMessages += rhs_in.sessionMessages;
    // messagesPerSecond += rhs_in.messagesPerSecond;

    capturedFrames += rhs_in.capturedFrames;
    droppedFrames += rhs_in.droppedFrames;

    totalFrames += rhs_in.totalFrames;

    return *this;
  }

  ACE_UINT32 dataMessages;      // data messages
  ACE_UINT32 sessionMessages;   // session messages
  float      messagesPerSecond;
  ////////////////////////////////////////
  ACE_UINT32 capturedFrames;    // captured/generated frames
  ACE_UINT32 droppedFrames;     // dropped frames (i.e. driver congestion, buffer
                                // overflow, etc)

  ACE_UINT32 totalFrames;       // total # frames (progress)
};

#endif
