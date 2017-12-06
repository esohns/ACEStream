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

#ifndef STREAM_MODULE_DB_TOOLS_H
#define STREAM_MODULE_DB_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"

#include "stream_db_exports.h"

// definitions
// *TODO*: remove ASAP
#define STREAM_MODULE_DB_TOOLS_STRFTIME_FORMAT "%Y-%m-%d %H:%M:%S"
// *NOTE*: '\0' doesn't count: 4 + 2 + 2 + 2 + 2 + 2 + 5 whitespaces
#define STREAM_MODULE_DB_TOOLS_STRFTIME_SIZE   19

class STREAM_Db_Export Stream_Module_DataBase_Tools
{
 public:
  // *IMPORTANT NOTE*: uses localtime() (i.e. returns a 'wall clock'
  //                   representation). Note that databases are 'mobile assets';
  //                   make sure you know what you are doing
  //                   --> verify that:
  //                       - a (standardized) value (UTC/...) is stored
  //                       [- the DBMS supports timezone configuration]
  //                       [- the application supports timezone configuration]
  //                       so the application can interpret this information
  static std::string timestampToDatabaseString (const ACE_Time_Value&); // timestamp

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_DataBase_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_DataBase_Tools (const Stream_Module_DataBase_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_DataBase_Tools& operator= (const Stream_Module_DataBase_Tools&))
};

#endif
