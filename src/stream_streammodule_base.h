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

#ifndef STREAM_STREAMMODULE_BASE_H
#define STREAM_STREAMMODULE_BASE_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_istreamcontrol.h"
#include "stream_module_base.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType, // *NOTE*: stream notification interface
          typename ReaderTaskType,
          typename WriterTaskType>
class Stream_StreamModule_T
 : public Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               NotificationType,
                               ReaderTaskType,
                               WriterTaskType>
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef ReaderTaskType READER_T;
  typedef WriterTaskType WRITER_T;

  Stream_StreamModule_T (ISTREAM_T*,          // stream handle
                         const std::string&); // name
  virtual ~Stream_StreamModule_T ();

 private:
  typedef Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               NotificationType,
                               ReaderTaskType,
                               WriterTaskType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T (const Stream_StreamModule_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T& operator= (const Stream_StreamModule_T&))

  ReaderTaskType reader_;
  WriterTaskType writer_;
};

//////////////////////////////////////////

// *NOTE*: partial template specialization does not work when used via the
//         DATASTREAM_MODULE_* macro code (apparently the typedef resolution is
//         not finding the specialization)
//         --> define a separate template for input-only modules
// *TODO*: compilation fails with MSVC 2015u3, try gcc

//// partial specialization (input only)
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename SessionIdType,
//          typename SessionDataType,
//          typename SessionEventType,
//          typename ConfigurationType,
//          typename HandlerConfigurationType,
//          typename NotificationType, // *NOTE*: stream notification interface
//          typename TaskType>
//class Stream_StreamModule_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            SessionIdType,
//                            SessionDataType,
//                            SessionEventType,
//                            ConfigurationType,
//                            HandlerConfigurationType,
//                            NotificationType,
//                            ACE_Thru_Task<ACE_SYNCH_USE,
//                                          TimePolicyType>,
//                            TaskType>
// : public Stream_Module_Base_T<ACE_SYNCH_USE,
//                               TimePolicyType,
//                               SessionIdType,
//                               SessionDataType,
//                               SessionEventType,
//                               ConfigurationType,
//                               HandlerConfigurationType,
//                               NotificationType,
//                               ACE_Thru_Task<ACE_SYNCH_USE,
//                                             TimePolicyType>,
//                               TaskType>
//{
// public:
//  // convenient types
//  typedef Stream_IStream_T<ACE_SYNCH_USE,
//                           TimePolicyType> ISTREAM_T;
//  typedef ACE_Thru_Task<ACE_SYNCH_USE,
//                        TimePolicyType> READER_T;
//  typedef TaskType WRITER_T;
//
//  Stream_StreamModule_T (ISTREAM_T*,         // stream handle
//                         const std::string&, // name
//                         Common_IRefCount*,  // object counter
//                         bool = false);      // final module ?
//  virtual ~Stream_StreamModule_T ();
//
// private:
//  typedef Stream_Module_Base_T<ACE_SYNCH_USE,
//                               TimePolicyType,
//                               SessionIdType,
//                               SessionDataType,
//                               SessionEventType,
//                               ConfigurationType,
//                               HandlerConfigurationType,
//                               NotificationType,
//                               ACE_Thru_Task<ACE_SYNCH_USE,
//                                             TimePolicyType>,
//                               TaskType> inherited;
//
//  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T ())
//  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T (const Stream_StreamModule_T&))
//  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T& operator= (const Stream_StreamModule_T&))
//
//  ACE_Thru_Task<ACE_SYNCH_USE,
//                TimePolicyType> reader_;
//  TaskType                      writer_;
//};

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType, // *NOTE*: stream notification interface
          typename TaskType>
class Stream_StreamModuleInputOnly_T
 : public Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               NotificationType,
                               ACE_Thru_Task<ACE_SYNCH_USE,
                                             TimePolicyType>,
                               TaskType>
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> READER_T;
  typedef TaskType WRITER_T;

  Stream_StreamModuleInputOnly_T (ISTREAM_T*,          // stream handle
                                  const std::string&); // name
  virtual ~Stream_StreamModuleInputOnly_T ();

 private:
  typedef Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               NotificationType,
                               ACE_Thru_Task<ACE_SYNCH_USE,
                                             TimePolicyType>,
                               TaskType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnly_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnly_T (const Stream_StreamModuleInputOnly_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnly_T& operator= (const Stream_StreamModuleInputOnly_T&))

  ACE_Thru_Task<ACE_SYNCH_USE,
                TimePolicyType> reader_;
  TaskType                      writer_;
};
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType, // *NOTE*: stream notification interface
          typename TaskType>
class Stream_StreamModuleOutputOnly_T
 : public Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               NotificationType,
                               TaskType,
                               ACE_Thru_Task<ACE_SYNCH_USE,
                                             TimePolicyType> >
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> WRITER_T;
  typedef TaskType READER_T;

  Stream_StreamModuleOutputOnly_T (ISTREAM_T*,          // stream handle
                                   const std::string&); // name
  virtual ~Stream_StreamModuleOutputOnly_T ();

 private:
  typedef Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               NotificationType,
                               TaskType,
                               ACE_Thru_Task<ACE_SYNCH_USE,
                                             TimePolicyType> > inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleOutputOnly_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleOutputOnly_T (const Stream_StreamModuleOutputOnly_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleOutputOnly_T& operator= (const Stream_StreamModuleOutputOnly_T&))

  ACE_Thru_Task<ACE_SYNCH_USE,
                TimePolicyType> writer_;
  TaskType                      reader_;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType, // *NOTE*: stream notification interface
          typename ReaderTaskType,
          typename WriterTaskType>
class Stream_StreamModuleA_T
 : public Stream_Module_BaseA_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionIdType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                NotificationType,
                                ReaderTaskType,
                                WriterTaskType>
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef ReaderTaskType READER_T;
  typedef WriterTaskType WRITER_T;

  Stream_StreamModuleA_T (ISTREAM_T*,          // stream handle
                          const std::string&); // name
  virtual ~Stream_StreamModuleA_T ();

 private:
  typedef Stream_Module_BaseA_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionIdType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                NotificationType,
                                ReaderTaskType,
                                WriterTaskType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleA_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleA_T (const Stream_StreamModuleA_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleA_T& operator= (const Stream_StreamModuleA_T&))

  ReaderTaskType reader_;
  WriterTaskType writer_;
};

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType, // *NOTE*: stream notification interface
          typename TaskType>
class Stream_StreamModuleInputOnlyA_T
 : public Stream_Module_BaseA_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionIdType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                NotificationType,
                                ACE_Thru_Task<ACE_SYNCH_USE,
                                              TimePolicyType>,
                                TaskType>
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> READER_T;
  typedef TaskType WRITER_T;

  Stream_StreamModuleInputOnlyA_T (ISTREAM_T*,          // stream handle
                                   const std::string&); // name
  virtual ~Stream_StreamModuleInputOnlyA_T ();

 private:
  typedef Stream_Module_BaseA_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionIdType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                NotificationType,
                                ACE_Thru_Task<ACE_SYNCH_USE,
                                              TimePolicyType>,
                                TaskType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnlyA_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnlyA_T (const Stream_StreamModuleInputOnlyA_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnlyA_T& operator= (const Stream_StreamModuleInputOnlyA_T&))

  ACE_Thru_Task<ACE_SYNCH_USE,
                TimePolicyType> reader_;
  TaskType                      writer_;
};

// include template definition
#include "stream_streammodule_base.inl"

//////////////////////////////////////////

// *NOTE*: use these macros to instantiate the module definitions
// *IMPORTANT NOTE*: TASK_SYNCH_TYPE is [ACE_MT_SYNCH | ACE_NULL_SYNCH] and MUST
//                   correspond with the actual TASK_TYPE declaration !
// *TODO*: --> remove TASK_TYPE

#define DATASTREAM_MODULE_DUPLEX(SESSION_DATA_TYPE,\
                                 SESSION_EVENT_TYPE,\
                                 HANDLER_CONFIGURATION_TYPE,\
                                 NOTIFICATION_TYPE,\
                                 READER_TYPE,\
                                 WRITER_TYPE,\
                                 NAME) typedef Stream_StreamModule_T<ACE_MT_SYNCH,\
                                                                     Common_TimePolicy_t,\
                                                                     Stream_SessionId_t,\
                                                                     SESSION_DATA_TYPE,\
                                                                     SESSION_EVENT_TYPE,\
                                                                     struct Stream_ModuleConfiguration,\
                                                                     HANDLER_CONFIGURATION_TYPE,\
                                                                     NOTIFICATION_TYPE,\
                                                                     READER_TYPE,\
                                                                     WRITER_TYPE> NAME##_Module
//#define DATASTREAM_MODULE_INPUT_ONLY(SESSION_DATA_TYPE,\
//                                     SESSION_EVENT_TYPE,\
//                                     HANDLER_CONFIGURATION_TYPE,\
//                                     NOTIFICATION_TYPE,\
//                                     TASK_TYPE) typedef Stream_StreamModule_T<ACE_MT_SYNCH,\
//                                                                              Common_TimePolicy_t,\
//                                                                              Stream_SessionId_t,\
//                                                                              SESSION_DATA_TYPE,\
//                                                                              SESSION_EVENT_TYPE,\
//                                                                              struct Stream_ModuleConfiguration,\
//                                                                              HANDLER_CONFIGURATION_TYPE,\
//                                                                              NOTIFICATION_TYPE,\
//                                                                              TASK_TYPE> TASK_TYPE##_Module
//#define DATASTREAM_MODULE_INPUT_ONLY_T(SESSION_DATA_TYPE,\
//                                       SESSION_EVENT_TYPE,\
//                                       HANDLER_CONFIGURATION_TYPE,\
//                                       NOTIFICATION_TYPE,\
//                                       TASK_TYPE,\
//                                       NAME) typedef Stream_StreamModule_T<ACE_MT_SYNCH,\
//                                                                           Common_TimePolicy_t,\
//                                                                           Stream_SessionId_t,\
//                                                                           SESSION_DATA_TYPE,\
//                                                                           SESSION_EVENT_TYPE,\
//                                                                           struct Stream_ModuleConfiguration,\
//                                                                           HANDLER_CONFIGURATION_TYPE,\
//                                                                           NOTIFICATION_TYPE,\
//                                                                           TASK_TYPE> NAME##_Module
#define DATASTREAM_MODULE_INPUT_ONLY(SESSION_DATA_TYPE,\
                                     SESSION_EVENT_TYPE,\
                                     HANDLER_CONFIGURATION_TYPE,\
                                     NOTIFICATION_TYPE,\
                                     TASK_TYPE) typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,\
                                                                                       Common_TimePolicy_t,\
                                                                                       Stream_SessionId_t,\
                                                                                       SESSION_DATA_TYPE,\
                                                                                       SESSION_EVENT_TYPE,\
                                                                                       struct Stream_ModuleConfiguration,\
                                                                                       HANDLER_CONFIGURATION_TYPE,\
                                                                                       NOTIFICATION_TYPE,\
                                                                                       TASK_TYPE> TASK_TYPE##_Module
#define DATASTREAM_MODULE_INPUT_ONLY_T(SESSION_DATA_TYPE,\
                                       SESSION_EVENT_TYPE,\
                                       HANDLER_CONFIGURATION_TYPE,\
                                       NOTIFICATION_TYPE,\
                                       TASK_TYPE,\
                                       NAME) typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,\
                                                                                    Common_TimePolicy_t,\
                                                                                    Stream_SessionId_t,\
                                                                                    SESSION_DATA_TYPE,\
                                                                                    SESSION_EVENT_TYPE,\
                                                                                    struct Stream_ModuleConfiguration,\
                                                                                    HANDLER_CONFIGURATION_TYPE,\
                                                                                    NOTIFICATION_TYPE,\
                                                                                    TASK_TYPE> NAME##_Module

#define DATASTREAM_MODULE_OUTPUT_ONLY(SESSION_DATA_TYPE,\
                                      SESSION_EVENT_TYPE,\
                                      HANDLER_CONFIGURATION_TYPE,\
                                      NOTIFICATION_TYPE,\
                                      TASK_TYPE) typedef Stream_StreamModuleOutputOnly_T<ACE_MT_SYNCH,\
                                                                                         Common_TimePolicy_t,\
                                                                                         Stream_SessionId_t,\
                                                                                         SESSION_DATA_TYPE,\
                                                                                         SESSION_EVENT_TYPE,\
                                                                                         struct Stream_ModuleConfiguration,\
                                                                                         HANDLER_CONFIGURATION_TYPE,\
                                                                                         NOTIFICATION_TYPE,\
                                                                                         TASK_TYPE> TASK_TYPE##_Module

//////////////////////////////////////////

#define DATASTREAM_MODULE_DUPLEX_A(SESSION_DATA_TYPE,\
                                   SESSION_EVENT_TYPE,\
                                   HANDLER_CONFIGURATION_TYPE,\
                                   NOTIFICATION_TYPE,\
                                   READER_TYPE,\
                                   WRITER_TYPE,\
                                   NAME) typedef Stream_StreamModuleA_T<ACE_MT_SYNCH,\
                                                                        Common_TimePolicy_t,\
                                                                        Stream_SessionId_t,\
                                                                        SESSION_DATA_TYPE,\
                                                                        SESSION_EVENT_TYPE,\
                                                                        struct Stream_ModuleConfiguration,\
                                                                        HANDLER_CONFIGURATION_TYPE,\
                                                                        NOTIFICATION_TYPE,\
                                                                        READER_TYPE,\
                                                                        WRITER_TYPE> NAME##_Module

#define DATASTREAM_MODULE_INPUT_ONLY_A(SESSION_DATA_TYPE,\
                                       SESSION_EVENT_TYPE,\
                                       HANDLER_CONFIGURATION_TYPE,\
                                       NOTIFICATION_TYPE,\
                                       TASK_TYPE) typedef Stream_StreamModuleInputOnlyA_T<ACE_MT_SYNCH,\
                                                                                          Common_TimePolicy_t,\
                                                                                          Stream_SessionId_t,\
                                                                                          SESSION_DATA_TYPE,\
                                                                                          SESSION_EVENT_TYPE,\
                                                                                          struct Stream_ModuleConfiguration,\
                                                                                          HANDLER_CONFIGURATION_TYPE,\
                                                                                          NOTIFICATION_TYPE,\
                                                                                          TASK_TYPE> TASK_TYPE##_Module
#define DATASTREAM_MODULE_INPUT_ONLY_A_T(SESSION_DATA_TYPE,\
                                         SESSION_EVENT_TYPE,\
                                         HANDLER_CONFIGURATION_TYPE,\
                                         NOTIFICATION_TYPE,\
                                         TASK_TYPE,\
                                         NAME) typedef Stream_StreamModuleInputOnlyA_T<ACE_MT_SYNCH,\
                                                                                       Common_TimePolicy_t,\
                                                                                       Stream_SessionId_t,\
                                                                                       SESSION_DATA_TYPE,\
                                                                                       SESSION_EVENT_TYPE,\
                                                                                       struct Stream_ModuleConfiguration,\
                                                                                       HANDLER_CONFIGURATION_TYPE,\
                                                                                       NOTIFICATION_TYPE,\
                                                                                       TASK_TYPE> NAME##_Module

#endif
