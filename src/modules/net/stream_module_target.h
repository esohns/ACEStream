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

#include "common_time_common.h"

#include "stream_task_base_synch.h"

template <typename SynchStrategyType,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
class Stream_Module_Net_Target_T
 : public Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 /////////
                                 ConfigurationType,
                                 /////////
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType>
 //, public Stream_IModuleHandler_T<ConfigurationType>
{
 public:
  // *NOTE*: this module has two modes of operation:
  //         active:  establish and manage a connection
  //         passive: use an existing connection (handle passed in initialize())
  Stream_Module_Net_Target_T (bool = false); // passive ?
  virtual ~Stream_Module_Net_Target_T ();

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  //// implement Stream_IModuleHandler_T
  //virtual const ConfigurationType& get () const;

 protected:
  typename ConnectionManagerType::ICONNECTION_T* connection_;
  ConnectorType                                  connector_;
  SessionDataContainerType*                      sessionData_;

 private:
  typedef Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 /////////
                                 ConfigurationType,
                                 /////////
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Target_T (const Stream_Module_Net_Target_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Target_T& operator= (const Stream_Module_Net_Target_T&))

  // *NOTE*: facilitate asynchronous connects
  //typename ConnectorType::ICONNECTOR_T* iconnector_;
  bool                                           isInitialized_;
  bool                                           isLinked_;
  bool                                           isPassive_;
  // *NOTE*: this lock prevents races during (ordered) shutdown
  // *TODO*: remove surplus STREAM_SESSION_END messages
  ACE_SYNCH_MUTEX                                lock_;
};

// include template implementation
#include "stream_module_target.inl"

#endif
