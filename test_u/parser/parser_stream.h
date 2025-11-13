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

#ifndef PARSER_STREAM_H
#define PARSER_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "common_parser_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "parser_message.h"
#include "parser_session_message.h"
#include "parser_stream_common.h"

class Parser_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        default_stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Parser_StreamState,
                        struct Stream_Configuration,
                        struct Stream_Statistic,
                        struct Parser_ModuleHandlerConfiguration,
                        Test_U_SessionManager_t,
                        Stream_ControlMessage_t,
                        Parser_Message,
                        Parser_SessionMessage,
                        struct Stream_UserData>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        default_stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Parser_StreamState,
                        struct Stream_Configuration,
                        struct Stream_Statistic,
                        struct Parser_ModuleHandlerConfiguration,
                        Test_U_SessionManager_t,
                        Stream_ControlMessage_t,
                        Parser_Message,
                        Parser_SessionMessage,
                        struct Stream_UserData> inherited;

 public:
  Parser_Stream ();
  virtual ~Parser_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // layout handle
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Parser_Stream (const Parser_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Parser_Stream& operator= (const Parser_Stream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

#endif
