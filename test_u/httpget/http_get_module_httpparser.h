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

#ifndef HTTP_GET_MODULE_HTTPPARSER_H
#define HTTP_GET_MODULE_HTTPPARSER_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_common.h"

#include "http_module_parser.h"

#include "http_get_stream_common.h"
#include "http_get_message.h"
#include "http_get_session_message.h"

class HTTPGet_Module_HTTPParser
 : public HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                HTTPGet_ControlMessage_t,
                                HTTPGet_Message,
                                HTTPGet_SessionMessage,
                                struct HTTPGet_ModuleHandlerConfiguration,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct HTTPGet_StreamState,
                                struct HTTPGet_SessionData,
                                HTTPGet_SessionData_t,
                                struct Stream_Statistic,
                                struct Stream_UserData>
{
 public:
  HTTPGet_Module_HTTPParser ();
  virtual ~HTTPGet_Module_HTTPParser ();

//  // override (part of) HTTP_IParser
//  virtual void record (struct HTTP_Record*&); // target data record

 private:
  typedef HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                HTTPGet_ControlMessage_t,
                                HTTPGet_Message,
                                HTTPGet_SessionMessage,
                                struct HTTPGet_ModuleHandlerConfiguration,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct HTTPGet_StreamState,
                                struct HTTPGet_SessionData,
                                HTTPGet_SessionData_t,
                                struct Stream_Statistic,
                                struct Stream_UserData> inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTPGet_Module_HTTPParser (const HTTPGet_Module_HTTPParser&))
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_Module_HTTPParser& operator= (const HTTPGet_Module_HTTPParser&))
};

#endif
