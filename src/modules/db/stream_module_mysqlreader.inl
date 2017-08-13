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

#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType>::Stream_Module_MySQLReader_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                                bool autoStart_in,
                                                                                bool generateSessionMessages_in,
                                                                                bool manageLibrary_in)
 : inherited (lock_in,                    // lock handle
              autoStart_in,               // auto-start ?
              generateSessionMessages_in) // generate sesssion messages ?
 , state_ (NULL)
 , manageLibrary_ (manageLibrary_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::Stream_Module_MySQLReader_T"));

//  int result = -1;

//  if (manageLibrary_)
//  {
//    result = mysql_library_init (0,     // argc
//                                 NULL,  // argv
//                                 NULL); // groups
//    if (result)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("failed to mysql_library_init(): \"%s\", aborting\n"),
//                  ACE_TEXT (mysql_error (NULL))));
//  } // end IF
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType>::~Stream_Module_MySQLReader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::~Stream_Module_MySQLReader_T"));

  if (state_)
    mysql_close (state_);

  if (manageLibrary_)
    mysql_library_end ();
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType>::initialize (const ConfigurationType& configuration_in,
                                                               Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::initialize"));

  bool result = false;
  int result_2 = -1;

  // step0: initialize library ?
  static bool first_run = true;
  if (first_run && manageLibrary_)
  {
    result_2 = mysql_library_init (0,     // argc
                                   NULL,  // argv
                                   NULL); // groups
    if (result_2)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to mysql_library_init(): \"%s\", aborting\n"),
                  ACE_TEXT (mysql_error (NULL))));
      return false;
    } // end IF
    first_run = false;
  } // end IF

  if (inherited::initialized_)
  {
    if (state_)
      mysql_close (state_);
    state_ = NULL;
  } // end IF

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
  result_2 = mysql_options (state_,
                            MYSQL_READ_DEFAULT_FILE,
                            configuration_in.DBOptionFileName.c_str ());
  if (result_2)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to mysql_options(MYSQL_READ_DEFAULT_FILE,\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (configuration_in.DBOptionFileName.c_str ()),
                ACE_TEXT (mysql_error (state_))));
    return false;
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

  return result;
}

//template <typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_MySQLReader_T<SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (DataMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::handleDataMessage"));

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
void
Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_.streamConfiguration);
  ACE_ASSERT (inherited::initialized_);

  const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
      message_inout->get ();
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // schedule regular statistic collection ?
      if (inherited::configuration_.streamConfiguration->statisticReportingInterval !=
          ACE_Time_Value::zero)
      {
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL, 0);
        ACE_ASSERT (inherited::timerID_ == -1);
        ACE_Event_Handler* handler_p = &inherited::statisticCollectionHandler_;
        inherited::timerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                        NULL,                       // argument
                                                                        COMMON_TIME_NOW + interval, // first wakeup time
                                                                        interval);                  // interval
        if (inherited::timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (ID: %d) for interval %#T...\n"),
//                    inherited::timerID_,
//                    &interval));
      } // end IF

      // sanity check(s)
      ACE_ASSERT (state_);

      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result =
          inherited::configuration_.peerAddress.addr_to_string (buffer,
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
      result =
          inherited::configuration_.peerAddress.get_host_address (host_address,
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
          mysql_real_connect (state_,                                                   // state handle
                              host_address,                                             // host name/address
                              inherited::configuration_.DBUser.c_str (),                // db user
                              inherited::configuration_.DBPassword.c_str (),            // db password (non-encrypted)
                              inherited::configuration_.DBDatabase.c_str (),            // db database
                              inherited::configuration_.peerAddress.get_port_number (), // port
                              NULL,                                                     // (UNIX) socket/named pipe
                              client_flags);                                            // client flags
      if (result_p != state_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_real_connect(\"%s\",\"%s\",\"%s\",\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT (buffer),
                    ACE_TEXT (inherited::configuration_.DBUser.c_str ()),
                    ACE_TEXT (inherited::configuration_.DBPassword.c_str ()),
                    ACE_TEXT (inherited::configuration_.DBDatabase.c_str ()),
                    ACE_TEXT (mysql_error (state_))));

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
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::timerID_,
                                                                      &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      inherited::timerID_));
        inherited::timerID_ = -1;
      } // end IF

      // sanity check(s)
      if (!state_)
        return; // nothing to do

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

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timestamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//
//  ACE_NOTREACHED (return;)
//}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
int
Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            StatisticHandlerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::svc"));

  int result = -1;
  int result_2 = -1;
  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  DataMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);
  ACE_ASSERT (state_);

  std::string query_string = ACE_TEXT_ALWAYS_CHAR ("SELECT * FROM ");
  query_string += inherited::configuration_.DBTable;
  MYSQL_RES* result_p = NULL;
  bool free_result = false;
  unsigned int number_of_fields = 0;
  unsigned long* field_lengths_p = NULL;
  MYSQL_ROW row;
  result = mysql_real_query (state_,
                             query_string.c_str (),
                             query_string.size ());
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to mysql_real_query(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (query_string.c_str ()),
                ACE_TEXT (mysql_error (state_))));

    // signal the controller
    finished = true;
    inherited::finished ();

    goto done;
  } // end IF
  result_p = mysql_use_result (state_);
  if (!result_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to mysql_use_result(): \"%s\", aborting\n"),
                ACE_TEXT (mysql_error (state_))));

    // signal the controller
    finished = true;
    inherited::finished ();

    goto done;
  } // end IF
  free_result = true;
  number_of_fields = mysql_num_fields (result_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("loaded DB table \"%s\" (%u record(s) in %u fields)...\n"),
              ACE_TEXT (inherited::configuration_.DBTable.c_str ()),
              mysql_num_rows (result_p),
              number_of_fields));

  // step1: start processing data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result == 0)
    {
      ACE_ASSERT (message_block_p);
      ACE_Message_Block::ACE_Message_Type message_type =
        message_block_p->msg_type ();
      if (message_type == ACE_Message_Block::MB_STOP)
      {
        // clean up
        message_block_p->release ();
        message_block_p = NULL;

        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will not
        //         have been set at this stage

        // signal the controller ?
        if (!finished)
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("session aborted...\n")));

          finished = true;
          inherited::finished ();

          continue;
        } // end IF

        break; // aborted
      } // end IF

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
    } // end IF
    else if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));

        // signal the controller ?
        if (!finished)
        {
          finished = true;
          inherited::finished ();
        } // end IF

        break;
      } // end IF
    } // end IF

    // *TODO*: remove type inference
    message_p =
        allocateMessage (inherited::configuration_.streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_.streamConfiguration->bufferSize));

      // signal the controller
      finished = true;
      inherited::finished ();

      continue;
    } // end IF

    row = mysql_fetch_row (result_p);
    if (!row)
    {
      unsigned int error = mysql_errno (state_);
      if (error)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_fetch_row(): \"%s\", aborting\n"),
                    ACE_TEXT (mysql_error (state_))));

      // signal the controller
      finished = true;
      inherited::finished ();

      if (!error)
        result_2 = 0;

      continue; // done
    } // end IF
    field_lengths_p = mysql_fetch_lengths (result_p);
    if (!field_lengths_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to mysql_fetch_lengths(): \"%s\", aborting\n"),
                  ACE_TEXT (mysql_error (state_))));

      // signal the controller
      finished = true;
      inherited::finished ();

      continue;
    } // end IF

    // *TODO*
  } while (true);

done:
  if (free_result)
    mysql_free_result (result_p);

  return result_2;
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//DataMessageType*
//Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::allocateMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_.streamConfiguration);
//
//  // initialize return value(s)
//  DataMessageType* message_out = NULL;
//
//  if (inherited::configuration_.streamConfiguration->messageAllocator)
//  {
//    try
//    {
//      // *TODO*: remove type inference
//      message_out =
//          static_cast<DataMessageType*> (inherited::configuration_.streamConfiguration->messageAllocator->malloc (requestedSize_in));
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
//                  requestedSize_in));
//      message_out = NULL;
//    }
//  } // end IF
//  else
//  {
//    ACE_NEW_NORETURN (message_out,
//                      DataMessageType (requestedSize_in));
//  } // end ELSE
//  if (!message_out)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
//                requestedSize_in));
//  } // end IF
//
//  return message_out;
//}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//bool
//Stream_Module_MySQLReader_T<ACE_SYNCH_USE,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_MySQLReader_T::putStatisticMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::sessionData_);
//  // *TODO*: remove type inferences
//  ACE_ASSERT (inherited::configuration_.streamConfiguration);
//
//  // step1: update session state
//  SessionDataType& session_data_r =
//      const_cast<SessionDataType&> (inherited::sessionData_->get ());
//  // *TODO*: remove type inferences
//  session_data_r.currentStatistic = statisticData_in;
//
//  // *TODO*: attach stream state information to the session data
//
////  // step2: create session data object container
////  SessionDataContainerType* session_data_p = NULL;
////  ACE_NEW_NORETURN (session_data_p,
////                    SessionDataContainerType (inherited::sessionData_,
////                                              false));
////  if (!session_data_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
////    return false;
////  } // end IF
//
//  // step3: send the statistic data downstream
////  // *NOTE*: fire-and-forget session_data_p here
//  // *TODO*: remove type inference
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       *inherited::sessionData_,
//                                       inherited::configuration_.streamConfiguration->messageAllocator);
//}
