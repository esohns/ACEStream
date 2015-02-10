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

#ifndef RPG_STREAM_TOOLS_H
#define RPG_STREAM_TOOLS_H

#include "rpg_stream_exports.h"

#include <ace/Global_Macros.h>
#include <ace/Time_Value.h>

#include <string>

class RPG_Stream_Export RPG_Stream_Tools
{
 public:
  // *WARNING*: beware, this uses localtime_r internally, so you should probably pass in a local time
  // - uses strftime internally: "%Y_%m_%d_%H_%M_%S" (see manpage)
  static const std::string timestamp2LocalString(const ACE_Time_Value&); // timestamp

 private:
  // safety measures
  ACE_UNIMPLEMENTED_FUNC(RPG_Stream_Tools());
  ACE_UNIMPLEMENTED_FUNC(RPG_Stream_Tools(const RPG_Stream_Tools&));
  ACE_UNIMPLEMENTED_FUNC(RPG_Stream_Tools& operator=(const RPG_Stream_Tools&));
};

#endif
