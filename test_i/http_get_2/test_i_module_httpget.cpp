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
#include "stdafx.h"

#include "test_i_module_httpget.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

#include "test_i_connection_common.h"

#include "test_i_http_get_defines.h"

Test_I_Stream_HTTPGet::Test_I_Stream_HTTPGet (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , iterator_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTTPGet::Test_I_Stream_HTTPGet"));

}

void
Test_I_Stream_HTTPGet::handleDataMessage (Test_I_Stream_Message*& message_inout,
                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTTPGet::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // handle redirects
  inherited::received_ = false;
  inherited::handleDataMessage (message_inout,
                                passMessageDownstream_out);
  if (!inherited::received_)
  {
    // probable reason: re-sent request
    // --> wait for response
    return;
  } // end IF

  const Test_I_Stream_MessageData& message_data_container_r =
      message_inout->getR ();
  Test_I_MessageData& message_data_r =
    const_cast<Test_I_MessageData&> (message_data_container_r.getR ());
  message_data_r.stockItem = *iterator_;

  // send next request ?
  do
  {
    if (++iterator_ == inherited::configuration_->stockItems.end ())
    { // done --> close connection
      Test_I_HTTPGet_SessionData& session_data_r =
        const_cast<Test_I_HTTPGet_SessionData&> (inherited::sessionData_->getR ());
      ACE_ASSERT (session_data_r.connection);
      if (session_data_r.connection)
      {
        session_data_r.connection->close ();
        session_data_r.connection = NULL;
      } // end IF
      return; // done
    } // end IF

    if (!(*iterator_).ISIN.empty ())
      break;
  } while (true);

  std::string url_string = inherited::configuration_->URL;
  HTTP_Form_t form_data;
  form_data.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_FORM_KEY_SEARCH_STRING),
                                    (*iterator_).ISIN));
  if (form_data.empty ())
    makeURI (inherited::configuration_->URL,
             (*iterator_).ISIN,
             url_string);

  // send HTTP GET/POST request
  if (!inherited::send (url_string,
                        inherited::configuration_->HTTPHeaders,
                        form_data))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to send HTTP request \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->URL.c_str ())));
    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: fetching \"%s\"...\n"),
              inherited::mod_->name (),
              ACE_TEXT ((*iterator_).symbol.c_str ())));
}

void
Test_I_Stream_HTTPGet::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTTPGet::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      //ACE_ASSERT (!inherited::sessionData_);

      // *TODO*: remove type inferences
      inherited::sessionData_ =
        &const_cast<Test_I_HTTPGet_SessionData_t&> (message_inout->getR ());
      inherited::sessionData_->increase ();

      iterator_ = inherited::configuration_->stockItems.begin ();
      do
      {
        if (iterator_ == inherited::configuration_->stockItems.end ())
          return; // done

        if (!(*iterator_).ISIN.empty ())
          break;

        ++iterator_;
      } while (true);
      // sanity check(s)
      if (iterator_ == inherited::configuration_->stockItems.end ())
        return;

      const Test_I_HTTPGet_SessionData_t& sesion_data_container_r =
        message_inout->getR ();
      const Test_I_HTTPGet_SessionData& session_data_r =
        sesion_data_container_r.getR ();

      // sanity check(s)
      // *TODO*: remove type inferences
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        if (session_data_r.connectionStates.empty () ||
            ((*session_data_r.connectionStates.begin ()).second->status != NET_CONNECTION_STATUS_OK))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: no connection, returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
      } // end lock scope

      //std::string url_string = inherited::configuration_->URL;
      //std::string::size_type position =
      //  url_string.find (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER),
      //                   0,
      //                   ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER)));
      //ACE_ASSERT (position != std::string::npos);
      //url_string =
      //  url_string.replace (position,
      //                      ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER)),
      //                      (*iterator_).ISIN.c_str ());
      HTTP_Form_t form_data;
      form_data.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_FORM_KEY_SEARCH_STRING),
                                        (*iterator_).ISIN));

      // send first HTTP POST request
      if (!inherited::send (inherited::configuration_->URL,
                            inherited::configuration_->HTTPHeaders,
                            form_data))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to send HTTP request \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->URL.c_str ())));
        return;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: fetching \"%s\"...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT ((*iterator_).symbol.c_str ())));

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}

void
Test_I_Stream_HTTPGet::makeURI (const std::string& baseURL_in,
                                const std::string& ISIN_in,
                                std::string& URI_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTTPGet::makeURI"));

  // initialize return value(s)
  URI_out = baseURL_in;

  std::string::size_type position =
    URI_out.find (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER),
                  0,
                  ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER)));
  if (position != std::string::npos)
    URI_out = URI_out.replace (position,
                               ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER)),
                               ISIN_in.c_str ());
  else
  {
    URI_out += ACE_TEXT_ALWAYS_CHAR ("?");
    URI_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_FORM_KEY_SEARCH_STRING);
    URI_out += ACE_TEXT_ALWAYS_CHAR ("=");
    URI_out += ISIN_in;
    URI_out += ACE_TEXT_ALWAYS_CHAR ("&button=");
  } // end ELSE
}
