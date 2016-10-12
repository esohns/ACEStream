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

#ifndef TEST_I_MODULE_SPLITTER_H
#define TEST_I_MODULE_SPLITTER_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_common.h"

#include "stream_misc_splitter.h"

#include "test_i_target_message.h"
#include "test_i_target_session_message.h"
#include "test_i_target_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Target_DirectShow_Module_Splitter
 : public Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                   Test_I_Target_DirectShow_ControlMessage_t,
                                   Test_I_Target_DirectShow_Stream_Message,
                                   Test_I_Target_DirectShow_Stream_SessionMessage,
                                   Test_I_Target_DirectShow_SessionData>
{
 public:
  Test_I_Target_DirectShow_Module_Splitter ();
  virtual ~Test_I_Target_DirectShow_Module_Splitter ();

  virtual bool initialize (const Test_I_Target_DirectShow_ModuleHandlerConfiguration&);

 private:
  typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                   Test_I_Target_DirectShow_ControlMessage_t,
                                   Test_I_Target_DirectShow_Stream_Message,
                                   Test_I_Target_DirectShow_Stream_SessionMessage,
                                   Test_I_Target_DirectShow_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_DirectShow_Module_Splitter (const Test_I_Target_DirectShow_Module_Splitter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_DirectShow_Module_Splitter& operator= (const Test_I_Target_DirectShow_Module_Splitter&))
};

//////////////////////////////////////////

class Test_I_Target_MediaFoundation_Module_Splitter
 : public Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                   Test_I_Target_MediaFoundation_ControlMessage_t,
                                   Test_I_Target_MediaFoundation_Stream_Message,
                                   Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                   Test_I_Target_MediaFoundation_SessionData>
{
 public:
  Test_I_Target_MediaFoundation_Module_Splitter ();
  virtual ~Test_I_Target_MediaFoundation_Module_Splitter ();

  virtual bool initialize (const Test_I_Target_MediaFoundation_ModuleHandlerConfiguration&);

 private:
  typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                   Test_I_Target_MediaFoundation_ControlMessage_t,
                                   Test_I_Target_MediaFoundation_Stream_Message,
                                   Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                   Test_I_Target_MediaFoundation_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_MediaFoundation_Module_Splitter (const Test_I_Target_MediaFoundation_Module_Splitter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_MediaFoundation_Module_Splitter& operator= (const Test_I_Target_MediaFoundation_Module_Splitter&))
};
#endif

#endif
