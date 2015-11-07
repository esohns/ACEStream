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

#include <iostream>

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_MySQLWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::Stream_Module_MySQLWriter_T ()
 : inherited ()
 , cleanLibrary_ (false)
 , isInitialized_ (false)
 , state_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::Stream_Module_MySQLWriter_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_MySQLWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::~Stream_Module_MySQLWriter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::~Stream_Module_MySQLWriter_T"));

  if (state_)
    mysql_close (state_);

  if (cleanLibrary_)
    mysql_library_end ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
void
Stream_Module_MySQLWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::handleDataMessage"));

  ssize_t bytes_written = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (!connection_)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to open db connection, returning\n")));
    return;
  } // end IF

}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
void
Stream_Module_MySQLWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
      message_inout->get ();
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (state_);

      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result = configuration_.peerAddress.addr_to_string (buffer,
                                                          sizeof (buffer));
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", aborting\n")));

        session_data_r.aborted = true;

        return;
      } // end IF
      ACE_TCHAR host_address[BUFSIZ];
      ACE_OS::memset (host_address, 0, sizeof (host_address));
      result = configuration_.peerAddress.get_host_address (host_address,
                                                            sizeof (host_address));
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::get_host_address(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (buffer)));

        session_data_r.aborted = true;

        return;
      } // end IF

      unsigned long client_flags =
          (//CAN_HANDLE_EXPIRED_PASSWORDS | // handle expired passwords
           //CLIENT_COMPRESS              | // use compression protocol
           //CLIENT_FOUND_ROWS            | // return #found (matched) rows
                                            // instead of #changed rows
           CLIENT_IGNORE_SIGPIPE        | // do not install a SIGPIPE handler
           CLIENT_IGNORE_SPACE          | // permit spaces after function
                                          // names
           //CLIENT_INTERACTIVE           | // permit interactive_timeout
                                            // seconds (instead of wait_timeout
                                            // seconds) of inactivity
           //CLIENT_LOCAL_FILES           | // enable LOAD_LOCAL_DATA handling
           CLIENT_MULTI_RESULTS);//         | // client can handle multiple result
                                            // sets from multiple-statement
                                            // executions/stored procedures
           //CLIENT_MULTI_STATEMENTS      | // client may send multiple
                                            // statements in a single string
                                            // (separated by ';')
           //CLIENT_NO_SCHEMA             | // do not permit
                                            // 'db_name.tbl_name.col_name'
                                            // syntax (ODBC)
           //CLIENT_ODBC                  | // unused
           //CLIENT_SSL                   | // use SSL. Do NOT set this
                                            // manually, use mysql_ssl_set()
                                            // instead
           //CLIENT_REMEMBER_OPTIONS);      // remember options specified by
                                            // calls to mysql_options()
      MYSQL* result_p =
          mysql_real_connect (state_,                                        // state handle
                              host_address,                                  // host name/address
                              configuration_.DBUser.c_str (),                // db user
                              configuration_.DBPassword.c_str (),            // db password (non-encrypted)
                              configuration_.DBDatabase.c_str (),            // db database
                              configuration_.peerAddress.get_port_number (), // port
                              NULL,                                          // (UNIX) socket/named pipe
                              client_flags);                                 // client flags
      if (result_p != state_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_real_connect(\"%s\",\"%s\",\"%s\",\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT (buffer),
                    ACE_TEXT (configuration_.DBUser.c_str ()),
                    ACE_TEXT (configuration_.DBPassword.c_str ()),
                    ACE_TEXT (configuration_.DBDatabase.c_str ()),
                    ACE_TEXT (mysql_error (&mysql))));

        session_data_r.aborted = true;

        return;
      } // end IF
//      result = mysql_ping ();
//      if (result)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to mysql_ping(): \"%s\", aborting\n"),
//                    ACE_TEXT (mysql_error (&mysql))));

//        session_data_r.aborted = true;

//        return;
//      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("opened db connection to \"%s\"...\n"),
                  ACE_TEXT (buffer)));

//      // enable debug messages ?
//      if (configuration_.debug)
//      {
//        /*
//        Activate debug trace of the MySQL client library (C API)
//        Only available with a debug build of the MySQL client library!
//        */
//        connection_->setClientOption ("libmysql_debug", "d:t:O,client.trace");

//        /*
//        Connector/C++ tracing is available if you have compiled the
//        driver using cmake -DMYSQLCPPCONN_TRACE_ENABLE:BOOL=1
//        */
//        int on_off = 1;
//        connection_->setClientOption ("clientTrace", &on_off);
//      } // end IF

      break;
    }
    case STREAM_SESSION_END:
    {
      // sanity check(s)
      if (!state_)
        return; // nothing to do

      std::string query_string = ACE_TEXT_ALWAYS_CHAR ("INSERT INTO ");
      query_string << configuration_.DBTable.c_str ()
                   << ACE_TEXT_ALWAYS_CHAR ("VALUES (");
      for (Test_I_PageDataIterator_t iterator = session_data_r.data.pageData.begin ();
           iterator != session_data_r.data.pageData.end ();
           ++iterator)
      {
        query_string << ACE_TEXT_ALWAYS_CHAR (",");

        query_string << ACE_TEXT_ALWAYS_CHAR ("),(");
      } // end FOR
      query_string << ACE_TEXT_ALWAYS_CHAR (");");

      result = mysql_query (state_,
                            query_string.c_str ());
      /* Fetch in reverse = descending order! */
      res->afterLast();
      while (res->previous())
        cout << "\t... MySQL counts: " << res->getInt("id") << endl;
      delete res;

      mysql_close (state_);
      state_ = NULL;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed db connection...\n")));

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
bool
Stream_Module_MySQLWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::initialize"));

  int result = -1;

  configuration_ = configuration_in;

  // step0: initialize library ?
  static bool first_run = true;
  if (first_run)
  {
    result = mysql_library_init (0,     // argc
                                 NULL,  // argv
                                 NULL); // groups
    if (result)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to mysql_library_init(): \"%s\", aborting\n"),
                  ACE_TEXT (mysql_error (NULL))));
      return false;
    } // end IF
    cleanLibrary_ = true;
    first_run = false;
  } // end IF

  if (isInitialized_)
  {
    if (state_)
      mysql_close (state_);
    state_ = NULL;

    isInitialized_ = false;
  } // end IF
  ACE_ASSERT (!state_);

//  mysql_thread_init ();
//  my_init ();
  state_ = mysql_init (NULL);
  if (!state_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to mysql_init(): \"%s\", aborting\n"),
                ACE_TEXT (mysql_error (NULL))));
    return false;
  } // end IF

  // *TODO*: set options
  //      switch (configuration_.transportLayer)
  //      {
  //        case NET_TRANSPORT_LAYER_TCP:
  //          connection_string = ACE_TEXT_ALWAYS_CHAR ("tcp"); break;
  //        case NET_TRANSPORT_LAYER_UDP:
  //          connection_string = ACE_TEXT_ALWAYS_CHAR ("udp"); break;
  //        default:
  //        {
  //          ACE_DEBUG ((LM_ERROR,
  //                      ACE_TEXT ("invalid/unknown transport layer type (was: %d), aborting\n")));

  //          session_data_r.aborted = true;

  //          return;
  //        }
  //      } // end SWITCH
  //      connection_string += ACE_TEXT_ALWAYS_CHAR ("://");
  //      connection_string += ACE_TEXT_ALWAYS_CHAR (buffer);
  // MYSQL_OPT_PROTOCOL, MYSQL_SET_CHARSET_NAME, MYSQL_OPT_RECONNECT...
//  char* argument_p = configuration_.DBOptionFileName.c_str ();
  result = mysql_options (state_,
                          MYSQL_READ_DEFAULT_FILE,
                          configuration_.DBOptionFileName.c_str ());
  if (result)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to mysql_options(MYSQL_READ_DEFAULT_FILE,\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (configuration_.DBOptionFileName.c_str ()),
                ACE_TEXT (mysql_error (state_))));
    return false;
  } // end IF

  isInitialized_ = true;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
const ModuleHandlerConfigurationType&
Stream_Module_MySQLWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::get"));

  return configuration_;
}
