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

#include "stream_net_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ListenerType,
          typename UserDataType>
Stream_Module_Net_Server_Stream_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  StreamName,
                                  ControlType,
                                  NotificationType,
                                  StatusType,
                                  StateType,
                                  ConfigurationType,
                                  StatisticContainerType,
                                  TimerManagerType,
                                  HandlerConfigurationType,
                                  SessionManagerType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ListenerType,
                                  UserDataType>::Stream_Module_Net_Server_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Server_Stream_T::Stream_Module_Net_Server_Stream_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName,
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename SessionManagerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ListenerType,
          typename UserDataType>
bool
Stream_Module_Net_Server_Stream_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  StreamName,
                                  ControlType,
                                  NotificationType,
                                  StatusType,
                                  StateType,
                                  ConfigurationType,
                                  StatisticContainerType,
                                  TimerManagerType,
                                  HandlerConfigurationType,
                                  SessionManagerType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ListenerType,
                                  UserDataType>::load (Stream_ILayout* layout_in,
                                                       bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Server_Stream_T::load"));

  // sanity check(s)
  ACE_ASSERT (layout_in);

  typename inherited::MODULE_T* module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  LISTENER_MODULE_T (this,
                                     ACE_TEXT_ALWAYS_CHAR (MODULE_NET_LISTENER_DEFAULT_NAME_STRING)),
                  false);
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                ACE_TEXT (inherited::name_.c_str ())));
    return false;
  } // end IF
  layout_in->append (module_p, NULL, 0);

  delete_out = true;

  return true;
}
