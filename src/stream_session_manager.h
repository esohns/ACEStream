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

#ifndef STREAM_SESSION_MANAGER_T_H
#define STREAM_SESSION_MANAGER_T_H

#include "ace/Condition_Recursive_Thread_Mutex.h"
#include "ace/Containers_T.h"
#include "ace/Recursive_Thread_Mutex.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_icounter.h"
#include "common_iget.h"
#include "common_istatistic.h"

#include "common_timer_resetcounterhandler.h"

#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"

template <ACE_SYNCH_DECL,
          typename NotificationType, // session-
          typename ConfigurationType, // session-
          typename SessionDataType, // inherits Stream_SessionData
          typename StatisticContainerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Session_Manager_T
 : public Stream_IStreamControlBase
 , public Stream_IEvent_T<NotificationType>
 , public Stream_ISessionCB
 , public Common_ICounter
 , public Common_IGetR_T<SessionDataType>
{
  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Stream_Session_Manager_T<ACE_SYNCH_USE,
                                                      NotificationType,
                                                      ConfigurationType,
                                                      SessionDataType,
                                                      StatisticContainerType,
                                                      UserDataType>,
                             ACE_SYNCH_MUTEX_T>;

 public:
  // convenience types
  typedef ConfigurationType CONFIGURATION_T;
  typedef SessionDataType DATA_T;
  typedef UserDataType USERDATA_T;
  typedef ACE_Singleton<Stream_Session_Manager_T<ACE_SYNCH_USE,
                                                 NotificationType,
                                                 ConfigurationType,
                                                 SessionDataType,
                                                 StatisticContainerType,
                                                 UserDataType>,
                        ACE_SYNCH_MUTEX_T> SINGLETON_T;

  // configuration / initialization
  bool initialize (const ConfigurationType&); // configuration

  // implement (part of) Stream_IStreamControlBase
  virtual void start ();
  virtual void stop (bool = true,   // wait for completion ?
                     bool = true,   // recurse upstream (if any) ?
                     bool = false); // high priority ?
  inline virtual Stream_SessionId_t id () const { ACE_ASSERT (sessionData_); return sessionData_->sessionId; }
  inline virtual bool isRunning () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual void finished (bool = true) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  virtual unsigned int flush (bool = true,   // flush inbound data ?
                              bool = false,  // flush session messages ?
                              bool = false); // flush upstream (if any) ?
  virtual void idle (bool = true,        // wait forever ?
                     bool = true) const; // recurse upstream (if any) ?
  virtual void wait (bool = true,         // wait for any worker thread(s) ?
                     bool = false,        // wait for upstream (if any) ?
                     bool = false) const; // wait for downstream (if any) ?
  inline virtual void pause () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void rewind () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // implement Stream_ISessionCB
  inline virtual void onSessionBegin (Stream_SessionId_t) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void onSessionEnd (Stream_SessionId_t) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // implement Common_IGetR_T
  inline virtual const SessionDataType& getR () const { ACE_ASSERT (sessionData_); return *sessionData_; }

  // *WARNING*: this method is NOT (!) re-entrant
  virtual void set (const SessionDataType&); // session data

 protected:
  // *NOTE*: support derived classes
  Stream_Session_Manager_T ();
  virtual ~Stream_Session_Manager_T ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Session_Manager_T (const Stream_Session_Manager_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Session_Manager_T& operator= (const Stream_Session_Manager_T&))

  // convenient types
  typedef Stream_Session_Manager_T<ACE_SYNCH_USE,
                                   NotificationType,
                                   ConfigurationType,
                                   SessionDataType,
                                   StatisticContainerType,
                                   UserDataType> OWN_TYPE_T;

  // implement Stream_IEvent_T
  virtual void onEvent (NotificationType);

  // implement Common_ICounter
  // *NOTE*: visits each connection updating its statistic to support throughput
  //         measurement
  virtual void reset ();

  // timer
  Common_Timer_ResetCounterHandler             resetTimeoutHandler_;
  long                                         resetTimeoutHandlerId_;
  ACE_Time_Value                               resetTimeoutInterval_;

  ConfigurationType*                           configuration_;
  mutable ACE_SYNCH_MUTEX                      lock_;
  SessionDataType*                             sessionData_; // default-
};

// include template definition
#include "stream_session_manager.inl"

#endif
