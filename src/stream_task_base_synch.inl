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

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionControlType,
                       SessionEventType,
                       UserDataType>::Stream_TaskBaseSynch_T (typename TASK_BASE_T::ISTREAM_T* stream_in)
 : inherited (stream_in)
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
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
int
Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       SessionIdType,
                       SessionControlType,
                       SessionEventType,
                       UserDataType>::put (ACE_Message_Block* messageBlock_in,
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
