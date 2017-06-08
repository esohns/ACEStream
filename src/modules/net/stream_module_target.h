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

#ifndef STREAM_MODULE_NET_TARGET_H
#define STREAM_MODULE_NET_TARGET_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Stream.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_task_base_synch.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          ////////////////////////////////
          typename ConnectionConfigurationIteratorType, // (const-)
          typename ConnectionManagerType,
          typename ConnectorType>
class Stream_Module_Net_Target_T
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
{
 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
  Stream_Module_Net_Target_T (ISTREAM_T*,    // stream handle
                              bool = false); // passive ?
  virtual ~Stream_Module_Net_Target_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
//  inline virtual void handleDataMessage (DataMessageType*&, // data message handle
//                                         bool&) {};         // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  typename ConnectionManagerType::ICONNECTION_T*   connection_;
  ConnectorType                                    connector_;

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

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Target_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Target_T (const Stream_Module_Net_Target_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Target_T& operator= (const Stream_Module_Net_Target_T&))

  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef typename std::map<std::string,
                            ConfigurationType>::iterator CONFIGURATION_ITERATOR_T;

  typename ConnectorType::ADDRESS_T address_;
  bool                              isOpen_;
  bool                              isPassive_;
  // *NOTE*: this lock prevents races during shutdown
  ACE_SYNCH_MUTEX                   lock_;
  bool                              unlink_;
};

// include template definition
#include "stream_module_target.inl"

#endif
