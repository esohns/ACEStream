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

#ifndef STREAM_MODULE_NET_SOURCE_H
#define STREAM_MODULE_NET_SOURCE_H

#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>

//#include "common_iget.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_task_base_synch.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          ////////////////////////////////
          typename ConnectionManagerType>
class Stream_Module_Net_Source_Reader_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 ACE_Message_Block,
                                 ACE_Message_Block,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
  Stream_Module_Net_Source_Reader_T ();
  virtual ~Stream_Module_Net_Source_Reader_T ();

  // implement (part of) Stream_ITaskBase
  virtual void handleControlMessage (ACE_Message_Block&);

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 ACE_Message_Block,
                                 ACE_Message_Block,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Reader_T (const Stream_Module_Net_Source_Reader_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Reader_T& operator= (const Stream_Module_Net_Source_Reader_T&))

  // convenient types
  typedef Common_IGetP_T<typename ConnectionManagerType::CONNECTION_T> SIBLING_T;
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
          typename AddressType,
          typename ConnectionManagerType,
          typename ConnectorType>
class Stream_Module_Net_Source_Writer_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 //, public Common_IGetP_T<typename ConnectionManagerType::CONNECTION_T>
{
 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
  Stream_Module_Net_Source_Writer_T ();
  virtual ~Stream_Module_Net_Source_Writer_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  inline virtual void handleDataMessage (DataMessageType*&, // data message handle
                                         bool&) {};         // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  //// implement Common_IGetP_T
  //inline virtual const typename ConnectionManagerType::CONNECTION_T* const get () const { return connection_; };

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Writer_T (const Stream_Module_Net_Source_Writer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_Writer_T& operator= (const Stream_Module_Net_Source_Writer_T&))

  AddressType                                   address_;
  ConnectorType                                 connector_;
  typename ConnectionManagerType::CONNECTION_T* connection_;
  bool                                          isLinked_;
  bool                                          isOpen_;
  bool                                          isPassive_;
  Stream_IStream*                               stream_;
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
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename ConnectionManagerType,
          typename ConnectorType,
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
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType>
{
 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
  Stream_Module_Net_SourceH_T (ACE_SYNCH_MUTEX_T* = NULL, // lock handle (state machine)
                               bool = true,               // generate session messages ?
                               ///////////
                               bool = false);             // passive ?
  virtual ~Stream_Module_Net_SourceH_T ();

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
                                    UserDataType>::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // info
  bool isInitialized () const;

  // implement (part of) Stream_ITaskBase
  inline virtual void handleDataMessage (DataMessageType*&, // data message handle
                                         bool&) {};         // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
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
                                      UserDataType> inherited;

//  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_SourceH_T (const Stream_Module_Net_SourceH_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_SourceH_T& operator= (const Stream_Module_Net_SourceH_T&))

  // helper methods
//  virtual int svc (void);
  //ProtocolMessageType* allocateMessage (unsigned int); // (requested) size
  //bool putStatisticMessage (const StatisticContainerType&) const; // statistic info

  ACE_INET_Addr                                 address_;
  ConnectorType                                 connector_;
  typename ConnectionManagerType::CONNECTION_T* connection_;
  bool                                          isLinked_;
  bool                                          isOpen_;
  bool                                          isPassive_;
  Stream_IStream*                               stream_;
};

// include template definition
#include "stream_module_source.inl"

#endif
