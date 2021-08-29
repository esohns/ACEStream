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
   : bytes (0.0F)
   , dataMessages (0)
   , sessionMessages (0)
   , bytesPerSecond (0.0F)
   , messagesPerSecond (0.0F)
   , timeStamp (ACE_Time_Value::zero)
  {}

  struct Stream_StatisticBase operator~ ()
  {
    bytes = 0.0F;
    dataMessages = 0;
    sessionMessages = 0;
    bytesPerSecond = 0.0F;
    messagesPerSecond = 0.0F;
    timeStamp = ACE_Time_Value::zero;

    return *this;
  }
  struct Stream_StatisticBase operator+= (const struct Stream_StatisticBase& rhs_in)
  {
    bytes += rhs_in.bytes;
    dataMessages += rhs_in.dataMessages;
    sessionMessages += rhs_in.sessionMessages;

    return *this;
  }

  float          bytes;           // amount of processed data
  unsigned int   dataMessages;    // data messages
  unsigned int   sessionMessages; // session messages

  // (current) runtime performance
  float          bytesPerSecond;
  float          messagesPerSecond;

  ACE_Time_Value timeStamp;
};

struct Stream_Statistic
 : Stream_StatisticBase
{
  Stream_Statistic ()
   : Stream_StatisticBase ()
   , capturedFrames (0)
   , droppedFrames (0)
   , totalFrames (0)
  {}

  struct Stream_Statistic operator~ ()
  {
    capturedFrames = 0;
    droppedFrames = 0;

    Stream_StatisticBase::operator~ ();

    return *this;
  }
  struct Stream_Statistic operator+= (const struct Stream_Statistic& rhs_in)
  {
    capturedFrames += rhs_in.capturedFrames;
    droppedFrames += rhs_in.droppedFrames;

    Stream_StatisticBase::operator+= (rhs_in);

    return *this;
  }

  unsigned int capturedFrames; // captured/generated frames
  unsigned int droppedFrames;  // dropped frames (i.e. driver congestion, buffer overflow, etc)

  unsigned int totalFrames;  // total # frames (progress)
};

#endif
