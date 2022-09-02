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

#ifndef TEST_I_DECODER_T_H
#define TEST_I_DECODER_T_H

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Global_Macros.h"

#include "common_timer_manager_common.h"

#include "stream_headmoduletask_base.h"
#include "stream_streammodule_base.h"

#include "stream_dec_libav_decoder.h"

#include "test_i_extract_stream_common.h"
#include "test_i_message.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload
                                             // (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Test_I_Decoder_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType>
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Decoder_T (ISTREAM_T*); // stream handle
#else
  Test_I_Decoder_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_Decoder_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

 private:
  // convenient types
  typedef Test_I_Decoder_T<ACE_SYNCH_USE,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           TimerManagerType,
                           UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Decoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Decoder_T (const Test_I_Decoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Decoder_T& operator= (const Test_I_Decoder_T&))

  // helper methods
  virtual int svc (void);

  struct AVFormatContext* context_;
};

// include template definition
#include "test_i_decoder.inl"

//////////////////////////////////////////

typedef Test_I_Decoder_T<ACE_MT_SYNCH,
                         Stream_ControlMessage_t,
                         Test_I_Message_t,
                         Test_I_SessionMessage_t,
                         struct Test_I_ExtractStream_ModuleHandlerConfiguration,
                         enum Stream_ControlType,
                         enum Stream_SessionMessageType,
                         struct Test_I_ExtractStream_StreamState,
                         Test_I_ExtractStream_SessionData,
                         Test_I_ExtractStream_SessionData_t,
                         struct Stream_Statistic,
                         Common_Timer_Manager_t,
                         struct Stream_UserData> Test_I_Decoder;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_ExtractStream_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_ExtractStream_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_Decoder);                                           // module name

#endif
