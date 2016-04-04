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

#ifndef STREAM_MODULE_MESSAGEHANDLER_H
#define STREAM_MODULE_MESSAGEHANDLER_H

#include <list>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_inotify.h"
#include "common_isubscribe.h"
#include "common_iclone.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_task_base_synch.h"

template <typename SessionMessageType,
          typename MessageType,
          ///////////////////////////////
          typename ModuleHandlerConfigurationType,
          ///////////////////////////////
          typename SessionIdType,
          typename SessionDataType>
class Stream_Module_MessageHandler_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ModuleHandlerConfigurationType>
 , public Common_ISubscribe_T<Common_INotify_T<SessionIdType,
                                               typename SessionDataType::DATA_T,
                                               MessageType,
                                               SessionMessageType> >
 , public Common_IClone_T<Stream_Module_t>
{
 public:
  typedef Common_INotify_T<SessionIdType,
                           typename SessionDataType::DATA_T,
                           MessageType,
                           SessionMessageType> INOTIFY_T;
  typedef std::list<INOTIFY_T*> SUBSCRIBERS_T;

  Stream_Module_MessageHandler_T ();
  virtual ~Stream_Module_MessageHandler_T ();

  void initialize (SUBSCRIBERS_T* = NULL,              // subscribers (handle)
                   ACE_SYNCH_RECURSIVE_MUTEX* = NULL); // subscribers lock

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ModuleHandlerConfigurationType&);
  virtual const ModuleHandlerConfigurationType& get () const;

  // implement Common_ISubscribe_T
  virtual void subscribe (INOTIFY_T*);   // new subscriber
  virtual void unsubscribe (INOTIFY_T*); // existing subscriber

 protected:
  ModuleHandlerConfigurationType*   configuration_;

  bool                              delete_;
  // *NOTE*: recursive so that callees may unsubscribe from within the
  //         notification callbacks...
  ACE_SYNCH_RECURSIVE_MUTEX*        lock_;
  SUBSCRIBERS_T*                    subscribers_;

  typename SessionDataType::DATA_T* sessionData_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  typedef Stream_Module_MessageHandler_T<SessionIdType,
                                         SessionMessageType,
                                         MessageType,
                                         
                                         ModuleHandlerConfigurationType,
                                         
                                         SessionDataType> OWN_TYPE_T;
  typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_T (const Stream_Module_MessageHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_T& operator= (const Stream_Module_MessageHandler_T&))
};

// include template definition
#include "stream_misc_messagehandler.inl"

#endif
