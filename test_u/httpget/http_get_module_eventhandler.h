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

#ifndef HTTP_GET_MODULE_EVENTHANDLER_H
#define HTTP_GET_MODULE_EVENTHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Task_T.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

#include "http_get_message.h"
#include "http_get_session_message.h"
#include "http_get_stream_common.h"

class HTTPGet_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct HTTPGet_ModuleHandlerConfiguration,
                                         HTTPGet_ControlMessage_t,
                                         HTTPGet_Message,
                                         HTTPGet_SessionMessage,
                                         Stream_SessionId_t,
                                         struct HTTPGet_SessionData,
                                         struct Stream_UserData>
{
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct HTTPGet_ModuleHandlerConfiguration,
                                         HTTPGet_ControlMessage_t,
                                         HTTPGet_Message,
                                         HTTPGet_SessionMessage,
                                         Stream_SessionId_t,
                                         struct HTTPGet_SessionData,
                                         struct Stream_UserData> inherited;

 public:
  HTTPGet_Module_EventHandler (ISTREAM_T*); // stream handle
  inline virtual ~HTTPGet_Module_EventHandler () {}

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_Module_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_Module_EventHandler (const HTTPGet_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (HTTPGet_Module_EventHandler& operator= (const HTTPGet_Module_EventHandler&))
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct HTTPGet_SessionData,                // session data type
                              enum Stream_SessionMessageType,            // session event type
                              struct HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_INotify_t,                          // stream notification interface type
                              HTTPGet_Module_EventHandler);              // writer type

#endif
