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

#ifndef STREAM_ISTREAMCONTROL_H
#define STREAM_ISTREAMCONTROL_H

#include <deque>
#include <string>

#include "ace/Module.h"
#include "ace/Stream.h"
#include "ace/Synch_Traits.h"
#include "ace/Task_T.h"

#include "common_idumpstate.h"
#include "common_iget.h"

#include "stream_common.h"
#include "stream_ilayout.h"
#include "stream_ilock.h"
#include "stream_inotify.h"

class Stream_IStreamControlBase
{
 public:
  virtual void start () = 0;
  virtual void stop (bool = true,       // wait for completion ?
                     bool = true,       // recurse upstream (if any) ?
                     bool = false) = 0; // high priority ?

  virtual Stream_SessionId_t id () const = 0; // current session- : -1
  // *NOTE*: the semantic really is: is there activity on the stream (i.e. is
  //         there currently data being processed by active components)
  virtual bool isRunning () const = 0;

  // *NOTE*: signals asynchronous completion
  virtual void finished (bool = true) = 0; // recurse upstream (if any) ?

//  // *NOTE*: wait for all queued data to drain
//  virtual void idle (bool = false) = 0; // wait for upstream (if any) ?
  // *NOTE*: flush the pipeline, releasing any data
  // *IMPORTANT NOTE*: session messages are not flushed iff all asynchronous
  //                   modules implement Stream_IMessageQueue
  // *TODO*: this precondition should not be strictly necessary
  virtual unsigned int flush (bool = true,       // flush inbound data ?
                              bool = false,      // flush session messages ?
                              bool = false) = 0; // flush upstream (if any) ?

  // *TODO*: this currently waits for writer-side (!) data only
  virtual void idle (bool = true,            // wait forever ?
                     bool = true) const = 0; // recurse upstream (if any) ?
  // *NOTE*: wait for workers, and/or all queued data to drain
  virtual void wait (bool = true,             // wait for any worker thread(s) ?
                     bool = false,            // wait for upstream (if any) ?
                     bool = false) const = 0; // wait for downstream (if any) ?

  virtual void pause () = 0;
  virtual void rewind () = 0;
};

template <typename ControlType,
          typename StatusType,
          typename StateType>
class Stream_IStreamControlBase_T
 : public Stream_IStreamControlBase
// , public Common_IGet_T<StateType>
{
 public:
  // *NOTE*: enqeues a control message
  virtual void control (ControlType,       // control type
                        bool = false) = 0; // recurse upstream (if any) ?
  virtual StatusType status () const = 0;
  virtual const StateType& state () const = 0;
};

template <typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType>
class Stream_IStreamControl_T
 : public Stream_IStreamControlBase_T<ControlType,
                                      StatusType,
                                      StateType>
 , public Stream_INotify_T<NotificationType>
{};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
class Stream_IStream_T
 : public Stream_ILock_T<ACE_SYNCH_USE>
 , public Common_IGetR_T<ACE_Stream<ACE_SYNCH_USE, TimePolicyType> > // get stream handle
 , public Common_ISetP_T<ACE_Stream<ACE_SYNCH_USE, TimePolicyType> > // set upstream
 , public Common_IDumpState
{
 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;

  virtual std::string name () const = 0;
  virtual void name (const std::string&) = 0;

  // *WARNING*: this APIs is not thread-safe
  virtual const MODULE_T* find (const std::string&,      // module name
                                bool = false,            // sanitize module names ?
                                bool = false) const = 0; // recurse upstream (if any) ?

  virtual bool link (STREAM_T*) = 0; // upstream handle
  // *IMPORTANT NOTE*: must be invoked on 'downstream' (!) sub-stream(s)
  virtual void _unlink () = 0;

  // *WARNING*: these APIs are not thread-safe
  virtual STREAM_T* downstream () const = 0;
  virtual STREAM_T* upstream (bool = false) const = 0; // recurse (if any) ?

  //virtual TASK_T* tail () const = 0;
};

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
class Stream_IStreamLayout_T
 : public Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType>
{
 public:
  // *IMPORTANT NOTE*: access to the module list happens in lockstep, i.e.
  //                   derived classes need not synchronize this, and should not
  //                   block in this method
  virtual bool load (Stream_ILayout*, // layout handle
                     bool&) = 0;      // return value: delete modules ?
};

#endif
