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

#include "stream_exports.h"

class Stream_Export Stream_Tools
{
 public:
  // *WARNING*: beware, this uses localtime_r internally, so you should probably pass in a local time
  // - uses strftime internally: "%Y_%m_%d_%H_%M_%S" (see manpage)
  static std::string timestamp2LocalString (const ACE_Time_Value&); // timestamp

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Tools ());
  ACE_UNIMPLEMENTED_FUNC (Stream_Tools (const Stream_Tools&));
  ACE_UNIMPLEMENTED_FUNC (Stream_Tools& operator= (const Stream_Tools&));
};

#endif
