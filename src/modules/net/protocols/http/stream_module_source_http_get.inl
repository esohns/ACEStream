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

#include "stream_dec_common.h"

#include "net_connection_configuration.h"

#include "http_codes.h"
#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Net_Source_HTTP_Get_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                    SessionMessageType>::Stream_Module_Net_Source_HTTP_Get_T (ISTREAM_T* stream_in)
#else
                                    SessionMessageType>::Stream_Module_Net_Source_HTTP_Get_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , receivedBytes_ (0)
 , resentRequest_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::Stream_Module_Net_Source_HTTP_Get_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_Net_Source_HTTP_Get_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::initialize"));

  if (inherited::isInitialized_)
  {
  } // end IF

  receivedBytes_ = 0;
  resentRequest_ = false;

  // *TODO*: validate URI
  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Net_Source_HTTP_Get_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  struct HTTP_Record* record_p = NULL;
  bool delete_record = false;
  HTTP_HeadersIterator_t iterator;
  std::string uri_string, host_name_string;
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  passMessageDownstream_out = false;

  // *NOTE*: if the HTTP header was parsed upstream, there is no need to do it
  //         here
  if (message_inout->isInitialized ())
  {
    const typename DataMessageType::DATA_T& data_container_r =
        message_inout->getR ();
    typename DataMessageType::DATA_T::DATA_T& data_r =
        const_cast<typename DataMessageType::DATA_T::DATA_T&> (data_container_r.getR ());
    record_p = &data_r;
  } // end IF
  else
  {
    record_p = parse (*message_inout);
    delete_record = true;
  } // end IF
  if (!record_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve HTTP record, aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  switch (record_p->status)
  {
    case HTTP_Codes::HTTP_STATUS_OK:
    {
      receivedBytes_ += message_inout->total_length ();
      // step1: got all data ? --> close connection
      iterator =
          record_p->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
      if (iterator == record_p->headers.end ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: missing \"%s\" HTTP header, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
        goto error;
      } // end IF
      std::istringstream converter ((*iterator).second);
      unsigned int content_length = 0;
      converter >> content_length;
      if (inherited::configuration_->closeAfterReception &&
          (content_length == receivedBytes_))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received all content, closing connection\n"),
                    inherited::mod_->name ()));
        ACE_ASSERT (session_data_r.connection);
        session_data_r.connection->close ();
      } // end IF

      passMessageDownstream_out = true;

      break; // done
    }
    case HTTP_Codes::HTTP_STATUS_MULTIPLECHOICES:
    case HTTP_Codes::HTTP_STATUS_MOVEDPERMANENTLY:
    case HTTP_Codes::HTTP_STATUS_MOVEDTEMPORARILY:
    case HTTP_Codes::HTTP_STATUS_NOTMODIFIED:
    case HTTP_Codes::HTTP_STATUS_USEPROXY:
    case HTTP_Codes::HTTP_STATUS_SWITCHPROXY:
    case HTTP_Codes::HTTP_STATUS_TEMPORARYREDIRECT:
    case HTTP_Codes::HTTP_STATUS_PERMANENTREDIRECT:
    {
      std::string host_name_string_2;
      std::string uri_string_2;
      bool use_SSL = false, use_SSL_2 = false;

      // step1: redirected --> extract location
      iterator =
          record_p->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_LOCATION_STRING));
      if (iterator == record_p->headers.end ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: missing \"%s\" HTTP header, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (HTTP_PRT_HEADER_LOCATION_STRING)));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: \"%s\" has been redirected to \"%s\" (status was: %d)\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->URL.c_str ()),
                  ACE_TEXT ((*iterator).second.c_str ()),
                  record_p->status));

      // step2: send request ?
      // *IMPORTANT NOTE*: only auto-effectuate same-server/protocol redirects
      if (!HTTP_Tools::parseURL ((*iterator).second,
                                 host_name_string,
                                 uri_string,
                                 use_SSL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT ((*iterator).second.c_str ())));
        goto error;
      } // end IF
      if (!HTTP_Tools::parseURL (inherited::configuration_->URL,
                                 host_name_string_2,
                                 uri_string_2,
                                 use_SSL_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT ((*iterator).second.c_str ())));
        goto error;
      } // end IF
      if (likely ((host_name_string != host_name_string_2) ||
                  (use_SSL != use_SSL_2)))
      { // *TODO*
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: URL (was: \"%s\") redirects to a different host, and/or requires a HTTP(S) connection, cannot proceed\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->URL.c_str ())));

        passMessageDownstream_out = false;

        goto error;
      } // end IF

      if (!send ((*iterator).second,
                 inherited::configuration_->HTTPHeaders,
                 inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT ((*iterator).second.c_str ())));
        goto error;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid HTTP response (status was: %d): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  record_p->status,
                  ACE_TEXT (record_p->reason.c_str ())));
      goto error;
    }
  } // end SWITCH

  goto continue_;

error:
  this->notify (STREAM_SESSION_MESSAGE_ABORT);

continue_:
  if (delete_record)
  { ACE_ASSERT (record_p);
    delete record_p;
  } // end IF

  if (!passMessageDownstream_out)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Net_Source_HTTP_Get_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      // send HTTP request ?
      if (inherited::configuration_->waitForConnect)
        break;
      if (!send (inherited::configuration_->URL,
                 inherited::configuration_->HTTPHeaders,
                 inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->URL.c_str ())));
        goto error;
      } // end IF
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started HTTP request for \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->URL.c_str ())));
#endif // _DEBUG
      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      // send HTTP request ?
      if (!inherited::configuration_->waitForConnect)
        break;
      if (!send (inherited::configuration_->URL,
                 inherited::configuration_->HTTPHeaders,
                 inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->URL.c_str ())));
        goto error_2;
      } // end IF
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started HTTP request for \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->URL.c_str ())));
#endif // _DEBUG
      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
DataMessageType*
Stream_Module_Net_Source_HTTP_Get_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::makeRequest (const std::string& URI_in,
                                                                      const HTTP_Headers_t& headers_in,
                                                                      const HTTP_Form_t& form_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::makeRequest"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->connectionConfigurations);

  Net_ConnectionConfigurationsIterator_t iterator =
    inherited::configuration_->connectionConfigurations->find (inherited::mod_->name ());
  if (iterator == inherited::configuration_->connectionConfigurations->end ())
    iterator =
      inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: applying dedicated connection configuration\n"),
                inherited::mod_->name ()));
#endif // _DEBUG
  ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());

  // step1: allocate message
  typename DataMessageType::DATA_T::DATA_T* message_data_p = NULL;
  ACE_NEW_NORETURN (message_data_p,
                    typename DataMessageType::DATA_T::DATA_T ());
  if (!message_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    return NULL;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  typename DataMessageType::DATA_T* message_data_container_p = NULL;
  ACE_NEW_NORETURN (message_data_container_p,
                    typename DataMessageType::DATA_T (message_data_p,
                                                      true));
  if (!message_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    return NULL;
  } // end IF
  // *TODO*: remove type inference
  DataMessageType* message_out =
    inherited::allocateMessage ((*iterator).second->allocatorConfiguration->defaultBufferSize);
  if (!message_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                (*iterator).second->allocatorConfiguration->defaultBufferSize));
    return NULL;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  message_out->initialize (message_data_container_p,
                           message_out->sessionId (),
                           NULL);

  // step2: populate HTTP GET request
  const typename DataMessageType::DATA_T& message_data_container_r =
      message_out->getR ();
  typename DataMessageType::DATA_T::DATA_T& message_data_r =
      const_cast<typename DataMessageType::DATA_T::DATA_T&> (message_data_container_r.getR ());
  message_data_r.form = form_in;
  message_data_r.headers = headers_in;
  message_data_r.method =
    (form_in.empty () ? HTTP_Codes::HTTP_METHOD_GET
                      : HTTP_Codes::HTTP_METHOD_POST);
  message_data_r.URI = URI_in;
  message_data_r.version = HTTP_Codes::HTTP_VERSION_1_1;

  return message_out;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_Net_Source_HTTP_Get_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::send (const std::string& URL_in,
                                                               const HTTP_Headers_t& headers_in,
                                                               const HTTP_Form_t& form_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::send"));

  int result = -1;
  std::string hostname_string, URI_string;
  bool use_SSL = false;
  if (!HTTP_Tools::parseURL (URL_in,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (URL_in.c_str ())));
    return false;
  } // end IF

  HTTP_Headers_t headers = headers_in;
  HTTP_HeadersConstIterator_t iterator =
    headers_in.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING));
  if (iterator == headers_in.end ())
    headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                    hostname_string));
  else
  {
    ACE_ASSERT ((*iterator).second == hostname_string);
  } // end ELSE

  // *TODO*: estimate a reasonable buffer size
  DataMessageType* message_p = makeRequest (URI_string,
                                            headers,
                                            form_in);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_Source_HTTP_Get_T::makeRequest(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (URI_string.c_str ())));
    return false;
  } // end IF

  // send data (fire-and-forget message_p)
  result = inherited::reply (message_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

    // clean up
    message_p->release ();

    return false;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: dispatched HTTP request (URI was: \"%s\")\n"),
  //            inherited::mod_->name (),
  //            ACE_TEXT (URI_in.c_str ())));

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Record*
Stream_Module_Net_Source_HTTP_Get_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::parse (DataMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_HTTP_Get_T::parse"));

  // initialize return value(s)
  HTTP_Record* result_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::configuration_);

  // (try to) retrieve status code
  std::stringstream converter;
  std::string line_string;
  std::istrstream input_stream (message_in.rd_ptr (),
                                message_in.length ());
//  std::istringstream input_stream (message_in.rd_ptr (),
//                                   message_in.length ());
  input_stream.get (*converter.rdbuf ());
  if (!input_stream)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to extract first line from HTTP response, aborting\n"),
                inherited::mod_->name ()));
    return NULL;
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
                ACE_TEXT ("%s: invalid HTTP response (first line was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (line_string.c_str ())));
    return NULL;
  } // end IF
//  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  ACE_ASSERT (!match_results.empty ());

  ACE_NEW_NORETURN (result_p,
                    HTTP_Record ());
  ACE_ASSERT (result_p);

  ACE_ASSERT (match_results[1].matched);
  std::string buffer = match_results[1].str ();
  result_p->version =
      HTTP_Tools::VersionToType (buffer.substr (ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX)),
                                                std::string::npos));

  ACE_ASSERT (match_results[2].matched);
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << match_results[2].str ();
  int status;
  converter >> status;
  result_p->status = static_cast<HTTP_Status_t> (status);

  ACE_ASSERT (match_results[3].matched);
  result_p->reason = match_results[3].str ();

  switch (result_p->status)
  {
    case HTTP_Codes::HTTP_STATUS_OK:
    {
      // skip over HTTP response entity head
      // *NOTE*: this cannot produce a valid entity for 'chunked' transfers
      //         (see: HTTP 1.1 rfc 2616), as the entity body is encoded...
      std::string doctype;

      regex_string = ACE_TEXT_ALWAYS_CHAR ("^(<!DOCTYPE html>)$");
      regex.assign (regex_string,
                    (std::regex_constants::ECMAScript |
                     std::regex_constants::icase));
      std::smatch match_results_2;

      std::string response_string (message_in.rd_ptr (),
                                   message_in.length ());
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
//        ACE_ASSERT (match_results_2.ready () && !match_results_2.empty ());
        ACE_ASSERT (!match_results_2.empty ());
        ACE_ASSERT (match_results_2[1].matched);
        break;
      } // end WHILE
      message_in.rd_ptr (offset);

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

      std::string response_string (message_in.rd_ptr (),
                                   message_in.length ());
      std::istringstream input_stream_2 (response_string);
      while (std::getline (input_stream_2, line_string))
      {
        if (!std::regex_match (line_string,
                               match_results_2,
                               regex,
                               std::regex_constants::match_default))
          continue;
//        ACE_ASSERT (match_results_2.ready () && !match_results_2.empty ());
        ACE_ASSERT (!match_results_2.empty ());
        ACE_ASSERT (match_results_2[2].matched);
        location = match_results_2[2].str ();
        break;
      } // end IF
      if (location.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid HTTP response (missing \"Location\"), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      // *TODO*: remove type inference
      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("%s: \"%s\" has been redirected to \"%s\" (status was: %d), resending HTTP request\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->URL.c_str ()), ACE_TEXT (location.c_str ()),
                  result_p->status));

      // step2: send request
      if (!send (location,
                 inherited::configuration_->HTTPHeaders,
                 inherited::configuration_->HTTPForm))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (location.c_str ())));
        break;
      } // end IF
      resentRequest_ = true;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid HTTP response (status was: %d), aborting\n"),
                  inherited::mod_->name (),
                  result_p->status));
      goto error;
    }
  } // end SWITCH

  return result_p;

error:
  delete result_p;

  return NULL;
}
