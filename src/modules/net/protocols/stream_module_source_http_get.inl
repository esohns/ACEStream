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

#include <regex>
#include <sstream>
#include <string>
#include <strstream>

#include "ace/Log_Msg.h"

#include "stream_iallocator.h"
#include "stream_macros.h"

#include "http_codes.h"
#include "http_defines.h"
#include "http_tools.h"

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::Stream_Module_Net_Source_HTTP_Get_T ()
 : inherited ()
 , configuration_ ()
 , headerReceived_ (false)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::Stream_Module_Net_Source_HTTP_Get_T"));

}

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::~Stream_Module_Net_Source_HTTP_Get_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::~Stream_Module_Net_Source_HTTP_Get_T"));

}

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::initialize"));

  configuration_ = configuration_in;
  headerReceived_ = false;
  // *TODO*: validate URI

  isInitialized_ = true;

  return true;
}

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (message_inout);
  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  if (headerReceived_)
    return; // done

  // (try to) retrieve status code
  std::stringstream converter;
  std::string line_string;
  std::istrstream input_stream (message_inout->rd_ptr (),
                                message_inout->length ());
//  std::istringstream input_stream (message_inout->rd_ptr (),
//                                   message_inout->length ());
  input_stream.get (*converter.rdbuf ());
  if (!input_stream)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to extract first line from HTTP response, returning\n")));
    return;
  } // end IF
  line_string = converter.str ();

  std::string regex_string =
      ACE_TEXT_ALWAYS_CHAR ("^([^[:space:]]+)(?:[[:space:]])([[:digit:]]{3})(?:[[:space:]])(.+)\r$");
  std::regex regex (regex_string);
  std::smatch match_results;
  if (!std::regex_match (line_string,
                         match_results,
                         regex,
                         std::regex_constants::match_default))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid HTTP response (first line was: \"%s\"), returning\n"),
                ACE_TEXT (line_string.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());

  ACE_ASSERT (match_results[2].matched);
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << match_results[2].str ();
  int http_response_status;
  converter >> http_response_status;
  switch (static_cast<HTTP_Status_t> (http_response_status))
  {
    case HTTP_Codes::HTTP_STATUS_OK:
    {
      // skip over response
      std::string doctype;

      regex_string = ACE_TEXT_ALWAYS_CHAR ("^(<!DOCTYPE html>)$");
      regex.assign (regex_string,
                    (std::regex_constants::ECMAScript |
                     std::regex_constants::icase));
      std::smatch match_results_2;

      std::string response_string (message_inout->rd_ptr (),
                                   message_inout->length ());
      std::istringstream input_stream (response_string);
      unsigned int offset = 0;
      while (std::getline (input_stream, line_string))
      {
        if (!std::regex_match (line_string,
                               match_results_2,
                               regex,
                               std::regex_constants::match_default))
        {
          offset += line_string.size () + 1; // don't forget the missing '\n'
          continue;
        } // end IF
        ACE_ASSERT (match_results_2.ready () && !match_results_2.empty ());
        ACE_ASSERT (match_results_2[1].matched);
        break;
      } // end WHILE
      message_inout->rd_ptr (offset);

      break;
    }
    case HTTP_Codes::HTTP_STATUS_MULTIPLECHOICES:
    case HTTP_Codes::HTTP_STATUS_MOVEDPERMANENTLY:
    case HTTP_Codes::HTTP_STATUS_MOVEDTEMPORARILY:
    case HTTP_Codes::HTTP_STATUS_NOTMODIFIED:
    {
      // step1: redirected --> extract location
      std::string location;

      regex_string = ACE_TEXT_ALWAYS_CHAR ("^(Location: )(.+?)(?:\r)$");
      regex.assign (regex_string);
      std::smatch match_results_2;

      std::string response_string (message_inout->rd_ptr (),
                                   message_inout->length ());
      std::istringstream input_stream_2 (response_string);
      while (std::getline (input_stream_2, line_string))
      {
        if (!std::regex_match (line_string,
                               match_results_2,
                               regex,
                               std::regex_constants::match_default))
          continue;
        ACE_ASSERT (match_results_2.ready () && !match_results_2.empty ());
        ACE_ASSERT (match_results_2[2].matched);
        location = match_results_2[2].str ();
        break;
      } // end IF
      if (location.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid HTTP response (missing \"Location\"), returning\n")));
        return;
      } // end IF
      // *TODO*: remove type inference
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("\"%s\" has been redirected to \"%s\" (status was: %d)\n"),
                  ACE_TEXT (configuration_.URL.c_str ()), ACE_TEXT (location.c_str ()),
                  http_response_status));

      // *TODO*: sending a (second) request here does not work...
//      // step2: send request
//      if (!sendRequest (location))
//      {
//        ACE_ASSERT (inherited::mod_);
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", returning\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (location.c_str ())));
//        return;
//      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid HTTP response (status was: %d), returning\n"),
                  http_response_status));
      return;
    }
  } // end SWITCH

  headerReceived_ = true;
}

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (inherited::mod_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      headerReceived_ = false;

      // send HTTP Get request
      if (!sendRequest (configuration_.URL))
      {
        ACE_ASSERT (inherited::mod_);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (configuration_.URL.c_str ())));
        return;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
ProtocolMessageType*
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::allocateMessage"));

  // initialize return value(s)
  ProtocolMessageType* message_out = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_.streamConfiguration);

  if (configuration_.streamConfiguration->messageAllocator)
  {
    try
    {
      // *TODO*: remove type inference
      message_out =
        static_cast<ProtocolMessageType*> (configuration_.streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_out = NULL;
    }
  } // end IF
  else
  {
    ACE_NEW_NORETURN (message_out,
                      ProtocolMessageType (requestedSize_in));
  } // end ELSE
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
}

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
ProtocolMessageType*
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::makeRequest (const std::string& URI_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::makeRequest"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_.socketHandlerConfiguration);

  // initialize return value(s)
  // *TODO*: remove type inference
  ProtocolMessageType* message_out =
    allocateMessage (configuration_.socketHandlerConfiguration->PDUSize);
  if (!message_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Net_Source_HTTP_Get_T::allocateMessage(%u), aborting\n"),
                configuration_.socketHandlerConfiguration->PDUSize));
    return NULL;
  } // end IF

  // step1: request line
  std::string buffer = HTTP_Tools::Method2String (HTTP_Codes::HTTP_METHOD_GET);
  buffer += ACE_TEXT_ALWAYS_CHAR (" ");
  buffer += URI_in;
  buffer += ACE_TEXT_ALWAYS_CHAR (" ");
  buffer += ACE_TEXT_ALWAYS_CHAR (HTTP_VERSION_STRING_PREFIX);
  buffer += HTTP_Tools::Version2String (HTTP_Codes::HTTP_VERSION_1_1);
  buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");
  // step2: separator
  buffer += ACE_TEXT_ALWAYS_CHAR ("\r\n");

  result = message_out->copy (buffer.c_str (),
                              buffer.size ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));

    // clean up
    message_out->release ();

    return NULL;
  } // end IF

  return message_out;
}

template <typename ConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_Module_Net_Source_HTTP_Get_T<ConfigurationType, 
                                    SessionMessageType,
                                    ProtocolMessageType>::sendRequest (const std::string& URI_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::sendRequest"));

  int result = -1;

  // initialize return value(s)
  // *TODO*: estimate a reasonable buffer size
  ProtocolMessageType* message_p = makeRequest (URI_in);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Net_Source_HTTP_Get_T::makeRequest(\"%s\"), aborting\n"),
                ACE_TEXT (URI_in.c_str ())));
    return false;
  } // end IF

  // send data
  result = inherited::reply (message_p);
  ACE_ASSERT (inherited::mod_);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

    // clean up
    message_p->release ();

    return false;
  } // end IF
  message_p = NULL;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: dispatched HTTP Get (URI was: \"%s\")\n"),
              inherited::mod_->name (),
              ACE_TEXT (URI_in.c_str ())));

  return true;
}
