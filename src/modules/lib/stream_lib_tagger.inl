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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          enum Stream_MediaType_Type MediaType,
          typename UserDataType>
Stream_Module_Tagger_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       MediaType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       UserDataType>::Stream_Module_Tagger_T (ISTREAM_T* stream_in)
#else
                       UserDataType>::Stream_Module_Tagger_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tagger_T::Stream_Module_Tagger_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          enum Stream_MediaType_Type MediaType,
          typename UserDataType>
void
Stream_Module_Tagger_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       MediaType,
                       UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tagger_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  message_inout->setMediaType (MediaType);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          enum Stream_MediaType_Type MediaType,
          typename UserDataType>
void
Stream_Module_Tagger_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       MediaType,
                       UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Tagger_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  message_inout->setMediaType (MediaType);
}
