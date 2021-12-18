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

#ifndef STREAM_MISC_COMMON_H
#define STREAM_MISC_COMMON_H

#include "ace/Basic_Types.h"
#include "ace/Time_Value.h"

enum Stream_Miscellaneous_DelayModeType
{
  STREAM_MISCELLANEOUS_DELAY_MODE_BYTES = 0,
  STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES,
  ////////////////////////////////////////
  STREAM_MISCELLANEOUS_DELAY_MODE_MAX,
  STREAM_MISCELLANEOUS_DELAY_MODE_INVALID
};

struct Stream_Miscellaneous_DelayConfiguration
{
  Stream_Miscellaneous_DelayConfiguration ()
   : averageBytesPerInterval (0)
   , interval (ACE_Time_Value::zero)
   , mode (STREAM_MISCELLANEOUS_DELAY_MODE_INVALID)
  {}

  ACE_UINT64                              averageBytesPerInterval;
  ACE_Time_Value                          interval;
  enum Stream_Miscellaneous_DelayModeType mode;
};

#endif
