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

#ifndef TEST_U_FILECOPY_MODULE_EVENTHANDLER_H
#define TEST_U_FILECOPY_MODULE_EVENTHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

#include "test_u_filecopy_common.h"
#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

class Stream_Filecopy_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Stream_Filecopy_ModuleHandlerConfiguration,
                                         ACE_Message_Block,
                                         Stream_Filecopy_Message,
                                         Stream_Filecopy_SessionMessage,
                                         Stream_SessionId_t,
                                         struct Stream_Filecopy_SessionData,
                                         struct Stream_UserData>
{
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Stream_Filecopy_ModuleHandlerConfiguration,
                                         ACE_Message_Block,
                                         Stream_Filecopy_Message,
                                         Stream_Filecopy_SessionMessage,
                                         Stream_SessionId_t,
                                         struct Stream_Filecopy_SessionData,
                                         struct Stream_UserData> inherited;

 public:
  Stream_Filecopy_Module_EventHandler (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Filecopy_Module_EventHandler () {}

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Module_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Module_EventHandler (const Stream_Filecopy_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Module_EventHandler& operator= (const Stream_Filecopy_Module_EventHandler&))
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_Filecopy_SessionData,                // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_Filecopy_IStreamNotify_t,                   // stream notification interface type
                              Stream_Filecopy_Module_EventHandler);              // writer type

#endif
