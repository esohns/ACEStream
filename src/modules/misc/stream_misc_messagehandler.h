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

#ifndef STREAM_MISC_MESSAGEHANDLER_H
#define STREAM_MISC_MESSAGEHANDLER_H

#include <list>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iclone.h"
#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_isessionnotify.h"
#include "stream_task_base_synch.h"

#include "stream_misc_aggregator.h"

// forward declarations
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;

extern const char libacestream_default_misc_messagehandler_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType, // not (!) reference-counted
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_MessageHandler_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public Common_ISubscribe_T<Stream_ISessionDataNotify_T<SessionDataType,
                                                          enum Stream_SessionMessageType,
                                                          DataMessageType,
                                                          SessionMessageType> >
 // *IMPORTANT NOTE*: derived classes need to implement the cloning mechanism
 , public Common_IClone_T<ACE_Task<ACE_SYNCH_USE,
                                   TimePolicyType> >
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

 public:
  // convenient types
  typedef Stream_ISessionDataNotify_T<SessionDataType,
                                      enum Stream_SessionMessageType,
                                      DataMessageType,
                                      SessionMessageType> INOTIFY_T;
  typedef Common_ISubscribe_T<INOTIFY_T> ISUBSCRIBE_T;
  typedef std::list<INOTIFY_T*> SUBSCRIBERS_T;

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_MessageHandler_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_MessageHandler_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  inline virtual ~Stream_Module_MessageHandler_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL); // report cache usage ?

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ISubscribe_T
  virtual void subscribe (INOTIFY_T*);   // new subscriber
  virtual void unsubscribe (INOTIFY_T*); // existing subscriber

  // implement Stream_IModuleHandler_T
  virtual bool postClone (ACE_Module<ACE_SYNCH_USE,
                                     TimePolicyType>*, // handle to 'original'
                          bool = false);               // initialize from 'original' ?

  // implement Common_IClone_T
  virtual ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType>* clone () const;

 protected:
  // convenient types
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> STREAM_TASK_T;
 typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;

  // *IMPORTANT NOTE*: this must be 'recursive', so that callees may unsubscribe
  //                   from within the notification callbacks
  typename ACE_SYNCH_USE::RECURSIVE_MUTEX lock_;
  SUBSCRIBERS_T                           subscribers_;

 private:
  // convenient types
  typedef Stream_Module_MessageHandler_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_T (const Stream_Module_MessageHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_T& operator= (const Stream_Module_MessageHandler_T&))

  // helper types
  struct SUBSCRIBERS_IS_EQUAL_P
  {
    inline bool operator() (INOTIFY_T* first, INOTIFY_T* second) { return (first == second); }
  };
};

//////////////////////////////////////////

// *IMPORTANT NOTE*: this one uses an external subscriber list/lock
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType, // not (!) reference-counted
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_MessageHandler_2
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public Common_ISubscribe_T<Stream_ISessionDataNotify_T<SessionDataType,
                                                          enum Stream_SessionMessageType,
                                                          DataMessageType,
                                                          SessionMessageType> >
 // *IMPORTANT NOTE*: derived classes need to implement the cloning mechanism
 , public Common_IClone_T<ACE_Task<ACE_SYNCH_USE,
                                   TimePolicyType> >
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

 public:
  // convenient types
  typedef Stream_ISessionDataNotify_T<SessionDataType,
                                      enum Stream_SessionMessageType,
                                      DataMessageType,
                                      SessionMessageType> INOTIFY_T;
  typedef Common_ISubscribe_T<INOTIFY_T> ISUBSCRIBE_T;
  typedef std::list<INOTIFY_T*> SUBSCRIBERS_T;

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_MessageHandler_2 (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_MessageHandler_2 (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_MessageHandler_2 ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL); // report cache usage ?

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ISubscribe_T
  virtual void subscribe (INOTIFY_T*);   // new subscriber
  virtual void unsubscribe (INOTIFY_T*); // existing subscriber

  // implement Stream_IModuleHandler_T
  virtual bool postClone (ACE_Module<ACE_SYNCH_USE,
                                     TimePolicyType>*, // handle to 'original'
                          bool = false);               // initialize from 'original' ?

  // implement Common_IClone_T
  virtual ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType>* clone () const;

 protected:
  // convenient types
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> STREAM_TASK_T;
 typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;

  bool                                     delete_;
  // *IMPORTANT NOTE*: this must be 'recursive', so that callees may unsubscribe
  //                   from within the notification callbacks
  typename ACE_SYNCH_USE::RECURSIVE_MUTEX* lock_;
  SUBSCRIBERS_T*                           subscribers_;

 private:
  // convenient types
  typedef Stream_Module_MessageHandler_2<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_2 (const Stream_Module_MessageHandler_2&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandler_2& operator= (const Stream_Module_MessageHandler_2&))

  // helper types
  struct SUBSCRIBERS_IS_EQUAL_P
  {
    inline bool operator() (INOTIFY_T* first, INOTIFY_T* second) { return (first == second); }
  };
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType, // NOT reference counted-
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_MessageHandlerA_T
 : public Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType>
 , public Common_ISubscribe_T<Stream_ISessionDataNotify_T<SessionDataType,
                                                          enum Stream_SessionMessageType,
                                                          DataMessageType,
                                                          SessionMessageType> >
 // *IMPORTANT NOTE*: derived classes need to implement the cloning mechanism
 , public Common_IClone_T<ACE_Task<ACE_SYNCH_USE,
                                   TimePolicyType> >
{
  typedef Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> inherited;

 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef Stream_ISessionDataNotify_T<SessionDataType,
                                      enum Stream_SessionMessageType,
                                      DataMessageType,
                                      SessionMessageType> INOTIFY_T;
  typedef std::list<INOTIFY_T*> SUBSCRIBERS_T;
  typedef Stream_Module_Aggregator_ReaderTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> READER_T;

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_MessageHandlerA_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_MessageHandlerA_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Module_MessageHandlerA_T ();

  // override ACE_Task member(s)
  inline virtual TASK_T* next (void) { return NULL; };

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL); // report cache usage ?

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ISubscribe_T
  virtual void subscribe (INOTIFY_T*);   // new subscriber
  virtual void unsubscribe (INOTIFY_T*); // existing subscriber

  // implement Stream_IModuleHandler_T
  virtual bool postClone (ACE_Module<ACE_SYNCH_USE,
                                     TimePolicyType>*, // handle to 'original'
                          bool = false);               // initialize from 'original' ?

  // implement Common_IClone_T
  inline virtual ACE_Task<ACE_SYNCH_USE,
                          TimePolicyType>* clone () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) };

 protected:
  // convenient types
  typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;

  bool                                     delete_;

  SUBSCRIBERS_T*                           subscribers_;
  // *IMPORTANT NOTE*: this must be 'recursive', so that callees may unsubscribe
  //                   from within the notification callbacks
  typename ACE_SYNCH_USE::RECURSIVE_MUTEX* subscribersLock_;

 private:
  // convenient types
  typedef Stream_Module_MessageHandlerA_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandlerA_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandlerA_T (const Stream_Module_MessageHandlerA_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_MessageHandlerA_T& operator= (const Stream_Module_MessageHandlerA_T&))

  // helper types
  struct SUBSCRIBERS_IS_EQUAL_P
  {
    inline bool operator() (INOTIFY_T* first, INOTIFY_T* second) { return (first == second); }
  };
};

// include template definition
#include "stream_misc_messagehandler.inl"

#endif
