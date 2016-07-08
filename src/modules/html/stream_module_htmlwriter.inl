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

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_HTMLWriter_T<SynchStrategyType,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::Stream_Module_HTMLWriter_T ()
 : inherited ()
 , document_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLWriter_T::Stream_Module_HTMLWriter_T"));

}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_HTMLWriter_T<SynchStrategyType,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::~Stream_Module_HTMLWriter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLWriter_T::~Stream_Module_HTMLWriter_T"));

  // clean up
  if (document_)
    xmlFreeDoc (document_);
}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
void
Stream_Module_HTMLWriter_T<SynchStrategyType,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLWriter_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (configuration_);

  // *TODO*: remove type inferences
  const typename SessionMessageType::DATA_T& session_data_container_r =
    message_inout->get ();
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (session_data_container_r.get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (!document_);
      document_ = htmlNewDocNoDtD (NULL,  // URI
                                   NULL); // external ID
      if (!document_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to htmlNewDocNoDtD(), aborting\n"),
                    inherited::mod_->name ()));

        // *TODO*: remove type inference
        session_data_r.aborted = true;

        return;
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      if (!document_) break; // nothing to do

      result = htmlSaveFile (configuration_->targetFileName.c_str (),
                             document_);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to htmlSaveFile(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (configuration_->targetFileName.c_str ())));

        // *TODO*: remove type inference
        session_data_r.aborted = true;

        return;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: wrote \"%s\" (%d byte(s))\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_->targetFileName.c_str ()),
                  result));

      break;
    }
    default:
      break;
  } // end SWITCH
}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType>
//const ModuleHandlerConfigurationType&
//Stream_Module_HTMLWriter_T<SessionMessageType,
//                           MessageType,
//                           ModuleHandlerConfigurationType,
//                           SessionDataType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLWriter_T::get"));
//
//  // sanity check(s)
//  ACE_ASSERT (configuration_);
//
//  return *configuration_;
//}
template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
bool
Stream_Module_HTMLWriter_T<SynchStrategyType,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLWriter_T::initialize"));

  static bool is_first_run = true;

  if (is_first_run)
  {
    LIBXML_TEST_VERSION
    is_first_run = false;
  } // end IF

  if (document_)
  {
    xmlFreeDoc (document_);
    document_ = NULL;
  } // end IF

  return inherited::initialize (configuration_in);
  ;
}
