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

#include "ace/config-lite.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Thread.h"

#include "common_macros.h"
#include "common_tools.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename HandlerType,
          typename StreamType>
Stream_Input_Manager_T<ACE_SYNCH_USE,
                       ConfigurationType,
                       HandlerType,
                       StreamType>::Stream_Input_Manager_T ()
 : inherited ()
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Input_Manager_T::Stream_Input_Manager_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename HandlerType,
          typename StreamType>
bool
Stream_Input_Manager_T<ACE_SYNCH_USE,
                       ConfigurationType,
                       HandlerType,
                       StreamType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Input_Manager_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (unlikely (!stream_.initialize (*configuration_in.streamConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to initialize input stream, aborting\n")));
    return false;
  } // end IF

  return inherited::initialize (configuration_in);
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename HandlerType,
          typename StreamType>
bool
Stream_Input_Manager_T<ACE_SYNCH_USE,
                       ConfigurationType,
                       HandlerType,
                       StreamType>::start (ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Input_Manager_T::start"));

  stream_.start ();

  if (unlikely (!inherited::start (timeout_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Input_Manager_T::start(), aborting\n")));
    goto error;
  } // end IF

  return true;

error:
  stream_.stop (true); // wait ?

  return false;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename HandlerType,
          typename StreamType>
int
Stream_Input_Manager_T<ACE_SYNCH_USE,
                       ConfigurationType,
                       HandlerType,
                       StreamType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Input_Manager_T::close"));

  // *NOTE*: this method may be invoked
  //         - by an external thread closing down the active object
  //           (arg_in == 1 !)
  //         - by the worker thread which calls this after returning from svc()
  //           (arg_in == 0 !) --> in this case, this should be a NOP
  switch (arg_in)
  {
    case 0:
    { ACE_ASSERT (ACE_OS::thr_equal (ACE_Thread::self (), inherited::last_thread ()));
      break;
    }
    case 1:
    {
      // sanity check(s)
      if (unlikely (!inherited::configuration_))
        goto continue_2; // nothing to do
      if (unlikely (inherited::configuration_->manageEventDispatch))
      {
        if (unlikely (inherited::thr_count_ == 0))
          goto continue_; // nothing to do
        ACE_ASSERT (inherited::configuration_->eventDispatchState);
        Common_Tools::finalizeEventDispatch (*inherited::configuration_->eventDispatchState,
                                             false); // wait ?
      } // end IF

continue_:
      if (likely (inherited::handler_))
      { // *NOTE*: handler_ cleans itself up
        inherited::handler_->deregister (); inherited::handler_ = NULL;
      } // end IF

continue_2:
      stream_.stop (true); // wait ?

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument: %u, aborting\n"),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
}
