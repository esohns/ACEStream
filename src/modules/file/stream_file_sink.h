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

#ifndef STREAM_MODULE_FILEWRITER_H
#define STREAM_MODULE_FILEWRITER_H

#include "ace/FILE_Addr.h"
#include "ace/FILE_IO.h"
#include "ace/Global_Macros.h"

#include "common_ilock.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_task_base_asynch.h"
#include "stream_task_base_synch.h"

// forward declarations
class Stream_IAllocator;

extern const char libacestream_default_file_sink_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_FileWriter_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
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
                                  struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_FileWriter_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_FileWriter_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_FileWriter_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_T (const Stream_Module_FileWriter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_T& operator= (const Stream_Module_FileWriter_T&))

  bool          isOpen_;
  ACE_FILE_Addr path_;
  int           previousError_; // print (significant) errors message once only
  ACE_FILE_IO   stream_;
};

//////////////////////////////////////////

// *NOTE*: this is the synchronous version (i.e. writes to the file on-the-fly);
//         use e.g. if there is a dedicated processing thread for this modules'
//         particular branch
template <typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_FileWriter_2
 : public Stream_TaskBaseSynch_T</*ACE_NULL_SYNCH*/ACE_MT_SYNCH,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_MT_SYNCH>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T</*ACE_NULL_SYNCH*/ACE_MT_SYNCH,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_MT_SYNCH>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // convenient types
  typedef /*ACE_NULL_SYNCH*/ACE_MT_SYNCH SYNCH_T; // *TODO*: redundant; remove ASAP

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_FileWriter_2 (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_FileWriter_2 (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_FileWriter_2 ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_2 (const Stream_Module_FileWriter_2&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_2& operator= (const Stream_Module_FileWriter_2&))

  bool          isOpen_;
  ACE_FILE_Addr path_;
  int           previousError_; // print (significant) errors message once only
  ACE_FILE_IO   stream_;
};

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename TimePolicyType,
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
          typename StatisticHandlerType>
class Stream_Module_FileWriterH_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
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
                                      StatisticHandlerType,
                                      struct Stream_UserData>
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
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
                                      StatisticHandlerType,
                                      struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_FileWriterH_T (ISTREAM_T*,                     // stream handle
#else
  Stream_Module_FileWriterH_T (typename inherited::ISTREAM_T*, // stream handle
#endif
                               bool = false,                   // auto-start ?
                               bool = true);                   // generate session messages ?
  virtual ~Stream_Module_FileWriterH_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                    TimePolicyType,
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
                                    StatisticHandlerType,
                                    struct Stream_UserData>::initialize;

  // override (part of) Stream_IModuleHandler_T
  inline virtual const ConfigurationType& get () const { ACE_ASSERT (inherited::configuration_); return *inherited::configuration_; }
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriterH_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriterH_T (const Stream_Module_FileWriterH_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriterH_T& operator= (const Stream_Module_FileWriterH_T&))

  // helper methods
  bool putStatisticMessage (const StatisticContainerType&) const;

  bool          isOpen_;
  ACE_FILE_Addr path_;
  int           previousError_; // print (significant) errors message once only
  ACE_FILE_IO   stream_;
};

////////////////////////////////////////////////////////////////////////////////

// include template definition
#include "stream_file_sink.inl"

#endif
