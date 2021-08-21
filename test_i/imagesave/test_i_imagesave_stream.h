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

#ifndef TEST_I_STREAM_H
#define TEST_I_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_i_imagesave_common.h"
#include "test_i_imagesave_common_modules.h"
#include "test_i_imagesave_message.h"
#include "test_i_imagesave_session_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

class Test_I_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_ImageSave_StreamState,
                        struct Test_I_ImageSave_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_ImageSave_ModuleHandlerConfiguration,
                        Test_I_ImageSave_SessionData,
                        Test_I_ImageSave_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_ImageSave_StreamState,
                        struct Test_I_ImageSave_StreamConfiguration,
                        struct Test_I_StatisticData,
                        struct Test_I_ImageSave_ModuleHandlerConfiguration,
                        Test_I_ImageSave_SessionData,
                        Test_I_ImageSave_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage_t> inherited;

 public:
  Test_I_Stream ();
  inline virtual ~Test_I_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream (const Test_I_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream& operator= (const Test_I_Stream&))

  // modules
  Test_I_Source_Module          source_;
  Test_I_LibAVDecoder_Module    decoder_;
  //Test_I_StatisticReport_Module report_;
  Test_I_Display_Module         display_;
};

#endif
