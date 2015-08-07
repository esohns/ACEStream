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
//#include "ace/Stream_Modules.h"
#include "ace/Synch_Traits.h"

#include "stream_module_base.h"

// forward declaration(s)
class Common_IRefCount;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Thru_Task;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename ReaderTaskType,
          typename WriterTaskType>
class Stream_StreamModule_T
 : public Stream_Module_Base_T<TaskSynchType,
                               TimePolicyType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               //////////
                               ReaderTaskType,
                               WriterTaskType>
{
 public:
  Stream_StreamModule_T (const std::string&, // name
                         Common_IRefCount*,  // object counter
                         bool);              // final module ?
  virtual ~Stream_StreamModule_T ();

 protected:
//  // define convenient types
//  typedef Stream_Module_Base_T<TaskSynchType,
//                               TimePolicyType,
//                               ConfigurationType,
//                               ReaderTaskType,
//                               WriterTaskType> STREAM_MODULE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T (const Stream_StreamModule_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModule_T& operator= (const Stream_StreamModule_T&))

 private:
  typedef Stream_Module_Base_T<TaskSynchType,
                               TimePolicyType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               //////////
                               ReaderTaskType,
                               WriterTaskType> inherited;

  ReaderTaskType reader_;
  WriterTaskType writer_;
};

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename TaskType>
class Stream_StreamModuleInputOnly_T
 : public Stream_StreamModule_T<TaskSynchType,
                                TimePolicyType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                /////////
                                ACE_Thru_Task<TaskSynchType,
                                              TimePolicyType>,
                                TaskType>
{
 public:
  Stream_StreamModuleInputOnly_T (const std::string&, // name
                                  Common_IRefCount*,  // object counter
                                  bool);              // final module ?
  virtual ~Stream_StreamModuleInputOnly_T ();

 protected:
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnly_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnly_T (const Stream_StreamModuleInputOnly_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StreamModuleInputOnly_T& operator= (const Stream_StreamModuleInputOnly_T&))

 private:
  typedef Stream_StreamModule_T<TaskSynchType,
                                TimePolicyType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                /////////
                                ACE_Thru_Task<TaskSynchType,
                                              TimePolicyType>,
                                TaskType> inherited;
};

// include template implementation
#include "stream_streammodule_base.inl"

// *NOTE*: use this macro to instantiate the module definitions
// *IMPORTANT NOTE*: TASK_SYNCH_TYPE is [ACE_MT_SYNCH | ACE_NULL_SYNCH] and MUST
// correspond to the actual TASK_TYPE declaration !
// *TODO*: --> remove TASK_TYPE
#define DATASTREAM_MODULE_INPUT_ONLY(TASK_SYNCH_TYPE,\
                                     TIME_POLICY_TYPE,\
                                     CONFIGURATION_TYPE,\
                                     HANDLER_CONFIGURATION_TYPE,\
                                     TASK_TYPE) typedef Stream_StreamModuleInputOnly_T<TASK_SYNCH_TYPE,\
                                                                                       TIME_POLICY_TYPE,\
                                                                                       CONFIGURATION_TYPE,\
                                                                                       HANDLER_CONFIGURATION_TYPE,\
                                                                                       TASK_TYPE> TASK_TYPE##_Module
#define DATASTREAM_MODULE_INPUT_ONLY_T(TASK_SYNCH_TYPE,\
                                       TIME_POLICY_TYPE,\
                                       CONFIGURATION_TYPE,\
                                       HANDLER_CONFIGURATION_TYPE,\
                                       TASK_TYPE,\
                                       NAME) typedef Stream_StreamModuleInputOnly_T<TASK_SYNCH_TYPE,\
                                                                                    TIME_POLICY_TYPE,\
                                                                                    CONFIGURATION_TYPE,\
                                                                                    HANDLER_CONFIGURATION_TYPE,\
                                                                                    TASK_TYPE> NAME##_Module
#define DATASTREAM_MODULE_DUPLEX(TASK_SYNCH_TYPE,\
                                 TIME_POLICY_TYPE,\
                                 CONFIGURATION_TYPE,\
                                 HANDLER_CONFIGURATION_TYPE,\
                                 READER_TYPE,\
                                 WRITER_TYPE,\
                                 NAME) typedef Stream_StreamModule_T<TASK_SYNCH_TYPE,\
                                                                     TIME_POLICY_TYPE,\
                                                                     CONFIGURATION_TYPE,\
                                                                     HANDLER_CONFIGURATION_TYPE,\
                                                                     READER_TYPE,\
                                                                     WRITER_TYPE> NAME##_Module

#endif
