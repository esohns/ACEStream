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

#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename UserDataType>
Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType,
                            UserDataType>::Stream_Module_QueueReader_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                        bool autoStart_in,
                                                                        bool generateSessionMessages_in)
 : inherited (lock_in,
              autoStart_in,
              generateSessionMessages_in)
 , queue_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::Stream_Module_QueueReader_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename UserDataType>
bool
Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::initialize"));

  if (inherited::isInitialized_)
  {
    queue_ = NULL;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename UserDataType>
int
Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType,
                            UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::svc"));

  // sanity check(s)
  ACE_ASSERT (queue_);

  // increment thread count
  ++inherited::thr_count;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  while (queue_->dequeue (message_block_p, NULL) != -1)
  { ACE_ASSERT (message_block_p);
    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      message_block_p->release (); message_block_p = NULL;
      result = 0;
      goto done;
    } // end IF

    result = inherited::put_next (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));
      message_block_p->release (); message_block_p = NULL;
      goto done;
    } // end IF
    message_block_p = NULL;
  } // end WHILE
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to ACE_Message_Queue_Base::dequeue(): \"%m\", aborting\n")));

  result = -1;

done:
  // signal the controller
  inherited::finished ();

  // decrement thread count
  --inherited::thr_count;

  return result;
}
