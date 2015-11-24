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

#ifndef STREAM_TOOLS_H
#define STREAM_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"

//#include "stream_common.h"
#include "stream_exports.h"

// forward declarations
enum Stream_MessageType : int;

// definitions
#define STREAM_TOOLS_STRFTIME_FORMAT "%Y_%m_%d_%H_%M_%S"
// *NOTE*: '\0' doesn't count: 4 + 2 + 2 + 2 + 2 + 2 + 5 whitespaces
#define STREAM_TOOLS_STRFTIME_SIZE   19

class Stream_Export Stream_Tools
{
 public:
  static std::string messageType2String (Stream_MessageType); // as returned by msg_type()

  // *WARNING*: this uses localtime_r internally --> pass in a local time
  //            - uses strftime() internally (see man page, format)
  static std::string timeStamp2LocalString (const ACE_Time_Value&); // timestamp

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Tools (const Stream_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Tools& operator= (const Stream_Tools&))
};

#endif
