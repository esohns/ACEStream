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

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_stat_common.h"
#include "stream_stat_statistic_report.h"

#include "test_i_message.h"
#include "test_i_modules.h"
#include "test_i_session_message.h"
#include "test_i_speechcommand_common.h"

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_I_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_DirectShow_ModuleHandlerConfiguration,
                        Test_I_DirectShow_SessionData,
                        Test_I_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_DirectShow_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_I_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_DirectShow_ModuleHandlerConfiguration,
                        Test_I_DirectShow_SessionData,
                        Test_I_DirectShow_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_DirectShow_SessionMessage_t> inherited;

 public:
  Test_I_DirectShow_Stream ();
  inline virtual ~Test_I_DirectShow_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&); // configuration
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration
#endif // ACE_WIN32 || ACE_WIN64

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (struct Stream_Statistic&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

 private:
  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Test_I_DirectShow_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        Test_I_Message,
                                                        Test_I_DirectShow_SessionMessage_t,
                                                        int,
                                                        struct Stream_Statistic,
                                                        Common_Timer_Manager_t,
                                                        Test_I_DirectShow_SessionData,
                                                        Test_I_DirectShow_SessionData_t> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Test_I_DirectShow_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        Test_I_Message,
                                                        Test_I_DirectShow_SessionMessage_t,
                                                        int,
                                                        struct Stream_Statistic,
                                                        Common_Timer_Manager_t,
                                                        Test_I_DirectShow_SessionData,
                                                        Test_I_DirectShow_SessionData_t> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Test_I_DirectShow_SessionData,        // session data type
                                enum Stream_SessionMessageType,       // session event type
                                struct Stream_ModuleConfiguration,
                                struct Test_I_DirectShow_ModuleHandlerConfiguration,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,                     // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Stream (const Test_I_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Stream& operator= (const Test_I_DirectShow_Stream&))

  // modules
  MODULE_STATISTIC_T statistic_;
};

class Test_I_MediaFoundation_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_I_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_MediaFoundation_SessionData,
                        Test_I_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_MediaFoundation_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_I_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_MediaFoundation_SessionData,
                        Test_I_MediaFoundation_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_MediaFoundation_SessionMessage_t> inherited;

 public:
  Test_I_MediaFoundation_Stream ();
  inline virtual ~Test_I_MediaFoundation_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&); // configuration
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration
#endif // ACE_WIN32 || ACE_WIN64

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (struct Stream_Statistic&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

 private:
  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Test_I_MediaFoundation_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        Test_I_Message,
                                                        Test_I_MediaFoundation_SessionMessage_t,
                                                        int,
                                                        struct Stream_Statistic,
                                                        Common_Timer_Manager_t,
                                                        Test_I_MediaFoundation_SessionData,
                                                        Test_I_MediaFoundation_SessionData_t> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Test_I_MediaFoundation_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        Test_I_Message,
                                                        Test_I_MediaFoundation_SessionMessage_t,
                                                        int,
                                                        struct Stream_Statistic,
                                                        Common_Timer_Manager_t,
                                                        Test_I_MediaFoundation_SessionData,
                                                        Test_I_MediaFoundation_SessionData_t> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Test_I_MediaFoundation_SessionData,   // session data type
                                enum Stream_SessionMessageType,       // session event type
                                struct Stream_ModuleConfiguration,
                                struct Test_I_MediaFoundation_ModuleHandlerConfiguration,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,                     // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Stream (const Test_I_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Stream& operator= (const Test_I_MediaFoundation_Stream&))

  // modules
  MODULE_STATISTIC_T statistic_;
};
#else
class Test_I_ALSA_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_I_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_ALSA_ModuleHandlerConfiguration,
                        Test_I_ALSA_SessionData,
                        Test_I_ALSA_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_ALSA_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_State,
                        struct Test_I_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_ALSA_ModuleHandlerConfiguration,
                        Test_I_ALSA_SessionData,
                        Test_I_ALSA_SessionData_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_ALSA_SessionMessage_t> inherited;

 public:
  Test_I_ALSA_Stream ();
  inline virtual ~Test_I_ALSA_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&); // configuration
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration
#endif // ACE_WIN32 || ACE_WIN64

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (struct Stream_Statistic&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

 private:
  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Test_I_ALSA_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        Test_I_Message,
                                                        Test_I_ALSA_SessionMessage_t,
                                                        int,
                                                        struct Stream_Statistic,
                                                        Common_Timer_Manager_t,
                                                        Test_I_ALSA_SessionData,
                                                        Test_I_ALSA_SessionData_t> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Test_I_ALSA_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        Test_I_Message,
                                                        Test_I_ALSA_SessionMessage_t,
                                                        int,
                                                        struct Stream_Statistic,
                                                        Common_Timer_Manager_t,
                                                        Test_I_ALSA_SessionData,
                                                        Test_I_ALSA_SessionData_t> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                Test_I_ALSA_SessionData,              // session data type
                                enum Stream_SessionMessageType,       // session event type
                                struct Stream_ModuleConfiguration,
                                struct Test_I_ALSA_ModuleHandlerConfiguration,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,                     // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_ALSA_Stream (const Test_I_ALSA_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_ALSA_Stream& operator= (const Test_I_ALSA_Stream&))

  // modules
  MODULE_STATISTIC_T statistic_;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
