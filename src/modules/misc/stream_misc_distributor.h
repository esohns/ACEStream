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

#ifndef STREAM_MISC_DISTRIBUTOR_H
#define STREAM_MISC_DISTRIBUTOR_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ace/Global_Macros.h"
#include "ace/Module.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"

#include "stream_common.h"
#include "stream_ilink.h"
#include "stream_task_base_asynch.h"

extern const char libacestream_default_misc_distributor_module_name_string[];

// forward declarations
class ACE_Message_Queue_Base;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType>
class Stream_Miscellaneous_Distributor_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Stream_IDistributorModule
 , public Stream_IModuleLinkCB
 , public Common_IGetP_2_T<ACE_Module<ACE_SYNCH_USE,
                                      TimePolicyType>*>
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Miscellaneous_Distributor_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Miscellaneous_Distributor_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  inline virtual ~Stream_Miscellaneous_Distributor_T () {}

  // override some ACE_Task_Base members
  inline virtual int open (void* = NULL) { return 0; }

  // override (part of) Common_ITask_T
  inline virtual void waitForIdleState () const { OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this); this_p->idle (); }

  // implement (part of) Stream_ITaskBase_T
  inline virtual void handleDataMessage (DataMessageType*& message_inout, bool& passMessageDownstream_out) { /*passMessageDownstream_out = true;*/ forward (message_inout, false); }
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IDistributorModule
  virtual bool initialize (const Stream_Branches_t&);
  virtual bool push (Stream_Module_t*);
  virtual bool pop (Stream_Module_t*);
  virtual Stream_Module_t* head (const std::string&) const;
  virtual std::string branch (Stream_Module_t*) const;
  virtual bool has (const std::string&,
                    unsigned int&) const;
  virtual Stream_ModuleList_t next ();

 protected:
  // convenient types
  typedef std::map<ACE_thread_t,
                   ACE_Message_Queue_Base*> THREAD_TO_QUEUE_MAP_T;
  typedef typename THREAD_TO_QUEUE_MAP_T::const_iterator THREAD_TO_QUEUE_ITERATOR_T;
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef std::map<ACE_Message_Queue_Base*,
                   MODULE_T*> QUEUE_TO_MODULE_MAP_T;
  typedef typename QUEUE_TO_MODULE_MAP_T::const_iterator QUEUE_TO_MODULE_ITERATOR_T;
  typedef std::map<std::string,
                   MODULE_T*> BRANCH_TO_HEAD_MAP_T;
  typedef typename BRANCH_TO_HEAD_MAP_T::iterator BRANCH_TO_HEAD_ITERATOR_T;
  typedef typename BRANCH_TO_HEAD_MAP_T::const_iterator BRANCH_TO_HEAD_CONST_ITERATOR_T;
  typedef std::pair<std::string, MODULE_T*> BRANCH_TO_HEAD_PAIR_T;
  struct BRANCH_TO_HEAD_MAP_FIND_S
   : public std::binary_function<BRANCH_TO_HEAD_PAIR_T,
                                 MODULE_T*,
                                 bool>
  {
    inline bool operator() (const BRANCH_TO_HEAD_PAIR_T& entry_in, MODULE_T* module_in) const { return !ACE_OS::strcmp (entry_in.second->name (), module_in->name ()); }
  };

  // *NOTE*: use Common_TaskBase_Ts' lock_
//  mutable ACE_SYNCH_MUTEX_T lock_;
  Stream_Branches_t         branches_;
  BRANCH_TO_HEAD_MAP_T      heads_;
  QUEUE_TO_MODULE_MAP_T     modules_;
  THREAD_TO_QUEUE_MAP_T     queues_;

 private:
  // convenient types
  typedef Stream_Miscellaneous_Distributor_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType> OWN_TYPE_T;
  typedef std::map<MODULE_T*,
                   SessionDataType*> HEAD_TO_SESSIONDATA_MAP_T;
  typedef typename HEAD_TO_SESSIONDATA_MAP_T::const_iterator HEAD_TO_SESSIONDATA_CONST_ITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_Distributor_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_Distributor_T (const Stream_Miscellaneous_Distributor_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_Distributor_T& operator= (const Stream_Miscellaneous_Distributor_T&))

  // helper methods
  virtual const MODULE_T* const getP_2 () const; // return value: head module handle
  void forward (ACE_Message_Block*, // message handle
                bool = false);      // dispose of original message ?

  // override ACE_Task_Base members
  virtual int svc (void);

  // override Common_ITaskControl_T members
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual void idle ();
  virtual void wait (bool = true) const; // wait for the message queue(s) ? : worker thread(s) only

  // implement Stream_IModuleLinkCB
  virtual void onLink (ACE_Module_Base*);
  virtual void onUnlink (ACE_Module_Base*);

  HEAD_TO_SESSIONDATA_MAP_T data_; // branch session-
};

// include template definition
#include "stream_misc_distributor.inl"

#endif
