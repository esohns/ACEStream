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
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"
#include "stream_module_messagehandler.h"

#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

class Stream_Filecopy_Module_EventHandler
 : public Stream_Module_MessageHandler_T<Stream_Filecopy_SessionMessage,
                                         Stream_Filecopy_Message,
                                         
                                         Stream_ModuleHandlerConfiguration,
                                         
                                         Stream_SessionData>
{
 public:
  Stream_Filecopy_Module_EventHandler ();
  virtual ~Stream_Filecopy_Module_EventHandler ();

  // implement Common_IClone_T
  virtual Stream_Module_t* clone ();

 private:
  typedef Stream_Module_MessageHandler_T<Stream_Filecopy_SessionMessage,
                                         Stream_Filecopy_Message,
                                         
                                         Stream_ModuleHandlerConfiguration,
                                         
                                         Stream_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Module_EventHandler (const Stream_Filecopy_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Module_EventHandler& operator= (const Stream_Filecopy_Module_EventHandler&))
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                         // task synch type
                              Common_TimePolicy_t,                  // time policy
                              Stream_ModuleConfiguration,           // module configuration type
                              Stream_ModuleHandlerConfiguration,    // module handler configuration type
                              Stream_Filecopy_Module_EventHandler); // writer type

#endif
