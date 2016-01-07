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

#ifndef STREAM_MODULE_QUEUEREADER_H
#define STREAM_MODULE_QUEUEREADER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_istatistic.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

// forward declarations
class ACE_Message_Queue_Base;

template <typename LockType,                 // connection stream state machine lock
          ///////////////////////////////
          typename SessionMessageType,
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename StreamStateType,
          ///////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ///////////////////////////////
          typename StatisticContainerType>
class Stream_Module_QueueReader_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType>
 , public Common_IStatistic_T<StatisticContainerType>
{
 public:
  Stream_Module_QueueReader_T (bool = false,  // active object ?
                               bool = false); // auto-start ?
  virtual ~Stream_Module_QueueReader_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<LockType,
                                    ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    SessionMessageType,
                                    ProtocolMessageType,
                                    ConfigurationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType>::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // info
  bool isInitialized () const;
//  unsigned int getSessionID () const;

  //// implement (part of) Stream_ITaskBase
  //virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
  //                                bool&);                // return value: pass message downstream ?
  //virtual void handleSessionMessage (SessionMessageType*&, // session message handle
  //                                   bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistics collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType> inherited;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Module_QueueReader_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_QueueReader_T (const Stream_Module_QueueReader_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_QueueReader_T& operator= (const Stream_Module_QueueReader_T&))

  // helper methods
  virtual int svc (void);
//   Stream_Message* allocateMessage(const unsigned int&); // requested size
  bool putStatisticMessage (const StatisticContainerType&) const; // statistics info

  bool                              isInitialized_;
  ACE_Message_Queue_Base*           queue_;

  // timer
  Stream_StatisticHandler_Reactor_t statisticCollectionHandler_;
  long                              timerID_;
};

// include template implementation
#include "stream_misc_queue_source.inl"

#endif