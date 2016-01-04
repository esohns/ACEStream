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
#include "stream_session_message_base.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::Stream_Module_Vis_GTK_DrawingArea_T ()
 : inherited ()
 , configuration_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::Stream_Module_Vis_GTK_DrawingArea_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Vis_GTK_DrawingArea_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::~Stream_Module_Vis_GTK_DrawingArea_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleDataMessage (MessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::handleSessionMessage"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      sessionData_ =
        const_cast<SessionDataType*> (&session_data_container_r.get ());
      ACE_ASSERT (sessionData_);

      break;
    }
    case STREAM_SESSION_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::initialize"));

  configuration_ = configuration_in;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
const ModuleHandlerConfigurationType&
Stream_Module_Vis_GTK_DrawingArea_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_DrawingArea_T::get"));

  return configuration_;
}
