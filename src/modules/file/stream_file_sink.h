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

#include "ace/FILE_IO.h"
#include "ace/Global_Macros.h"

#include "common_istatistic.h"
#include "common_time_common.h"

#include "stream_headmoduletask_base.h"
#include "stream_statistichandler.h"
#include "stream_task_base_asynch.h"

#define STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILE "output.tmp"

template <typename SessionMessageType,
          typename MessageType,
          ///////////////////////////////
          typename ModuleHandlerConfigurationType,
          ///////////////////////////////
          typename SessionDataType>
class Stream_Module_FileWriter_T
 : public Stream_TaskBaseAsynch_T<Common_TimePolicy_t,
                                  SessionMessageType,
                                  MessageType>
 , public Stream_IModuleHandler_T<ModuleHandlerConfigurationType>
{
 public:
  Stream_Module_FileWriter_T ();
  virtual ~Stream_Module_FileWriter_T ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ModuleHandlerConfigurationType&);
  virtual const ModuleHandlerConfigurationType& get () const;

 protected:
  ModuleHandlerConfigurationType configuration_;

 private:
  typedef Stream_TaskBaseAsynch_T<Common_TimePolicy_t,
                                  SessionMessageType,
                                  MessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_T (const Stream_Module_FileWriter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriter_T& operator= (const Stream_Module_FileWriter_T&))

  bool        isOpen_;
  int         previousError_; // print (significant) errors message once only
  ACE_FILE_IO stream_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename LockType,
          ///////////////////////////////
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename StreamStateType,
          ///////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ///////////////////////////////
          typename StatisticContainerType>
class Stream_Module_FileWriterH_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      TaskSynchType,
                                      TimePolicyType,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType>
 // implement this to have a generic (timed) event handler to trigger
 // periodic statistic collection
 , public Common_IStatistic_T<StatisticContainerType>
{
 public:
  Stream_Module_FileWriterH_T ();
  virtual ~Stream_Module_FileWriterH_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<LockType,
                                    /////
                                    TaskSynchType,
                                    TimePolicyType,
                                    SessionMessageType,
                                    ProtocolMessageType,
                                    /////
                                    ConfigurationType,
                                    /////
                                    StreamStateType,
                                    /////
                                    SessionDataType,
                                    SessionDataContainerType>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual const ConfigurationType& get () const;
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      TaskSynchType,
                                      TimePolicyType,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriterH_T (const Stream_Module_FileWriterH_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_FileWriterH_T& operator= (const Stream_Module_FileWriterH_T&))

  // convenience types
  typedef Stream_StatisticHandler_Reactor_T<StatisticContainerType> STATISTICHANDLER_T;
  //typedef IRC_Client_SessionData SESSIONDATA_T;

  // helper methods
  bool putStatisticMessage (const StatisticContainerType&) const;

  // timer
  STATISTICHANDLER_T statisticCollectHandler_;
  long               statisticCollectHandlerID_;

  bool               isOpen_;
  int                previousError_; // print (significant) errors message once only
  ACE_FILE_IO        stream_;

  bool               initialized_;
};

////////////////////////////////////////////////////////////////////////////////

#include "stream_file_sink.inl"

#endif
