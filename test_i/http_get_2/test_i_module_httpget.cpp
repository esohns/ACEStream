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

#include "stream_macros.h"

Test_I_Stream_HTTPGet::Test_I_Stream_HTTPGet ()
 : inherited ()
 , iterator_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTTPGet::Test_I_Stream_HTTPGet"));

}

Test_I_Stream_HTTPGet::~Test_I_Stream_HTTPGet ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTTPGet::~Test_I_Stream_HTTPGet"));

}

void
Test_I_Stream_HTTPGet::handleDataMessage (Test_I_Stream_Message*& message_inout,
                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTTPGet::handleDataMessage"));

  bool delete_record = false;

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  if (inherited::responseReceived_)
    return; // done

  // *NOTE*: if the HTTP header is parsed upstream, there is no need to do it
  //         here...
  HTTP_Record* record_p = NULL;
  if (message_inout->isInitialized ())
  {
    const Test_I_MessageData_t& data_container_r = message_inout->get ();
    const Test_I_MessageData& data_r = data_container_r.get ();
    record_p = data_r.HTTPRecord;
  } // end IF
  else
  {
    record_p = inherited::parseResponse (*message_inout);
    delete_record = true;
  } // end IF
  if (!record_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve HTTP record, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  switch (record_p->status)
  {
  case HTTP_Codes::HTTP_STATUS_OK:
  {
    inherited::responseReceived_ = true;
    break; // done
  }
  case HTTP_Codes::HTTP_STATUS_MULTIPLECHOICES:
  case HTTP_Codes::HTTP_STATUS_MOVEDPERMANENTLY:
  case HTTP_Codes::HTTP_STATUS_MOVEDTEMPORARILY:
  case HTTP_Codes::HTTP_STATUS_NOTMODIFIED:
  {
    // step1: redirected --> extract location
    HTTP_HeadersIterator_t iterator =
      record_p->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_LOCATION_HEADER_STRING));
    if (iterator == record_p->headers.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: missing \"%s\" HTTP header, returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (HTTP_PRT_LOCATION_HEADER_STRING)));
      break;
    } // end IF

      // *TODO*: remove type inference
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("\"%s\" has been redirected to \"%s\" (status was: %d)\n"),
                ACE_TEXT (inherited::configuration_->URL.c_str ()), ACE_TEXT ((*iterator).second.c_str ()),
                record_p->status));

    // step2: send request
    if (!inherited::sendRequest ((*iterator).second))
    {
      ACE_ASSERT (inherited::mod_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to send HTTP request \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT ((*iterator).second.c_str ())));
      break;
    } // end IF

    break;
  }
  default:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid HTTP response (status was: %d): \"%s\", aborting\n"),
                record_p->status,
                ACE_TEXT (record_p->reason.c_str ())));

    Test_I_Stream_SessionData& session_data_r =
      const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());
    session_data_r.aborted = true;
    // *TODO*: close the connection as well

    break;
  }
  } // end SWITCH

  if (delete_record)
  {
    // sanity check(s)
    ACE_ASSERT (record_p);

    delete record_p;
  } // end IF
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
  ACE_ASSERT (inherited::mod_);

  switch (message_inout->type ())
  {
  case STREAM_SESSION_BEGIN:
  {
    // sanity check(s)
    ACE_ASSERT (!sessionData_);

    // *TODO*: remove type inferences
    inherited::sessionData_ =
      &const_cast<Test_I_Stream_SessionData_t&> (message_inout->get ());
    inherited::sessionData_->increase ();

    iterator_ = inherited::configuration_->stockItems.begin ();
    if (iterator_ == inherited::configuration_->stockItems.end ())
      break; // nothing to do

    std::string url_string = configuration_->URL;
    std::string::size_type position =
      url_string.find (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER), 0);
    ACE_ASSERT (position != std::string::npos);
    url_string =
      url_string.replace (position,
                          ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (TEST_I_URL_SYMBOL_PLACEHOLDER)),
                          (*iterator_).ISIN.c_str ());

    // send first HTTP Get request
    if (!inherited::sendRequest (url_string))
    {
      ACE_ASSERT (inherited::mod_);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to send HTTP request \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (url_string.c_str ())));
      return;
    } // end IF

    break;
  }
  case STREAM_SESSION_END:
  {
    if (inherited::sessionData_)
    {
      inherited::sessionData_->decrease ();
      inherited::sessionData_ = NULL;
    } // end IF

    break;
  }
  default:
    break;
  } // end SWITCH
}
