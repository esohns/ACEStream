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

#ifndef STREAM_MODULE_DUMP_H
#define STREAM_MODULE_DUMP_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_task_base_synch.h"

template <typename SynchStrategyType,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ///////////////////////////////
          typename SessionDataContainerType>
class Stream_Module_Dump_T
 : public Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 /////////
                                 ConfigurationType,
                                 /////////
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType>
 //, public Stream_IModuleHandler_T<ModuleHandlerConfigurationType>
{
 public:
  Stream_Module_Dump_T ();
  virtual ~Stream_Module_Dump_T ();

  //virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  //// implement Stream_IModuleHandler_T
  //virtual const ModuleHandlerConfigurationType& get () const;

 private:
  typedef Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 /////////
                                 ConfigurationType,
                                 /////////
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType> inherited;

  //typedef Stream_Module_Dump_T<SessionMessageType,
  //                             MessageType,
  //                             ///////////
  //                             ModuleHandlerConfigurationType,
  //                             ///////////
  //                             SessionDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Dump_T (const Stream_Module_Dump_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Dump_T& operator= (const Stream_Module_Dump_T&))
};

// include template defintion
#include "stream_misc_dump.inl"

#endif
