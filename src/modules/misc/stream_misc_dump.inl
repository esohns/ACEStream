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

#include "stream_macros.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_Dump_T<SessionMessageType,
                     MessageType,
                     ModuleHandlerConfigurationType,
                     SessionDataType>::Stream_Module_Dump_T ()
 : inherited ()
 , configuration_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Dump_T::Stream_Module_Dump_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_Dump_T<SessionMessageType,
                               MessageType,
                               ModuleHandlerConfigurationType,
                               SessionDataType>::~Stream_Module_Dump_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Dump_T::~Stream_Module_Dump_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
void
Stream_Module_Dump_T<SessionMessageType,
                               MessageType,
                               ModuleHandlerConfigurationType,
                               SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Dump_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  message_inout->dump_state ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
void
Stream_Module_Dump_T<SessionMessageType,
                               MessageType,
                               ModuleHandlerConfigurationType,
                               SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Dump_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      //// *TODO*: remove type inferences
      //const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
      //  message_inout->get ();
      //const SessionDataType& session_data_r = session_data_container_r.get ();

      break;
    }
    case STREAM_SESSION_END:
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
bool
Stream_Module_Dump_T<SessionMessageType,
                               MessageType,
                               ModuleHandlerConfigurationType,
                               SessionDataType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Dump_T::initialize"));

  configuration_ =
    &const_cast<ModuleHandlerConfigurationType&> (configuration_in);

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
const ModuleHandlerConfigurationType&
Stream_Module_Dump_T<SessionMessageType,
                               MessageType,
                               ModuleHandlerConfigurationType,
                               SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Dump_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
