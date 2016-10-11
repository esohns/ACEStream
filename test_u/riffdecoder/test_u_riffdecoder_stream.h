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

#ifndef TEST_U_RIFFDECODER_STREAM_H
#define TEST_U_RIFFDECODER_STREAM_H

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_riffdecoder_common.h"
#include "test_u_riffdecoder_common_modules.h"
#include "test_u_riffdecoder_message.h"
#include "test_u_riffdecoder_session_message.h"

// forward declarations
class Stream_IAllocator;

class Test_U_RIFFDecoder_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Test_U_RIFFDecoder_StreamConfiguration,
                        Stream_Statistic,
                        Stream_ModuleConfiguration,
                        Test_U_RIFFDecoder_ModuleHandlerConfiguration,
                        Test_U_RIFFDecoder_SessionData,   // session data
                        Test_U_RIFFDecoder_SessionData_t, // session data container (reference counted)
                        Test_U_ControlMessage_t,
                        Test_U_RIFFDecoder_Message,
                        Test_U_RIFFDecoder_SessionMessage>
{
 public:
  Test_U_RIFFDecoder_Stream ();
  virtual ~Test_U_RIFFDecoder_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_U_RIFFDecoder_StreamConfiguration&, // configuration
                           bool = true,                                   // setup pipeline ?
                           bool = true);                                  // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Stream_Statistic&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        Test_U_RIFFDecoder_StreamConfiguration,
                        Stream_Statistic,
                        Stream_ModuleConfiguration,
                        Test_U_RIFFDecoder_ModuleHandlerConfiguration,
                        Test_U_RIFFDecoder_SessionData,   // session data
                        Test_U_RIFFDecoder_SessionData_t, // session data container (reference counted)
                        Test_U_ControlMessage_t,
                        Test_U_RIFFDecoder_Message,
                        Test_U_RIFFDecoder_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_RIFFDecoder_Stream (const Test_U_RIFFDecoder_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_RIFFDecoder_Stream& operator= (const Test_U_RIFFDecoder_Stream&))

  // modules
  Test_U_RIFFDecoder_Module_Source_Module   source_;
  Test_U_RIFFDecoder_Module_Decoder_Module  decoder_;
  Test_U_RIFFDecoder_StatisticReport_Module statistic_;

  static ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> currentSessionID;
};

#endif
