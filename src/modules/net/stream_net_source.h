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

#ifndef STREAM_NET_SOURCE_H
#define STREAM_NET_SOURCE_H

#include <vector>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_task_base_synch.h"

extern const char libacestream_default_net_source_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Net_Source_Reader_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
  Stream_Module_Net_Source_Reader_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_Net_Source_Reader_T () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleControlMessage (ControlMessageType&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Reader_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Reader_T (const Stream_Module_Net_Source_Reader_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Reader_T& operator= (const Stream_Module_Net_Source_Reader_T&))

  // convenient types
  //typedef Common_IGetP_T<typename ConnectionManagerType::CONNECTION_T> SIBLING_T;
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
          typename ConnectorType> // implements Net_IConnector
class Stream_Module_Net_Source_Writer_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
//, public Common_IGetP_T<typename ConnectorType::CONNECTION_T>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
   Stream_Module_Net_Source_Writer_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Net_Source_Writer_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  inline virtual void handleDataMessage (DataMessageType*&, bool&) {}
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef std::vector<ACE_HANDLE> HANDLES_T;
  typedef HANDLES_T::iterator HANDLESITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Writer_T (const Stream_Module_Net_Source_Writer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Writer_T& operator= (const Stream_Module_Net_Source_Writer_T&))

  ConnectorType                          connector_;
  typename ConnectorType::ICONNECTION_T* connection_;
  HANDLES_T                              handles_;
  bool                                   isOpen_;
  bool                                   isPassive_;
};

//////////////////////////////////////////

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
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename ConnectorType, // implements Net_IConnector
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_SourceH_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      typename SessionMessageType::DATA_T::DATA_T,
                                      typename SessionMessageType::DATA_T,
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
                                      typename SessionMessageType::DATA_T::DATA_T,
                                      typename SessionMessageType::DATA_T,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType> inherited;

 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
  Stream_Module_Net_SourceH_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Net_SourceH_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using inherited::initialize;
#endif // ACE_WIN32 || ACE_WIN64

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  inline virtual void handleDataMessage (DataMessageType*&, bool&) {}
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  // convenient types
  typedef std::vector<ACE_HANDLE> HANDLES_T;
  typedef HANDLES_T::iterator HANDLESITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_SourceH_T (const Stream_Module_Net_SourceH_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_SourceH_T& operator= (const Stream_Module_Net_SourceH_T&))

  ConnectorType                          connector_;
  typename ConnectorType::ICONNECTION_T* connection_;
  HANDLES_T                              handles_;
  bool                                   isOpen_;
  bool                                   isPassive_;
};

// include template definition
#include "stream_net_source.inl"

#endif
