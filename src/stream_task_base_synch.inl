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

#include "ace/Message_Block.h"
#include "ace/Time_Value.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionEventType>::Stream_TaskBaseSynch_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseSynch_T::Stream_TaskBaseSynch_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionEventType>::~Stream_TaskBaseSynch_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseSynch_T::~Stream_TaskBaseSynch_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
int
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                               ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseSynch_T::put"));

  ACE_UNUSED_ARG (timeout_in);

  // borrow the calling thread to do the work
  bool stop_processing = false;
  inherited::handleMessage (messageBlock_in,
                            stop_processing);

  //return (stop_processing ? -1 : 0);
  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
int
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionEventType>::open (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseSynch_T::open"));

  ACE_UNUSED_ARG (arg_in);

//   if (inherited::module ())
//   {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("module \"%s\" has no worker thread...\n"),
//                 ACE_TEXT (inherited::name ())));
//   } // end IF
//   else
//   {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("no worker thread\n")));
//   } // end ELSE

  // nothing to do
  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
int
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionEventType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseSynch_T::close"));

  ACE_UNUSED_ARG (arg_in);

  // *NOTE*: just a stub, there's nothing to do

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
int
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionEventType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseSynch_T::module_closed"));

  // *NOTE*: invoked by an external thread either from:
  //         - the ACE_Module dtor or
  //         - during explicit ACE_Module::close()
  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
void
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionEventType>::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseSynch_T::waitForIdleState"));

  // *NOTE*: just a stub, there's nothing to do
}
