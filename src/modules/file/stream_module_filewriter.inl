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

#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_macros.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_FileWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::Stream_Module_FileWriter_T ()
 : inherited ()
 , isOpen_ (false)
 , previousError_ (0)
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::Stream_Module_FileWriter_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
Stream_Module_FileWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::~Stream_Module_FileWriter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::~Stream_Module_FileWriter_T"));

  int result = -1;

  if (isOpen_)
  {
    result = stream_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
void
Stream_Module_FileWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::handleDataMessage"));

  ssize_t bytes_written = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (!isOpen_)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to open file, returning\n")));
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  size_t bytes_transferred = std::numeric_limits<unsigned int>::max ();
#else
  size_t bytes_transferred = -1;
#endif
  bytes_written = stream_.send_n (message_inout,       // (chained) message
                                  NULL,                // timeout
                                  &bytes_transferred); // bytes transferred
  switch (bytes_written)
  {
    case -1:
    {
      // *NOTE*: most probable cause: disk full
      int error = ACE_OS::last_error ();
      if (previousError_ &&
          (error == previousError_))
        break;
      previousError_ = error;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (error == ERROR_DISK_FULL); // 112: no space left on device
#else
      ACE_ASSERT (error == ENOSPC);
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_File_IO::send_n(%d): \"%m\", continuing\n"),
                  message_inout->total_length ()));
      break;
    }
    default:
    {
      if (bytes_written != static_cast<ssize_t> (message_inout->total_length ()))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_File_IO::send_n(): \"%m\" [wrote %d/%d bytes], continuing\n"),
                    bytes_transferred,
                    message_inout->total_length ()));
//      else
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("wrote %d bytes...\n"),
//                    bytes_transferred));

      // print progress dots ?
      // *TODO*: remove type inferences
      if (configuration_.printProgressDot)
        std::cout << '.';

      break;
    }
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
void
Stream_Module_FileWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();

      std::string directory, file_name;
      // *TODO*: remove type inferences
      directory =
        (session_data_r.targetFileName.empty () ? (configuration_.targetFileName.empty () ? Common_File_Tools::getTempDirectory ()
                                                                                          : configuration_.targetFileName)
                                                : session_data_r.targetFileName);
      file_name =
        (session_data_r.targetFileName.empty () ? (configuration_.targetFileName.empty () ? ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILE)
                                                                                          : configuration_.targetFileName)
                                                : session_data_r.targetFileName);
      // sanity check(s)
      if (!Common_File_Tools::isDirectory (directory))
      {
        directory =
          ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
        if (Common_File_Tools::isValidPath (directory))
        {
          if (!Common_File_Tools::isDirectory (directory))
            if (!Common_File_Tools::createDirectory (directory))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to create directory \"%s\": \"%m\", returning\n"),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else if (Common_File_Tools::isValidFileName (directory))
        {
          directory =
            ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
          if (!Common_File_Tools::isDirectory (directory))
            if (!Common_File_Tools::createDirectory (directory))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to create directory \"%s\": \"%m\", returning\n"),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid target directory (was: \"%s\"), falling back\n"),
                      ACE_TEXT (directory.c_str ())));
          directory = Common_File_Tools::getTempDirectory ();
        } // end ELSE
      } // end IF
      if (Common_File_Tools::isDirectory (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILE);
      else if (Common_File_Tools::isValidFileName (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
      file_name = directory +
                  ACE_DIRECTORY_SEPARATOR_CHAR_A +
                  file_name;

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("overwriting existing target file \"%s\"\n"),
                    ACE_TEXT (file_name.c_str ())));

      ACE_FILE_Addr file_address;
      result = file_address.set (file_name.c_str ());
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", returning\n"),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      ACE_FILE_Connector file_connector;
      result =
          file_connector.connect (stream_,                 // stream
                                  file_address,            // filename
                                  NULL,                    // timeout (block)
                                  ACE_Addr::sap_any,       // (local) filename: N/A
                                  0,                       // reuse_addr: N/A
                                  (O_CREAT |
                                   O_TRUNC |
                                   O_WRONLY),              // flags --> open
                                  ACE_DEFAULT_FILE_PERMS); // permissions --> open
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", returning\n"),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("opened file stream \"%s\"...\n"),
                  ACE_TEXT (file_name.c_str ())));

      break;
    }
    case STREAM_SESSION_END:
    {
      if (isOpen_)
      {
        ACE_FILE_Addr file_address;
        result = stream_.get_local_addr (file_address);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n")));
        ACE_TCHAR buffer[PATH_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = file_address.addr_to_string (buffer, sizeof (buffer));
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n")));
        ACE_FILE_Info file_info;
        ACE_OS::memset (&file_info, 0, sizeof (file_info));
        result = stream_.get_info (file_info);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::get_info(): \"%m\", continuing\n")));
        result = stream_.close ();
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", returning\n")));
          return;
        } // end IF
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("closed file stream \"%s\" (wrote: %u byte(s))...\n"),
                    ACE_TEXT (buffer),
                    static_cast<unsigned int> (file_info.size_)));
      } // end IF

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
Stream_Module_FileWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::initialize"));

  configuration_ = configuration_in;

  //// sanity check(s)
  //// *TODO*: remove type inferences
  //if (Common_File_Tools::isReadable (configuration_.fileName))
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("target file \"%s\" exists, continuing\n"),
  //              ACE_TEXT (configuration_.fileName.c_str ())));

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType>
const ModuleHandlerConfigurationType&
Stream_Module_FileWriter_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::get"));

  return configuration_;
}

////////////////////////////////////////////////////////////////////////////////

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_FileWriterH_T<LockType,
                           TaskSynchType,
                           TimePolicyType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::Stream_Module_FileWriterH_T ()
 : inherited (NULL,  // lock handle
              false, // passive by default
              false, // DON'T auto-start !
              false, // do not run the svc() routine on start
              true)  // push session messages
 , statisticCollectHandler_ (ACTION_COLLECT,
                             this,
                             false)
 , statisticCollectHandlerID_ (-1)
 , isOpen_ (false)
 , previousError_ (0)
 , stream_ ()
 , initialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::Stream_Module_FileWriterH_T"));

}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::~Stream_Module_FileWriterH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::~Stream_Module_FileWriterH_T"));

  int result = -1;

  // clean up timer if necessary
  if (statisticCollectHandlerID_ != -1)
  {
    result =
     COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_);
    if (result <= 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statisticCollectHandlerID_));
  } // end IF

  if (isOpen_)
  {
    result = stream_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));
  } // end IF
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::handleDataMessage"));

  ssize_t bytes_written = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (!isOpen_)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to open file, returning\n")));
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  size_t bytes_transferred = std::numeric_limits<unsigned int>::max ();
#else
  size_t bytes_transferred = -1;
#endif
  bytes_written = stream_.send_n (message_inout,       // (chained) message
                                  NULL,                // timeout
                                  &bytes_transferred); // bytes transferred
  switch (bytes_written)
  {
    case -1:
    {
      // *NOTE*: most probable cause: disk full
      int error = ACE_OS::last_error ();
      if (previousError_ &&
          (error == previousError_))
        break;
      previousError_ = error;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (error == ERROR_DISK_FULL); // 112: no space left on device
#else
      ACE_ASSERT (error == ENOSPC);
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_File_IO::send_n(%d): \"%m\", continuing\n"),
                  message_inout->total_length ()));
      break;
    }
    default:
    {
      if (bytes_written != static_cast<ssize_t> (message_inout->total_length ()))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_File_IO::send_n(): \"%m\" [wrote %d/%d bytes], continuing\n"),
                    bytes_transferred,
                    message_inout->total_length ()));
//      else
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("wrote %d bytes...\n"),
//                    bytes_transferred));

      // print progress dots ?
      // *TODO*: remove type inferences
      if (inherited::configuration_.printProgressDot)
        std::cout << '.';

      break;
    }
  } // end SWITCH
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;

      std::string directory, file_name;
      // *TODO*: remove type inferences
      directory =
        (session_data_r.targetFileName.empty () ? (inherited::configuration_.targetFileName.empty () ? Common_File_Tools::getTempDirectory ()
                                                : inherited::configuration_.targetFileName)
                                                : session_data_r.targetFileName);
      file_name =
        (session_data_r.targetFileName.empty () ? (inherited::configuration_.targetFileName.empty () ? ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILE)
                                                                                                     : inherited::configuration_.targetFileName)
                                                : session_data_r.targetFileName);
      // sanity check(s)
      if (!Common_File_Tools::isDirectory (directory))
      {
        directory =
          ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
        if (Common_File_Tools::isValidPath (directory))
        {
          if (!Common_File_Tools::isDirectory (directory))
            if (!Common_File_Tools::createDirectory (directory))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to create directory \"%s\": \"%m\", returning\n"),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else if (Common_File_Tools::isValidFileName (directory))
        {
          directory =
            ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
          if (!Common_File_Tools::isDirectory (directory))
            if (!Common_File_Tools::createDirectory (directory))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to create directory \"%s\": \"%m\", returning\n"),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid target directory (was: \"%s\"), falling back\n"),
                      ACE_TEXT (directory.c_str ())));
          directory = Common_File_Tools::getTempDirectory ();
        } // end ELSE
      } // end IF
      if (Common_File_Tools::isDirectory (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILE);
      else if (Common_File_Tools::isValidFileName (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
      file_name = directory +
                  ACE_DIRECTORY_SEPARATOR_CHAR_A +
                  file_name;

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("overwriting existing target file \"%s\"\n"),
                    ACE_TEXT (file_name.c_str ())));

      ACE_FILE_Addr file_address;
      result = file_address.set (file_name.c_str ());
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", returning\n"),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      ACE_FILE_Connector file_connector;
      result =
          file_connector.connect (stream_,                 // stream
                                  file_address,            // filename
                                  NULL,                    // timeout (block)
                                  ACE_Addr::sap_any,       // (local) filename: N/A
                                  0,                       // reuse_addr: N/A
                                  (O_CREAT |
                                   O_TRUNC |
                                   O_WRONLY),              // flags --> open
                                  ACE_DEFAULT_FILE_PERMS); // permissions --> open
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", returning\n"),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("opened file stream \"%s\"...\n"),
                  ACE_TEXT (file_name.c_str ())));

      break;
    }
    case STREAM_SESSION_END:
    {
      if (isOpen_)
      {
        ACE_FILE_Addr file_address;
        result = stream_.get_local_addr (file_address);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n")));
        ACE_TCHAR buffer[PATH_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = file_address.addr_to_string (buffer, sizeof (buffer));
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n")));
        ACE_FILE_Info file_info;
        ACE_OS::memset (&file_info, 0, sizeof (file_info));
        result = stream_.get_info (file_info);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::get_info(): \"%m\", continuing\n")));
        result = stream_.close ();
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", returning\n")));
          return;
        } // end IF
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("closed file stream \"%s\" (wrote: %u byte(s))...\n"),
                    ACE_TEXT (buffer),
                    static_cast<unsigned int> (file_info.size_)));
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::initialize"));

  int result = -1;

  if (initialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (statisticCollectHandlerID_ != -1)
    {
      int result =
       COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_);
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    statisticCollectHandlerID_));
      statisticCollectHandlerID_ = -1;
    } // end IF

    if (isOpen_)
    {
      result = stream_.close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));
    } // end IF
    isOpen_ = false;

    initialized_ = false;
  } // end IF

  if (configuration_in.streamConfiguration->statisticReportingInterval)
  {
    // schedule regular statistics collection...
    ACE_Time_Value interval (STREAM_STATISTIC_COLLECTION, 0);
    ACE_ASSERT (statisticCollectHandlerID_ == -1);
    ACE_Event_Handler* handler_p = &statisticCollectHandler_;
    statisticCollectHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule (handler_p,                        // event handler
                                                            NULL,                             // act
                                                            COMMON_TIME_POLICY () + interval, // first wakeup time
                                                            interval);                        // interval
    if (statisticCollectHandlerID_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule(): \"%m\", aborting\n")));
      return false;
    } // end IF
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
//                 statisticCollectHandlerID_,
//                 statisticCollectionInterval_in));
  } // end IF

  // *NOTE*: need to clean up timer beyond this point !

  //// sanity check(s)
  //// *TODO*: remove type inferences
  //if (Common_File_Tools::isReadable (configuration_.fileName))
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("target file \"%s\" exists, continuing\n"),
  //              ACE_TEXT (configuration_.fileName.c_str ())));

  // OK: all's well...
  initialized_ = inherited::initialize (configuration_in);
  if (!initialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return initialized_;
}
template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
const ConfigurationType&
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::get"));

  return inherited::configuration_;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::collect"));

  // sanity check(s)
  ACE_ASSERT (initialized_);

  // step1: initialize info container POD
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timestamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::report"));

  // sanity check(s)
  ACE_ASSERT (initialized_);

  // *TODO*: support (local) reporting here as well ?
  //         --> leave this to any downstream modules...
  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);

//  // step1: initialize session data
//  IRC_StreamSessionData* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    IRC_StreamSessionData ());
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  //ACE_OS::memset (data_p, 0, sizeof (IRC_SessionData));
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  session_data_r.currentStatistic = statisticData_in;

//  // step2: allocate session data container
//  IRC_StreamSessionData_t* session_data_container_p = NULL;
//  // *NOTE*: fire-and-forget stream_session_data_p
//  ACE_NEW_NORETURN (session_data_container_p,
//                    IRC_StreamSessionData_t (stream_session_data_p,
//                                                    true));
//  if (!session_data_container_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

//    // clean up
//    delete stream_session_data_p;

//    return false;
//  } // end IF

  // step3: send the data downstream...
  // *NOTE*: fire-and-forget session_data_container_p
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       *inherited::sessionData_,
                                       inherited::configuration_.streamConfiguration->messageAllocator);
}
