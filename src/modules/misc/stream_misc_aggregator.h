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

#ifndef STREAM_MISC_AGGREGATOR_H
#define STREAM_MISC_AGGREGATOR_H

#include <map>
#include <string>
#include <vector>

#include "ace/Global_Macros.h"
#include "ace/Module.h"
#include "ace/Synch_Traits.h"
#include "ace/Task_T.h"
#include "ace/Stream.h"

#include "common_ilock.h"

#include "stream_common.h"
#include "stream_ilink.h"
#include "stream_task_base_synch.h"

// forward declarations
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
class Stream_Module_Aggregator_WriterTask_T;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionIdType,
          typename SessionDataType>
class Stream_Module_Aggregator_ReaderTask_T
 : public ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType>
{
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> inherited;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;

  Stream_Module_Aggregator_ReaderTask_T (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_Aggregator_ReaderTask_T () {};

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  // convenient types
  typedef Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType,
                                                SessionIdType,
                                                SessionDataType> WRITER_TASK_T;
  typedef DataMessageType MESSAGE_T;
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_T (const Stream_Module_Aggregator_ReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_T& operator= (const Stream_Module_Aggregator_ReaderTask_T&))
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
          typename SessionIdType,
          typename SessionDataType>
class Stream_Module_Aggregator_WriterTask_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionIdType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_IModuleLinkCB
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionIdType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

  friend class Stream_Module_Aggregator_ReaderTask_T<ACE_SYNCH_USE,
                                                     TimePolicyType,
                                                     ConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType,
                                                     SessionIdType,
                                                     SessionDataType>;

 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef Stream_Module_Aggregator_ReaderTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType,
                                                SessionIdType,
                                                SessionDataType> READER_TASK_T;

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Aggregator_WriterTask_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_Aggregator_WriterTask_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_Aggregator_WriterTask_T ();

  // override ACE_Task_Base members
  virtual int put (ACE_Message_Block*,      // data chunk
                   ACE_Time_Value* = NULL); // timeout value

  // override ACE_Task members
  virtual TASK_T* next (void);

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,   // configuration handle
                           Stream_IAllocator* = NULL); // allocator handle

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // convenient types
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef ACE_Stream_Iterator<ACE_SYNCH_USE,
                              TimePolicyType> STREAM_ITERATOR_T;
  // *NOTE*: key: stream name, value: upstream predecessor/downstream successor
  typedef std::map<std::string, MODULE_T*> LINKS_T;
  typedef typename LINKS_T::const_iterator LINKS_ITERATOR_T;
  typedef std::map<Stream_SessionId_t,
                   typename inherited::TASK_BASE_T::ISTREAM_T*> SESSIONID_TO_STREAM_MAP_T;
  typedef typename SESSIONID_TO_STREAM_MAP_T::iterator SESSIONID_TO_STREAM_MAP_ITERATOR_T;
  typedef std::pair<Stream_SessionId_t, STREAM_T*> SESSIONID_TO_STREAM_PAIR_T;
  struct SESSIONID_TO_STREAM_MAP_FIND_S
   : public std::binary_function<SESSIONID_TO_STREAM_PAIR_T,
                                 typename inherited::TASK_BASE_T::ISTREAM_T*,
                                 bool>
  {
    inline bool operator() (const SESSIONID_TO_STREAM_PAIR_T& entry_in, typename inherited::TASK_BASE_T::ISTREAM_T* stream_in) const { return !ACE_OS::strcmp (entry_in.second->name (), stream_in->name ()); }
  };
  typedef std::map<SessionIdType,
                   typename SessionMessageType::DATA_T*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;

  ACE_SYNCH_MUTEX_T         lock_;
  LINKS_T                   readerLinks_;
  LINKS_T                   writerLinks_;
  SESSION_DATA_T            sessionData_;
  SESSIONID_TO_STREAM_MAP_T sessions_;

  std::string               outboundStreamName_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_WriterTask_T (const Stream_Module_Aggregator_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_WriterTask_T& operator= (const Stream_Module_Aggregator_WriterTask_T&))

  // override (part of) Stream_TaskBase_T
  virtual void handleMessage (ACE_Message_Block*, // message handle
                              bool&);             // return value: stop processing ?

  // implement Stream_IModuleLinkCB
  virtual void onLink (ACE_Module_Base*);
  virtual void onUnlink (ACE_Module_Base*);
};

// include template definition
#include "stream_misc_aggregator.inl"

#endif
