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

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_misc_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
Stream_Miscellaneous_Input_Stream_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ControlType,
                                    NotificationType,
                                    StatusType,
                                    StateType,
                                    ConfigurationType,
                                    StatisticContainerType,
                                    StatisticHandlerType,
                                    HandlerConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    UserDataType>::Stream_Miscellaneous_Input_Stream_T ()
 : inherited ()
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Input_Stream_T::Stream_Miscellaneous_Input_Stream_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
Stream_Miscellaneous_Input_Stream_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ControlType,
                                    NotificationType,
                                    StatusType,
                                    StateType,
                                    ConfigurationType,
                                    StatisticContainerType,
                                    StatisticHandlerType,
                                    HandlerConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    UserDataType>::~Stream_Miscellaneous_Input_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Input_Stream_T::~Stream_Miscellaneous_Input_Stream_T"));

  // get the head ('queue'-) module
  MODULE_T* module_p = NULL;
  int result = inherited::top (module_p);
  if (unlikely ((result == -1) || !module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Stream::top(): \"%m\", continuing\n"),
                ACE_TEXT (libacestream_default_misc_input_stream_name_string)));
    goto continue_;
  } // end IF
  // reset the queue; it gets deallocated earlier than the task
  TASK_T* task_p = module_p->writer ();
  ACE_ASSERT (task_p);
  task_p->msg_queue (NULL);

continue_:
  inherited::shutdown ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
bool
Stream_Miscellaneous_Input_Stream_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ControlType,
                                    NotificationType,
                                    StatusType,
                                    StateType,
                                    ConfigurationType,
                                    StatisticContainerType,
                                    StatisticHandlerType,
                                    HandlerConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    UserDataType>::load (Stream_ILayout* layout_in,
                                                         bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_Input_Stream_T::load"));

  delete_out = true;

  // sanity check(s)
  ACE_ASSERT (layout_in);

  typename inherited::MODULE_T* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  SOURCE_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);

  return true;
}
