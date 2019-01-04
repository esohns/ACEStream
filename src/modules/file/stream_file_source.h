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

#ifndef STREAM_MODULE_FILEREADER_H
#define STREAM_MODULE_FILEREADER_H

#include "ace/FILE_IO.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_ilock.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_task_base_asynch.h"
#include "stream_task_base_synch.h"

extern const char libacestream_default_file_source_module_name_string[];

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
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_FileReaderH_T
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
  Stream_Module_FileReaderH_T (ISTREAM_T*, // stream handle
#else
  Stream_Module_FileReaderH_T (typename inherited::ISTREAM_T*,                                           // stream handle
#endif
                               bool = false,                                                             // auto-start ? (active mode only)
                               enum Stream_HeadModuleConcurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE, // concurrency mode
                               bool = true);                                                             // generate session messages ?
  virtual ~Stream_Module_FileReaderH_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                                    UserDataType>::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReaderH_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReaderH_T (const Stream_Module_FileReaderH_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReaderH_T& operator= (const Stream_Module_FileReaderH_T&))

  // helper methods
  virtual int svc (void);
  //bool putStatisticMessage (const StatisticContainerType&) const; // statistics info

  bool        isOpen_;
  ACE_FILE_IO stream_;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ControlMessageType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_FileReader_Reader_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 struct Stream_ModuleHandlerConfiguration,
                                 ControlMessageType,
                                 ACE_Message_Block,
                                 ACE_Message_Block,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 struct Stream_ModuleHandlerConfiguration,
                                 ControlMessageType,
                                 ACE_Message_Block,
                                 ACE_Message_Block,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_FileReader_Reader_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_FileReader_Reader_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  inline virtual ~Stream_Module_FileReader_Reader_T () {}

  // override some task-based members
  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReader_Reader_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReader_Reader_T (const Stream_Module_FileReader_Reader_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReader_Reader_T& operator= (const Stream_Module_FileReader_Reader_T&))
};

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_FileReader_Writer_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType>
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_FileReader_Writer_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_FileReader_Writer_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_FileReader_Writer_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleControlMessage (ControlMessageType&); // control message
  virtual void handleSessionMessage (SessionMessageType*&, // session message
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReader_Writer_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReader_Writer_T (const Stream_Module_FileReader_Writer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileReader_Writer_T& operator= (const Stream_Module_FileReader_Writer_T&))

  // helper methods
  virtual int svc (void);

  bool*         aborted_;
  ACE_FILE_Addr fileName_;
  bool          isOpen_;
  bool          passDownstream_; // pass messages downstream as well ?
  ACE_FILE_IO   stream_;
};

// include template definition
#include "stream_file_source.inl"

#endif
