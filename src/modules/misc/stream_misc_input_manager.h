/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef STREAM_INPUT_MANAGER_T_H
#define STREAM_INPUT_MANAGER_T_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"
#include "ace/Task_T.h"
#include "ace/Time_Value.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_iinitialize.h"

#include "common_input_manager.h"

#include "common_time_common.h"

#include "common_task_base.h"

#include "stream_misc_common.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType, // implements Stream_Input_Manager_Configuration_T
          typename HandlerType, // implements Common_InputHandler_Base_T
          ////////////////////////////////
          typename StreamType>
class Stream_Input_Manager_T
 : public Common_Input_Manager_T<ACE_SYNCH_USE,
                                 ConfigurationType,
                                 HandlerType>
 , public Common_IGetR_T<StreamType>
{
  typedef Common_Input_Manager_T<ACE_SYNCH_USE,
                                 ConfigurationType,
                                 HandlerType> inherited;

  // singleton requires access to the ctor/dtor
  friend class ACE_Singleton<Stream_Input_Manager_T<ACE_SYNCH_USE,
                                                    ConfigurationType,
                                                    HandlerType,
                                                    StreamType>,
                             ACE_SYNCH_MUTEX_T>;

 public:
  // convenient types
  typedef ACE_Singleton<Stream_Input_Manager_T<ACE_SYNCH_USE,
                                               ConfigurationType,
                                               HandlerType,
                                               StreamType>,
                        ACE_SYNCH_MUTEX_T> SINGLETON_T;

  // override (part of) Common_IAsynchTask
  virtual bool start (ACE_Time_Value* = NULL); // N/A

  // implement Common_IInitialize
  virtual bool initialize (const ConfigurationType&);

  // implement Common_IGetR_T
  inline virtual const StreamType& getR () const { return stream_; }

 private:
  Stream_Input_Manager_T ();
  inline virtual ~Stream_Input_Manager_T () {}
  ACE_UNIMPLEMENTED_FUNC (Stream_Input_Manager_T (const Stream_Input_Manager_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Input_Manager_T& operator= (const Stream_Input_Manager_T&))

  // override/hide some ACE_Task_Base member(s)
  virtual int close (u_long = 0);

  StreamType stream_;
};

// include template definition
#include "stream_misc_input_manager.inl"

#endif
