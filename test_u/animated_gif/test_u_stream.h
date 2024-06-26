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

#ifndef TEST_U_STREAM_H
#define TEST_U_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_animated_gif_common.h"
#include "test_u_common_modules.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

class Test_U_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_StreamState,
                        struct Stream_Configuration,
                        struct Stream_Statistic,
                        struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                        Test_U_AnimatedGIF_SessionData, // session data
                        Test_U_AnimatedGIF_SessionData_t, // session data container (reference counted)
                        Stream_ControlMessage_t,
                        Test_U_Message,
                        Test_U_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_StreamState,
                        struct Stream_Configuration,
                        struct Stream_Statistic,
                        struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                        Test_U_AnimatedGIF_SessionData,
                        Test_U_AnimatedGIF_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_U_Message,
                        Test_U_SessionMessage> inherited;

 public:
  Test_U_Stream ();
  virtual ~Test_U_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream (const Test_U_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream& operator= (const Test_U_Stream&))
};

#endif // TEST_U_STREAM_H
