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

#ifndef STREAM_MODULE_BASE_H
#define STREAM_MODULE_BASE_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"

#include "stream_imodule.h"

// forward declaration(s)
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Task;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Stream;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          ////////////////////////////////
          const char* ModuleName, // *TODO*: use a variadic character array
          typename NotificationType, // *NOTE*: implements Stream_INotify_T<enum Stream_SessionMessageType>
          typename ReaderTaskType,
          typename WriterTaskType>
class Stream_Module_Base_T
 : public ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType>
 , public Stream_IModule_T<SessionIdType,
                           SessionDataType,
                           SessionEventType,
                           ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           HandlerConfigurationType>
{
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> inherited;

 public:
  // convenient types
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef ConfigurationType CONFIGURATION_T;
  typedef Stream_IModule_T<SessionIdType,
                           SessionDataType,
                           SessionEventType,
                           ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           HandlerConfigurationType> IMODULE_T;

  virtual ~Stream_Module_Base_T ();

  // override ACE_Module members
  virtual void next (MODULE_T*); // downstream module handle

  // implement (part of) Stream_IModule_T
  // *IMPORTANT NOTE*: the default implementation simply forwards all module
  //                   events to the processing stream instance
  virtual void notify (SessionIdType,            // session id
                       const SessionEventType&); // event (state/status change, ...)
  inline virtual const STREAM_T& getR () const { ACE_ASSERT (stream_); return *stream_; }
  virtual bool initialize (const ConfigurationType&);
  inline virtual const ConfigurationType& getR_2 () const { ACE_ASSERT (configuration_); return *configuration_; }
  virtual const HandlerConfigurationType& getR_3 () const;
  virtual void reset ();

 protected:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;

  Stream_Module_Base_T (const std::string&, // name {empty: default}
                        TASK_T*,            // handle to writer task
                        TASK_T*,            // handle to reader task
                        bool = false);      // delete tasks in dtor ?

  ConfigurationType* configuration_;
  bool               isInitialized_;
  NotificationType*  notify_;
  STREAM_T*          stream_;

 private:
  // convenient types
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> THRU_TASK_T;
  typedef Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               ModuleName,
                               NotificationType, // *NOTE*: stream notification interface
                               ReaderTaskType,
                               WriterTaskType> OWN_TYPE_T;
  typedef Stream_IModuleHandler_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  HandlerConfigurationType> IMODULE_HANDLER_T;
  typedef Common_IGet_T<HandlerConfigurationType> IGET_T;

  // implement (part of) Stream_IModule
  inline virtual void start (SessionIdType, const SessionDataType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void end (SessionIdType) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) } // session id
  inline virtual void onLink (ACE_Module_Base*) {}
  inline virtual void onUnlink (ACE_Module_Base*) {}
  virtual MODULE_T* clone ();

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Base_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Base_T (const Stream_Module_Base_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Base_T& operator= (const Stream_Module_Base_T&))

  bool               delete_;
  TASK_T*            reader_;
  TASK_T*            writer_;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          ////////////////////////////////
          const char* ModuleName, // *TODO*: use a variadic character array
          typename NotificationType, // *NOTE*: (derived from) Stream_INotify_T<enum Stream_SessionMessageType>
          typename ReaderTaskType,
          typename WriterTaskType>
class Stream_Module_BaseA_T
 : public Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               ModuleName,
                               NotificationType,
                               ReaderTaskType,
                               WriterTaskType>
{
  typedef Stream_Module_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionIdType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               ModuleName,
                               NotificationType,
                               ReaderTaskType,
                               WriterTaskType> inherited;

 public:
  // convenient types
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;

  inline virtual ~Stream_Module_BaseA_T () {}

  // override ACE_Module members
  virtual MODULE_T* next (void); // return value: downstream module handle

 protected:
  Stream_Module_BaseA_T (const std::string&,          // name
                         typename inherited::TASK_T*, // handle to writer task
                         typename inherited::TASK_T*, // handle to reader task
                         bool = false);               // delete tasks in dtor ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_BaseA_T (const Stream_Module_BaseA_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_BaseA_T& operator= (const Stream_Module_BaseA_T&))

  // overwrite (part of) Stream_IModule
  virtual void onLink (ACE_Module_Base*);
  virtual void onUnlink (ACE_Module_Base*);
};

// include template definition
#include "stream_module_base.inl"

#endif
