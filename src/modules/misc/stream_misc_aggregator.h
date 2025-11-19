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

#include "stream_common.h"
#include "stream_ilink.h"
#include "stream_task_base_asynch.h"
#include "stream_task_base_synch.h"

// forward declarations
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Aggregator_WriterTask_T;
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Aggregator_WriterTask_2;

extern const char libacestream_default_misc_aggregator_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
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
  inline virtual ~Stream_Module_Aggregator_ReaderTask_T () {}

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  // convenient types
  typedef Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> WRITER_TASK_T;
  typedef DataMessageType MESSAGE_T;
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_T (const Stream_Module_Aggregator_ReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_T& operator= (const Stream_Module_Aggregator_ReaderTask_T&))
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Aggregator_ReaderTask_2
 : public ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType>
{
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> inherited;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;

  Stream_Module_Aggregator_ReaderTask_2 (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_Aggregator_ReaderTask_2 () {}

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  // convenient types
  typedef Stream_Module_Aggregator_WriterTask_2<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> WRITER_TASK_T;
  typedef DataMessageType MESSAGE_T;
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_2 (const Stream_Module_Aggregator_ReaderTask_2&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_ReaderTask_2& operator= (const Stream_Module_Aggregator_ReaderTask_2&))
};

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Aggregator_WriterTask_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_IModuleLinkCB
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

  friend class Stream_Module_Aggregator_ReaderTask_T<ACE_SYNCH_USE,
                                                     TimePolicyType,
                                                     ConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType>;

 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef Stream_Module_Aggregator_ReaderTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> READER_TASK_T;

  Stream_Module_Aggregator_WriterTask_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Aggregator_WriterTask_T ();

  // override ACE_Task_Base member(s)
  virtual int put (ACE_Message_Block*,      // data chunk
                   ACE_Time_Value* = NULL); // timeout value
  // override ACE_Task member(s)
  virtual TASK_T* next (void);
  virtual int module_closed (void);

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,   // configuration handle
                           Stream_IAllocator* = NULL); // allocator handle

  // implement (part of) Stream_ITaskBase_T
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
  // *NOTE*: key: stream, value: upstream predecessor/downstream successor
  typedef std::map<STREAM_T*, MODULE_T*> LINKS_T;
  typedef typename LINKS_T::const_iterator LINKS_ITERATOR_T;

  typedef std::map<Stream_SessionId_t,
                   STREAM_T*> SESSIONID_TO_STREAM_MAP_T;
  typedef typename SESSIONID_TO_STREAM_MAP_T::iterator SESSIONID_TO_STREAM_MAP_ITERATOR_T;
  typedef std::pair<Stream_SessionId_t, STREAM_T*> SESSIONID_TO_STREAM_PAIR_T;
  struct SESSIONID_TO_STREAM_MAP_FIND_S
   //: public std::binary_function<SESSIONID_TO_STREAM_PAIR_T,
   //                              STREAM_T*,
   //                              bool>
  {
    typedef SESSIONID_TO_STREAM_PAIR_T first_argument_type;
    typedef STREAM_T*                  second_argument_type;
    typedef bool                       result_type;

    inline bool operator () (const SESSIONID_TO_STREAM_PAIR_T& entry_in, STREAM_T* stream_in) const { return (entry_in.second == stream_in); }
  };

  typedef std::map<Stream_SessionId_t,
                   typename inherited::TASK_T*> SESSIONID_TO_TAIL_MAP_T;
  typedef typename SESSIONID_TO_TAIL_MAP_T::iterator SESSIONID_TO_TAIL_MAP_ITERATOR_T;

  typedef std::map<Stream_SessionId_t,
                   typename SessionMessageType::DATA_T*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;

  LINKS_T                   readerLinks_;
  LINKS_T                   writerLinks_;
  ACE_SYNCH_MUTEX_T         sessionLock_;
  SESSION_DATA_T            sessionSessionData_;
  SESSIONID_TO_STREAM_MAP_T sessions_;
  SESSIONID_TO_TAIL_MAP_T   tails_;

 private:
  // convenient types
  typedef Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> OWN_TYPE_T;

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

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Aggregator_WriterTask_2
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Stream_IModuleLinkCB
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData> inherited;

  friend class Stream_Module_Aggregator_ReaderTask_2<ACE_SYNCH_USE,
                                                     TimePolicyType,
                                                     ConfigurationType,
                                                     ControlMessageType,
                                                     DataMessageType,
                                                     SessionMessageType>;

 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef Stream_Module_Aggregator_ReaderTask_2<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> READER_TASK_T;

  Stream_Module_Aggregator_WriterTask_2 (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Aggregator_WriterTask_2 ();

  // override ACE_Task_Base member(s)
  virtual int open (void* = NULL);
  // override ACE_Task member(s)
  virtual TASK_T* next (void);
  virtual int module_closed (void);

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,   // configuration handle
                           Stream_IAllocator* = NULL); // allocator handle

  // implement (part of) Stream_ITaskBase_T
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
  // *NOTE*: key: stream, value: upstream predecessor/downstream successor
  typedef std::map<STREAM_T*, MODULE_T*> LINKS_T;
  typedef typename LINKS_T::const_iterator LINKS_ITERATOR_T;
  
  typedef std::map<Stream_SessionId_t,
                   STREAM_T*> SESSIONID_TO_STREAM_MAP_T;
  typedef typename SESSIONID_TO_STREAM_MAP_T::iterator SESSIONID_TO_STREAM_MAP_ITERATOR_T;
  typedef std::pair<Stream_SessionId_t, STREAM_T*> SESSIONID_TO_STREAM_PAIR_T;
  struct SESSIONID_TO_STREAM_MAP_FIND_S
   //: public std::binary_function<SESSIONID_TO_STREAM_PAIR_T,
   //                              STREAM_T*,
   //                              bool>
  {
    typedef SESSIONID_TO_STREAM_PAIR_T                  first_argument_type;
    typedef typename inherited::TASK_BASE_T::ISTREAM_T* second_argument_type;
    typedef bool                                        result_type;

    inline bool operator() (const SESSIONID_TO_STREAM_PAIR_T& entry_in, STREAM_T* stream_in) const { return (entry_in.second == stream_in); }
  };

  typedef std::map<Stream_SessionId_t,
                   typename inherited::TASK_T*> SESSIONID_TO_TAIL_MAP_T;
  typedef typename SESSIONID_TO_TAIL_MAP_T::iterator SESSIONID_TO_TAIL_MAP_ITERATOR_T;

  typedef std::map<Stream_SessionId_t,
                   typename SessionMessageType::DATA_T*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;

  // helper methods
  virtual void forward (ACE_Message_Block*,  // message block handle
                        Stream_SessionId_t); // session id

  LINKS_T                   readerLinks_;
  LINKS_T                   writerLinks_;
  ACE_SYNCH_MUTEX_T         sessionLock_;
  SESSION_DATA_T            sessionSessionData_;
  SESSIONID_TO_STREAM_MAP_T sessions_;
  SESSIONID_TO_TAIL_MAP_T   tails_;

 private:
  // convenient types
  typedef Stream_Module_Aggregator_WriterTask_2<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_WriterTask_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_WriterTask_2 (const Stream_Module_Aggregator_WriterTask_2&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_WriterTask_2& operator= (const Stream_Module_Aggregator_WriterTask_2&))

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
