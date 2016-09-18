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
#include <regex>
#include <sstream>

#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType>::Stream_Module_FileWriter_T ()
 : inherited ()
 , fileName_ ()
 , isOpen_ (false)
 , previousError_ (0)
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::Stream_Module_FileWriter_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::handleDataMessage"));

  ssize_t bytes_written = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
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
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("wrote %d bytes...\n"),
      //              bytes_transferred));

      // print progress dots ?
      // *TODO*: remove type inferences
      if (inherited::configuration_->printProgressDot)
        std::cout << '.';

      break;
    }
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();
      std::string directory, file_name;
      int open_flags = (O_CREAT |
                        O_TRUNC |
                        O_WRONLY);

      const ACE_TCHAR* path_name_p = fileName_.get_path_name ();
      ACE_ASSERT (path_name_p);
      bool is_empty = !ACE_OS::strlen (path_name_p);
      if (is_empty &&
          session_data_r.targetFileName.empty ())
        goto continue_; // nothing to do

      // *TODO*: remove type inferences
      directory =
        (session_data_r.targetFileName.empty () ? (is_empty ? Common_File_Tools::getTempDirectory ()
                                                            : ACE_TEXT (path_name_p))
                                                : session_data_r.targetFileName);
      file_name =
        (session_data_r.targetFileName.empty () ? (is_empty ? ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILE)
                                                            : ACE_TEXT (path_name_p))
                                                : session_data_r.targetFileName);
      // sanity check(s)
      if (!Common_File_Tools::isDirectory (directory))
      {
        // *TODO*: ACE::dirname() returns '.' on an empty argument, this isn't
        //         entirely accurate
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
        else if (Common_File_Tools::isValidFilename (directory))
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
      else if (Common_File_Tools::isValidFilename (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
      file_name = directory +
                  ACE_DIRECTORY_SEPARATOR_CHAR_A +
                  file_name;

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("overwriting existing target file \"%s\"\n"),
                    ACE_TEXT (file_name.c_str ())));

      if (!Common_File_Tools::open (file_name,  // FQ file name
                                    open_flags, // flags
                                    stream_))   // stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened file stream \"%s\"...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));
continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
      result = stream_.get_local_addr (fileName_);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n")));
      ACE_TCHAR buffer[PATH_MAX];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result = fileName_.addr_to_string (buffer, sizeof (buffer));
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n")));

      if (isOpen_)
      {
        ACE_FILE_Info file_information;
        result = stream_.get_info (file_information);
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
                    ACE_TEXT ("%s: closed file stream \"%s\" (wrote: %q byte(s))...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (buffer),
                    file_information.size_));
      } // end IF

      unsigned int file_index = 0;
      std::stringstream converter;

      std::string regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([^_.]+)(?:_([[:digit:]]+))?(\\..+)$");
      std::regex regex (regex_string,
                        std::regex::ECMAScript);
      std::cmatch match_results;
      if (!std::regex_match (buffer,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid file name (was: \"%s\"), returning\n"),
                    buffer));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      ACE_ASSERT (match_results[1].matched);
      std::string file_name = match_results.str (1);
      if (match_results[2].matched)
      {
        converter << match_results.str (2);
        converter >> file_index;
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      } // end IF
      converter << ++file_index;
      file_name += '_';
      file_name += converter.str ();
      if (match_results[3].matched)
        file_name += match_results.str (3);

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("overwriting existing target file \"%s\"\n"),
                    ACE_TEXT (file_name.c_str ())));

      int open_flags = (O_CREAT |
                        O_TRUNC |
                        O_WRONLY);
      if (!Common_File_Tools::open (file_name,  // FQ file name
                                    open_flags, // flags
                                    stream_))   // stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened file stream \"%s\"...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (isOpen_)
      {
        result = stream_.get_local_addr (fileName_);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n")));
        ACE_TCHAR buffer[PATH_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = fileName_.addr_to_string (buffer, sizeof (buffer));
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n")));
        ACE_FILE_Info file_information;
        result = stream_.get_info (file_information);
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
                    ACE_TEXT ("%s: closed file stream \"%s\" (wrote: %q byte(s))...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (buffer),
                    file_information.size_));
      } // end IF

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
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::initialize"));

  int result =
    fileName_.set (ACE_TEXT (configuration_in.targetFileName.c_str ()));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_Addr::set (\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));
    return false;
  } // end IF

  // sanity check(s)
  // *TODO*: remove type inferences
  if (Common_File_Tools::isReadable (configuration_in.targetFileName))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("target file \"%s\" exists, continuing\n"),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));

  return inherited::initialize (configuration_in);
}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType>
//const ModuleHandlerConfigurationType&
//Stream_Module_FileWriter_T<SessionMessageType,
//                           MessageType,
//                           ModuleHandlerConfigurationType,
//                           SessionDataType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::get"));
//
//  // sanity check(s)
//  ACE_ASSERT (configuration_);
//
//  return *configuration_;
//}

////////////////////////////////////////////////////////////////////////////////

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::Stream_Module_FileWriterH_T (LockType* lock_in,
                                                                                  bool autoStart_in)
 : inherited (lock_in,      // lock handle
              autoStart_in, // auto-start ?
              true)         // generate sesssion messages ?
 , fileName_ ()
 , isOpen_ (false)
 , previousError_ (0)
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::Stream_Module_FileWriterH_T"));

}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::~Stream_Module_FileWriterH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::~Stream_Module_FileWriterH_T"));

  int result = -1;

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
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  ssize_t bytes_written = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
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
      if (inherited::configuration_->printProgressDot)
        std::cout << '.';

      break;
    }
  } // end SWITCH
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;

      std::string directory, file_name;
      const ACE_TCHAR* path_name_p = fileName_.get_path_name ();
      if ((!path_name_p) &&
          session_data_r.targetFileName.empty ())
        goto continue_; // nothing to do

      // *TODO*: remove type inferences
      directory =
        (session_data_r.targetFileName.empty () ? (!path_name_p ? Common_File_Tools::getTempDirectory ()
                                                                : ACE_TEXT (path_name_p))
                                                : session_data_r.targetFileName);
      file_name =
        (session_data_r.targetFileName.empty () ? (!path_name_p ? ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILE)
                                                                : ACE_TEXT (path_name_p))
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
        else if (Common_File_Tools::isValidFilename (directory))
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
      else if (Common_File_Tools::isValidFilename (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
      file_name = directory +
                  ACE_DIRECTORY_SEPARATOR_CHAR_A +
                  file_name;

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("overwriting existing target file \"%s\"\n"),
                    ACE_TEXT (file_name.c_str ())));

      result = fileName_.set (file_name.c_str ());
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
                                  fileName_,               // filename
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
                  ACE_TEXT ("%s: opened file stream \"%s\"...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (isOpen_)
      {
        result = stream_.get_local_addr (fileName_);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n")));
        ACE_TCHAR buffer[PATH_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = fileName_.addr_to_string (buffer, sizeof (buffer));
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
                    ACE_TEXT ("%s: closed file stream \"%s\" (wrote: %u byte(s))...\n"),
                    inherited::mod_->name (),
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
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::initialize"));

  bool result = false;
  int result_2 = -1;

  if (inherited::initialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    if (isOpen_)
    {
      result_2 = stream_.close ();
      if (result_2 == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));
    } // end IF
    isOpen_ = false;
  } // end IF

  result_2 =
    fileName_.set (ACE_TEXT (configuration_in.targetFileName.c_str ()));
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_Addr::set (\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));
    return false;
  } // end IF

  // sanity check(s)
  // *TODO*: remove type inferences
  if (Common_File_Tools::isReadable (configuration_in.targetFileName))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("target file \"%s\" exists, continuing\n"),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));

  // OK: all's well...
  result = inherited::initialize (configuration_in);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return result;
}
template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
const ConfigurationType&
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::get"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  return *inherited::configuration_;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileWriterH_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step1: initialize info container POD
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <typename LockType,
//          typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_FileWriterH_T<LockType,
//                            TaskSynchType,
//                            TimePolicyType,
//                            SessionMessageType,
//                            ProtocolMessageType,
//                            ConfigurationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return);
//}

//template <typename LockType,
//          typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//bool
//Stream_Module_FileWriterH_T<LockType,
//                            TaskSynchType,
//                            TimePolicyType,
//                            SessionMessageType,
//                            ProtocolMessageType,
//                            ConfigurationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::putStatisticMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
////  // step1: initialize session data
////  IRC_StreamSessionData* session_data_p = NULL;
////  ACE_NEW_NORETURN (session_data_p,
////                    IRC_StreamSessionData ());
////  if (!session_data_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
////    return false;
////  } // end IF
////  //ACE_OS::memset (data_p, 0, sizeof (IRC_SessionData));
//  SessionDataType& session_data_r =
//      const_cast<SessionDataType&> (inherited::sessionData_->get ());
//  session_data_r.currentStatistic = statisticData_in;
//
////  // step2: allocate session data container
////  IRC_StreamSessionData_t* session_data_container_p = NULL;
////  // *NOTE*: fire-and-forget stream_session_data_p
////  ACE_NEW_NORETURN (session_data_container_p,
////                    IRC_StreamSessionData_t (stream_session_data_p,
////                                                    true));
////  if (!session_data_container_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//
////    // clean up
////    delete stream_session_data_p;
//
////    return false;
////  } // end IF
//
//  // step3: send the data downstream...
//  // *NOTE*: fire-and-forget session_data_container_p
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       *inherited::sessionData_,
//                                       inherited::configuration_->streamConfiguration->messageAllocator);
//}
