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

#include "stream_module_db_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_MySQLWriter_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            SessionDataType>::Stream_Module_MySQLWriter_T (ISTREAM_T* stream_in)
#else
                            SessionDataType>::Stream_Module_MySQLWriter_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , state_ (NULL)
 , manageLibrary_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::Stream_Module_MySQLWriter_T"));

  //  int result = -1;

  //  if (manageLibrary_)
  //  {
  //    result = mysql_library_init (0,     // argc
  //                                 NULL,  // argv
  //                                 NULL); // groups
  //    if (unlikely (result))
  //      ACE_DEBUG ((LM_DEBUG,
  //                  ACE_TEXT ("failed to mysql_library_init(): \"%s\", aborting\n"),
  //                  ACE_TEXT (mysql_error (NULL))));
  //  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_MySQLWriter_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::~Stream_Module_MySQLWriter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::~Stream_Module_MySQLWriter_T"));

  if (state_)
    mysql_close (state_);

  if (manageLibrary_)
    mysql_library_end ();
}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType>
//void
//Stream_Module_MySQLWriter_T<SessionMessageType,
//                            MessageType,
//                            ModuleHandlerConfigurationType,
//                            SessionDataType>::handleDataMessage (MessageType*& message_inout,
//                                                                 bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::handleDataMessage"));
//
//  ssize_t bytes_written = -1;
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  // sanity check(s)
//  if (unlikely (!connection_))
//  {
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to open db connection, returning\n")));
//    return;
//  } // end IF
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Module_MySQLWriter_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  const typename SessionMessageType::DATA_T& session_data_container_r =
      message_inout->getR ();
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (session_data_container_r.getR ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (state_);
//      ACE_ASSERT (inherited::configuration_->connectionConfigurations);
//      ACE_ASSERT (!inherited::configuration_->connectionConfigurations->empty ());

//      Net_ConnectionConfigurationsIterator_t iterator =
//        inherited::configuration_->connectionConfigurations->find (inherited::mod_->name ());
//      if (iterator == inherited::configuration_->connectionConfigurations->end ())
//        iterator =
//          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
//#if defined (_DEBUG)
//      else
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("%s: applying connection configuration\n"),
//                    inherited::mod_->name ()));
//#endif // _DEBUG
//      ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());
      // *TODO*: remove type inferences
//      ACE_INET_Addr host_address =
//          NET_CONFIGURATION_TCP_CAST ((*iterator).second)->socketConfiguration.address;
      ACE_TCHAR host_address_string[BUFSIZ];
      ACE_OS::memset (host_address_string, 0, sizeof (ACE_TCHAR[BUFSIZ]));
      unsigned long client_flags = 0;
      const char* user_name_string_p = NULL;
      const char* password_string_p = NULL;
      const char* database_name_string_p = NULL;
      MYSQL* result_2 = NULL;
      const char* result_p =
          inherited::configuration_->peerAddress.get_host_addr (host_address_string,
                                                                sizeof (ACE_TCHAR[BUFSIZ]));
      if (unlikely (!result_p || (result_p != host_address_string)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_INET_Addr::get_host_addr(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      client_flags =
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
           CLIENT_MULTI_RESULTS         | // client can handle multiple result
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
           CLIENT_REMEMBER_OPTIONS);        // remember options specified by
                                            // calls to mysql_options()
      user_name_string_p =
        (inherited::configuration_->loginOptions.user.empty () ? NULL // <-- current user (Unix) : options file (?)
                                                               : inherited::configuration_->loginOptions.user.c_str ());
      password_string_p =
        (inherited::configuration_->loginOptions.password.empty () ? NULL // <-- (user table ?, options file (?))
                                                                   : inherited::configuration_->loginOptions.password.c_str ());
      database_name_string_p =
        (inherited::configuration_->loginOptions.database.empty () ? NULL // <-- default database : options file (?)
                                                                   : inherited::configuration_->loginOptions.database.c_str ());
      result_2 =
        mysql_real_connect (state_,                                                    // state handle
                            host_address_string,                                       // host name/address
                            user_name_string_p,                                        // user
                            password_string_p,                                         // password (non-encrypted)
                            database_name_string_p,                                    // database
                            inherited::configuration_->peerAddress.get_port_number (), // port
                            NULL,                                                      // (UNIX) socket/named pipe
                            client_flags);                                             // client flags
      if (unlikely (result_2 != state_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to mysql_real_connect(%s,\"%s\",\"%s\",\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    host_address_string,
                    ACE_TEXT (user_name_string_p),
                    ACE_TEXT (password_string_p),
                    ACE_TEXT (database_name_string_p),
                    ACE_TEXT (mysql_error (state_))));
        goto error;
      } // end IF
//      result = mysql_ping ();
//      if (unlikely (result))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to mysql_ping(): \"%s\", aborting\n"),
//                    ACE_TEXT (mysql_error (&mysql))));
//        goto error;
//      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened database connection to %s\n"),
                  inherited::mod_->name (),
                  host_address_string));

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

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      if (unlikely (!state_))
        return; // nothing to do

      std::string query_string = ACE_TEXT_ALWAYS_CHAR ("INSERT INTO ");
      query_string += inherited::configuration_->dataBaseTable
                   += ACE_TEXT_ALWAYS_CHAR ("VALUES (");
//      for (Test_I_PageDataIterator_t iterator = session_data_r.data.pageData.begin ();
//           iterator != session_data_r.data.pageData.end ();
//           ++iterator)
//      {
//        query_string += ACE_TEXT_ALWAYS_CHAR (",");

//        query_string += ACE_TEXT_ALWAYS_CHAR ("),(");
//      } // end FOR
      query_string += ACE_TEXT_ALWAYS_CHAR (")");
      my_ulonglong result_2 = 0;

      result = mysql_real_query (state_,
                                 query_string.c_str (),
                                 query_string.size ());
      if (unlikely (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to mysql_real_query(\"%s\"): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (query_string.c_str ()),
                    ACE_TEXT (mysql_error (state_))));
        goto close;
      } // end IF
      result_2 = mysql_affected_rows (state_);
      if (unlikely (result_2 != session_data_r.data.pageData.size ()))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: failed to insert %u record(s) (result was: %u), continuing\n"),
                    inherited::mod_->name (),
                    session_data_r.data.pageData.size (),
                    result_2));
        goto commit;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: inserted %u record(s)\n"),
                  inherited::mod_->name (),
                  session_data_r.data.pageData.size ()));

commit:
    //my_bool result_3 = mysql_commit (state_);
    //if (unlikely (result_3))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("%s: failed to mysql_commit(): \"%s\", aborting\n"),
    //              inherited::mod_->name (),
    //              ACE_TEXT (mysql_error (state_))));
    //  goto close;
    //} // end IF
//#if defined (_DEBUG)
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("committed %u data record(s)\n"),
    //            session_data_r.data.pageData.size (), result_2));
//#endif // _DEBUG

close:
      mysql_close (state_);
      state_ = NULL;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed database connection\n"),
                  inherited::mod_->name ()));

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
          typename SessionMessageType,
          typename SessionDataType>
bool
Stream_Module_MySQLWriter_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLWriter_T::initialize"));

  int result = -1;

  // step0: initialize library ?
  static bool first_run = true;
  if (first_run && manageLibrary_)
  {
    result = mysql_library_init (0,     // argc
                                 NULL,  // argv
                                 NULL); // groups
    if (unlikely (result))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to mysql_library_init(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (mysql_error (NULL))));
      return false;
    } // end IF
    first_run = false;
  } // end IF

  if (inherited::isInitialized_)
  {
    if (state_)
      mysql_close (state_);
    state_ = NULL;
  } // end IF
  ACE_ASSERT (!state_);

//  mysql_thread_init ();
//  my_init ();
  state_ = mysql_init (NULL);
  if (unlikely (!state_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: failed to mysql_init(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (mysql_error (NULL))));
    return false;
  } // end IF

  // [MYSQL_DEFAULT_AUTH [/ MYSQL_ENABLE_CLEARTEXT_PLUGIN]]
  // [MYSQL_OPT_BIND]
  // MYSQL_OPT_CONNECT_TIMEOUT
  // [MYSQL_OPT_NAMED_PIPE] // win32
  // MYSQL_OPT_PROTOCOL
  // MYSQL_OPT_READ_TIMEOUT
  // MYSQL_OPT_RECONNECT
  // MYSQL_OPT_WRITE_TIMEOUT
  // [MYSQL_READ_DEFAULT_FILE / MYSQL_READ_DEFAULT_GROUP]
  // [MYSQL_SET_CHARSET_NAME / MYSQL_AUTODETECT_CHARSET_NAME]
//  char* argument_p = configuration_.DBOptionFileName.c_str ();
  unsigned int timeout = MODULE_DB_MYSQL_DEFAULT_TIMEOUT_CONNECT;
  mysql_option option = MYSQL_OPT_CONNECT_TIMEOUT;
  bool value_b = false;
//  my_bool value_b = false;
  //result = mysql_options (state_,
  //                        option,
  //                        &timeout);
  //if (unlikely (result))
  //  goto error;
  //mysql_protocol_type protocol = MYSQL_PROTOCOL_TCP;
  //      switch (configuration_.transportLayer)
  //      {
  //        case NET_TRANSPORT_LAYER_TCP:
  //          protocol = MYSQL_PROTOCOL_TCP; break;
  //        case NET_TRANSPORT_LAYER_UDP:
  //        default:
  //        {
  //          ACE_DEBUG ((LM_ERROR,
  //                      ACE_TEXT ("invalid/unknown transport layer type (was: %d), aborting\n")));
  //          return false;
  //        }
  //      } // end SWITCH
  //option = MYSQL_OPT_PROTOCOL;
  //result = mysql_options (state_,
  //                        option,
  //                        &protocol);
  //if (unlikely (result))
  //  goto error;
  timeout = MODULE_DB_MYSQL_DEFAULT_TIMEOUT_READ;
  option = MYSQL_OPT_READ_TIMEOUT;
  result = mysql_options (state_,
                          option,
                          &timeout);
  if (unlikely (result))
    goto error;
  value_b = MODULE_DB_MYSQL_DEFAULT_RECONNECT;
  option = MYSQL_OPT_RECONNECT;
  result = mysql_options (state_,
                          option,
                          &value_b);
  if (unlikely (result))
    goto error;
  timeout = MODULE_DB_MYSQL_DEFAULT_TIMEOUT_WRITE;
  option = MYSQL_OPT_WRITE_TIMEOUT;
  result = mysql_options (state_,
                          option,
                          &timeout);
  if (unlikely (result))
    goto error;
  if (!configuration_in.dataBaseOptionsFileName.empty ())
  {
    option = MYSQL_READ_DEFAULT_FILE;
    result = mysql_options (state_,
                            option,
                            configuration_in.dataBaseOptionsFileName.c_str ());
    if (unlikely (result))
      goto error;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);

error:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: failed to mysql_options(%d): \"%s\", aborting\n"),
              inherited::mod_->name (),
              option,
              ACE_TEXT (mysql_error (state_))));
  return false;
}
