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

#ifndef STREAM_BASE_H
#define STREAM_BASE_H

#include <deque>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Containers_T.h"
#include "ace/Stream.h"

#include "common_idumpstate.h"

#include "stream_istreamcontrol.h"
#include "stream_streammodule_base.h"
#include "stream_headmoduletask_base.h"

// forward declaration(s)
class Stream_IAllocator;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionDataType,
          typename SessionMessageType,
          typename ProtocolMessageType>
class Stream_Base_T
 : public ACE_Stream<TaskSynchType,
                     TimePolicyType>
 , public Stream_IStreamControl
 , public Common_IDumpState
{
 public:
  // *NOTE*: this will try to sanely close down the stream:
  // 1: tell all worker threads to exit gracefully
  // 2: close() all modules which have not been enqueued onto the stream (next() == NULL)
  // 3: close() the stream (closes all enqueued modules: wait for queue to flush and threads, if any, to die)
  virtual ~Stream_Base_T ();

  // overload this from ACE_Stream to work as a hook to pass our messagecounter as argument to the modules
  // open() method...
  //virtual int push(ACE_Module<ACE_MT_SYNCH>*); // handle to module

  // implement Stream_IStreamControl
  // *NOTE*: delegate these calls to the head module which also implements that API...
  virtual void start ();
  virtual void stop (bool = true); // locked access ?
  virtual bool isRunning () const;
  virtual void pause ();
  virtual void rewind ();
  virtual void waitForCompletion ();

  // implement Common_IDumpState
  virtual void dump_state () const;

  bool isInitialized () const;

 protected:
  typedef ACE_Module<TaskSynchType,
                     TimePolicyType> Stream_Module_t;
  typedef ACE_Task<TaskSynchType,
                   TimePolicyType> Stream_Task_t;
  typedef Stream_IModule<TaskSynchType,
                         TimePolicyType> Stream_IModule_t;
  typedef ACE_Stream_Iterator<TaskSynchType,
                              TimePolicyType> Stream_StreamIterator_t;

  // *NOTE*: need to subclass this !
  Stream_Base_T ();

  // *NOTE*: children need to call this PRIOR to module RE-initialization
  // (i.e. in their own init()); this will:
  // - pop/close all push()ed modules, clean up default head/tail modules
  // - reset reader/writer tasks for ALL modules
  // - generate new default head/tail modules
  // *WARNING*: NEVER call this while isRunning() == true, otherwise you'll block
  // until the stream finishes (because close() of a module waits for its worker
  // thread to die...)
  bool reset ();

  // *NOTE*: children MUST call this in their dtor !
  void shutdown ();

  // *NOTE*: children need to set this IF their initialization succeeded; otherwise,
  // the dtor will NOT stop all worker threads before close()ing the modules...
  bool                        isInitialized_;

  // *NOTE*: children need to add handles to ALL of their modules to this container !
//   MODULE_CONTAINER_TYPE myAvailableModules;
  ACE_DLList<Stream_Module_t> availableModules_;

  // *NOTE*: children need to set this during THEIR initialization !
  Stream_IAllocator*          allocator_;

 private:
  typedef ACE_Stream<TaskSynchType,
                     TimePolicyType> inherited;
  typedef Stream_HeadModuleTaskBase_T<TaskSynchType,
                                      TimePolicyType,
                                      DataType,
                                      SessionDataType,
                                      SessionMessageType,
                                      ProtocolMessageType> Stream_HeadModuleTask_t;

  // convenient types
  typedef std::deque<Stream_Module_t*> Stream_Modules_t;
  typedef typename Stream_Modules_t::const_iterator Stream_ModulesIterator_t;
  typedef Stream_Base_T<TaskSynchType,
                        TimePolicyType,
                        DataType,
                        SessionDataType,
                        SessionMessageType,
                        ProtocolMessageType> own_type;

//   ACE_UNIMPLEMENTED_FUNC (Stream_Base_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_Base_T (const Stream_Base_T&));
  // *TODO*: apparently, ACE_UNIMPLEMENTED_FUNC gets confused by template arguments...
//   ACE_UNIMPLEMENTED_FUNC (Stream_Base_T& operator= (const Stream_Base_T&));

  // helper methods
  // wrap inherited::open/close() calls
  bool init ();
  bool fini ();
  void deactivateModules ();
};

// include template implementation
#include "stream_base.inl"

#endif
