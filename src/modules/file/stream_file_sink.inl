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
#include <limits>
#include <regex>
#include <sstream>

#include "ace/ACE.h"
#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_macros.h"

#include "stream_file_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           SessionMessageType>::Stream_Module_FileWriter_T (ISTREAM_T* stream_in)
#else
                           SessionMessageType>::Stream_Module_FileWriter_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , isOpen_ (false)
 , path_ ()
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
          typename SessionMessageType>
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::~Stream_Module_FileWriter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::~Stream_Module_FileWriter_T"));

  int result = -1;

  if (unlikely (isOpen_))
  {
    result = stream_.close ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::handleDataMessage"));

  ssize_t bytes_written = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!isOpen_))
    return;

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
//               (error == ERROR_INVALID_ACCESS); // 12 : (*TODO*: memory leakage ?)
      ACE_ASSERT (error == ERROR_DISK_FULL);      // 112: no space left on device
#else
      ACE_ASSERT (error == ENOSPC);
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_File_IO::send_n(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  message_inout->total_length ()));
      break;
    }
    default:
    {
      if (unlikely (bytes_written != static_cast<ssize_t> (message_inout->total_length ())))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_File_IO::send_n(): \"%m\" [wrote %d/%d bytes], continuing\n"),
                    inherited::mod_->name (),
                    bytes_transferred,
                    message_inout->total_length ()));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("wrote %d bytes...\n"),
      //              bytes_transferred));

      // print progress dots ?
      // *TODO*: remove type inferences
      if (unlikely (inherited::configuration_->printProgressDot))
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
          typename SessionMessageType>
void
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
          message_inout->getR ();
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          session_data_container_r.getR ();
      std::string directory, file_name;
      int open_flags = (O_CREAT |
                        O_TRUNC |
                        O_WRONLY);

      const ACE_TCHAR* path_name_p = path_.get_path_name ();
      ACE_ASSERT (path_name_p);
      bool is_empty = !ACE_OS::strlen (path_name_p);
      if (unlikely (is_empty &&
                    session_data_r.targetFileName.empty ()))
        goto continue_; // nothing to do

      // *TODO*: remove type inferences
      directory =
        (session_data_r.targetFileName.empty () ? (is_empty ? Common_File_Tools::getTempDirectory ()
                                                            : ACE_TEXT (path_name_p))
                                                : session_data_r.targetFileName);
      file_name =
        (session_data_r.targetFileName.empty () ? (is_empty ? ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILENAME)
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
            if (unlikely (!Common_File_Tools::createDirectory (directory)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to create directory \"%s\": \"%m\", returning\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else if (Common_File_Tools::isValidFilename (directory))
        {
          directory =
            ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
          if (!Common_File_Tools::isDirectory (directory))
            if (unlikely (!Common_File_Tools::createDirectory (directory)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to create directory \"%s\": \"%m\", returning\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: invalid target directory (was: \"%s\"), falling back\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (directory.c_str ())));
          directory = Common_File_Tools::getTempDirectory ();
        } // end ELSE
      } // end IF
      if (Common_File_Tools::isDirectory (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILENAME);
      else if (Common_File_Tools::isValidFilename (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
      file_name = directory +
                  ACE_DIRECTORY_SEPARATOR_CHAR_A +
                  file_name;

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: overwriting target file \"%s\"\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));

      if (unlikely (!Common_File_Tools::open (file_name,   // FQ file name
                                              open_flags,  // flags
                                              stream_)))   // stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened target file \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));
#endif // _DEBUG
continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
      result = stream_.get_local_addr (path_);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      ACE_TCHAR buffer[PATH_MAX];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result = path_.addr_to_string (buffer, sizeof (buffer));
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));

      if (likely (isOpen_))
      {
        ACE_FILE_Info file_information;
        result = stream_.get_info (file_information);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_info(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));

        result = stream_.close ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        isOpen_ = false;
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed target file \"%s\" (wrote: %q byte(s))\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (buffer),
                    file_information.size_));
#endif // _DEBUG
      } // end IF

      unsigned int file_index = 0;
      std::stringstream converter;

      std::string regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([^_.]+)(?:_([[:digit:]]+))?(\\..+)$");
      std::regex regular_expression (regex_string,
                                     std::regex::ECMAScript);
      std::cmatch match_results;
      if (unlikely (!std::regex_match (buffer,
                                       match_results,
                                       regular_expression,
                                       std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid file name (was: \"%s\"), returning\n"),
                    inherited::mod_->name (),
                    buffer));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());

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
                    ACE_TEXT ("%s: overwriting target file \"%s\"\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));

      int open_flags = (O_CREAT |
                        O_TRUNC |
                        O_WRONLY);
      if (unlikely (!Common_File_Tools::open (file_name,  // FQ file name
                                              open_flags, // flags
                                              stream_)))  // stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened target file \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));
#endif // _DEBUG
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      //if (inherited::thr_count_)
      //  inherited::stop (false); // wait ?

      if (likely (isOpen_))
      {
        result = stream_.get_local_addr (path_);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        ACE_TCHAR buffer[PATH_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = path_.addr_to_string (buffer, sizeof (buffer));
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        ACE_FILE_Info file_information;
        result = stream_.get_info (file_information);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_info(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));

        result = stream_.close ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        isOpen_ = false;
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed target file \"%s\" (wrote: %q byte(s))\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (buffer),
                    file_information.size_));
#endif // _DEBUG
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
          typename SessionMessageType>
bool
Stream_Module_FileWriter_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_T::initialize"));

  ACE_UNUSED_ARG (allocator_in);

  int result =
    path_.set (ACE_TEXT (configuration_in.targetFileName.c_str ()));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_FILE_Addr::set (\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));
    return false;
  } // end IF

#if defined (_DEBUG)
  if (Common_File_Tools::isReadable (configuration_in.targetFileName))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: target file \"%s\" exists, continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));
#endif // _DEBUG

  return inherited::initialize (configuration_in,
                                allocator_in);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_FileWriter_2<TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           SessionMessageType>::Stream_Module_FileWriter_2 (ISTREAM_T* stream_in)
#else
                           SessionMessageType>::Stream_Module_FileWriter_2 (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , isOpen_ (false)
 , path_ ()
 , previousError_ (0)
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_2::Stream_Module_FileWriter_2"));

}

template <typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_FileWriter_2<TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::~Stream_Module_FileWriter_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_2::~Stream_Module_FileWriter_2"));

  int result = -1;

  if (unlikely (isOpen_))
  {
    result = stream_.close ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));
  } // end IF
}

template <typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_FileWriter_2<TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_2::handleDataMessage"));

  ssize_t bytes_written = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!isOpen_))
    return;

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
//               (error == ERROR_INVALID_ACCESS); // 12 : (*TODO*: memory leakage ?)
      ACE_ASSERT (error == ERROR_DISK_FULL);      // 112: no space left on device
#else
      ACE_ASSERT (error == ENOSPC);
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_File_IO::send_n(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  message_inout->total_length ()));
      break;
    }
    default:
    {
      if (unlikely (bytes_written != static_cast<ssize_t> (message_inout->total_length ())))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_File_IO::send_n(): \"%m\" [wrote %d/%d bytes], continuing\n"),
                    inherited::mod_->name (),
                    bytes_transferred,
                    message_inout->total_length ()));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("wrote %d bytes...\n"),
      //              bytes_transferred));

      // print progress dots ?
      // *TODO*: remove type inferences
      if (unlikely (inherited::configuration_->printProgressDot))
        std::cout << '.';

      break;
    }
  } // end SWITCH
}

template <typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_FileWriter_2<TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_2::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
          message_inout->getR ();
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          session_data_container_r.getR ();
      std::string directory, file_name;
      int open_flags = (O_CREAT |
                        O_TRUNC |
                        O_WRONLY);

      const ACE_TCHAR* path_name_p = path_.get_path_name ();
      ACE_ASSERT (path_name_p);
      bool is_empty = !ACE_OS::strlen (path_name_p);
      if (unlikely (is_empty &&
                    session_data_r.targetFileName.empty ()))
        goto continue_; // nothing to do

      // *TODO*: remove type inferences
      directory =
        (session_data_r.targetFileName.empty () ? (is_empty ? Common_File_Tools::getTempDirectory ()
                                                            : ACE_TEXT (path_name_p))
                                                : session_data_r.targetFileName);
      file_name =
        (session_data_r.targetFileName.empty () ? (is_empty ? ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILENAME)
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
            if (unlikely (!Common_File_Tools::createDirectory (directory)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to create directory \"%s\": \"%m\", returning\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else if (Common_File_Tools::isValidFilename (directory))
        {
          directory =
            ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
          if (!Common_File_Tools::isDirectory (directory))
            if (unlikely (!Common_File_Tools::createDirectory (directory)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to create directory \"%s\": \"%m\", returning\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (directory.c_str ())));
              return;
            } // end IF
        } // end IF
        else
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: invalid target directory (was: \"%s\"), falling back\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (directory.c_str ())));
          directory = Common_File_Tools::getTempDirectory ();
        } // end ELSE
      } // end IF
      if (Common_File_Tools::isDirectory (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILENAME);
      else if (Common_File_Tools::isValidFilename (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
      file_name = directory +
                  ACE_DIRECTORY_SEPARATOR_CHAR_A +
                  file_name;

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: overwriting target file \"%s\"\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));

      if (unlikely (!Common_File_Tools::open (file_name,   // FQ file name
                                              open_flags,  // flags
                                              stream_)))   // stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened target file \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));
#endif // _DEBUG
continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
      result = stream_.get_local_addr (path_);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      ACE_TCHAR buffer[PATH_MAX];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result = path_.addr_to_string (buffer, sizeof (buffer));
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));

      if (likely (isOpen_))
      {
        ACE_FILE_Info file_information;
        result = stream_.get_info (file_information);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_info(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));

        result = stream_.close ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        isOpen_ = false;
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed target file \"%s\" (wrote: %q byte(s))\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (buffer),
                    file_information.size_));
#endif // _DEBUG
      } // end IF

      unsigned int file_index = 0;
      std::stringstream converter;

      std::string regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([^_.]+)(?:_([[:digit:]]+))?(\\..+)$");
      std::regex regular_expression (regex_string,
                                     std::regex::ECMAScript);
      std::cmatch match_results;
      if (unlikely (!std::regex_match (buffer,
                                       match_results,
                                       regular_expression,
                                       std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid file name (was: \"%s\"), returning\n"),
                    inherited::mod_->name (),
                    buffer));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());

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
                    ACE_TEXT ("%s: overwriting target file \"%s\"\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));

      int open_flags = (O_CREAT |
                        O_TRUNC |
                        O_WRONLY);
      if (unlikely (!Common_File_Tools::open (file_name,  // FQ file name
                                              open_flags, // flags
                                              stream_)))  // stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened target file \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));
#endif // _DEBUG
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      //if (inherited::thr_count_)
      //  inherited::stop (false); // wait ?

      if (likely (isOpen_))
      {
        result = stream_.get_local_addr (path_);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        ACE_TCHAR buffer[PATH_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = path_.addr_to_string (buffer, sizeof (buffer));
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        ACE_FILE_Info file_information;
        result = stream_.get_info (file_information);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_info(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));

        result = stream_.close ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        isOpen_ = false;
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed target file \"%s\" (wrote: %q byte(s))\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (buffer),
                    file_information.size_));
#endif // _DEBUG
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_Module_FileWriter_2<TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriter_2::initialize"));

  ACE_UNUSED_ARG (allocator_in);

  int result =
    path_.set (ACE_TEXT (configuration_in.targetFileName.c_str ()));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_FILE_Addr::set (\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));
    return false;
  } // end IF

#if defined (_DEBUG)
  if (Common_File_Tools::isReadable (configuration_in.targetFileName))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: target file \"%s\" exists, continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));
#endif // _DEBUG

  return inherited::initialize (configuration_in,
                                allocator_in);
}

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
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
          typename StatisticContainerType,
          typename StatisticHandlerType>
Stream_Module_FileWriterH_T<ACE_SYNCH_USE,
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
                            StatisticContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            StatisticHandlerType>::Stream_Module_FileWriterH_T (ISTREAM_T* stream_in,
#else
                            StatisticHandlerType>::Stream_Module_FileWriterH_T (typename inherited::ISTREAM_T* stream_in,
#endif
                                                                                bool autoStart_in,
                                                                                bool generateSessionMessages_in)
 : inherited (stream_in,
              autoStart_in,
              STREAM_HEADMODULECONCURRENCY_ACTIVE,
              generateSessionMessages_in)
 , isOpen_ (false)
 , previousError_ (0)
 , path_ ()
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::Stream_Module_FileWriterH_T"));

}

template <ACE_SYNCH_DECL,
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
          typename StatisticContainerType,
          typename StatisticHandlerType>
Stream_Module_FileWriterH_T<ACE_SYNCH_USE,
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
                            StatisticContainerType,
                            StatisticHandlerType>::~Stream_Module_FileWriterH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::~Stream_Module_FileWriterH_T"));

  int result = -1;

  if (isOpen_)
  {
    result = stream_.close ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF
}

template <ACE_SYNCH_DECL,
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
          typename StatisticContainerType,
          typename StatisticHandlerType>
void
Stream_Module_FileWriterH_T<ACE_SYNCH_USE,
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
                            StatisticContainerType,
                            StatisticHandlerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  ssize_t bytes_written = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!isOpen_))
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
                  ACE_TEXT ("%s: failed to ACE_File_IO::send_n(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  message_inout->total_length ()));
      break;
    }
    default:
    {
      if (unlikely (bytes_written != static_cast<ssize_t> (message_inout->total_length ())))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_File_IO::send_n(): \"%m\" [wrote %d/%d bytes], continuing\n"),
                    inherited::mod_->name (),
                    bytes_transferred,
                    message_inout->total_length ()));
//      else
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("wrote %d bytes\n"),
//                    bytes_transferred));

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
Stream_Module_FileWriterH_T<ACE_SYNCH_USE,
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
                            StatisticContainerType,
                            StatisticHandlerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  int result = -1;

  // sanity check(s)
  //ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();
      ACE_FILE_Connector file_connector;

      // sanity check(s)
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *(inherited::streamState_->currentSessionData->lock));
        inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;
      } // end lock scope

      std::string directory, file_name;
      const ACE_TCHAR* path_name_p = path_.get_path_name ();
      if ((!path_name_p) &&
          session_data_r.targetFileName.empty ())
        goto continue_; // nothing to do

      // *TODO*: remove type inferences
      directory =
        (session_data_r.targetFileName.empty () ? (!path_name_p ? Common_File_Tools::getTempDirectory ()
                                                                : ACE_TEXT (path_name_p))
                                                : session_data_r.targetFileName);
      file_name =
        (session_data_r.targetFileName.empty () ? (!path_name_p ? ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILENAME)
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
            if (unlikely (!Common_File_Tools::createDirectory (directory)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to create directory \"%s\": \"%m\", aborting\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (directory.c_str ())));
              goto error;
            } // end IF
        } // end IF
        else if (Common_File_Tools::isValidFilename (directory))
        {
          directory =
            ACE_TEXT_ALWAYS_CHAR (ACE::dirname (ACE_TEXT (directory.c_str ())));
          if (!Common_File_Tools::isDirectory (directory))
            if (unlikely (!Common_File_Tools::createDirectory (directory)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to create directory \"%s\": \"%m\", aborting\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (directory.c_str ())));
              goto error;
            } // end IF
        } // end IF
        else
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: invalid target directory (was: \"%s\"), falling back\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (directory.c_str ())));
          directory = Common_File_Tools::getTempDirectory ();
        } // end ELSE
      } // end IF
      if (Common_File_Tools::isDirectory (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_FILE_DEFAULT_OUTPUT_FILENAME);
      else if (Common_File_Tools::isValidFilename (file_name))
        file_name =
          ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (file_name.c_str ())));
      file_name = directory +
                  ACE_DIRECTORY_SEPARATOR_CHAR_A +
                  file_name;

      if (Common_File_Tools::isReadable (file_name))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: overwriting target file \"%s\"\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));

      result = path_.set (file_name.c_str ());
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));
        goto error;
      } // end IF
      result =
          file_connector.connect (stream_,                 // stream
                                  path_,               // filename
                                  NULL,                    // timeout (block)
                                  ACE_Addr::sap_any,       // (local) filename: N/A
                                  0,                       // reuse_addr: N/A
                                  (O_CREAT |
                                   O_TRUNC |
                                   O_WRONLY),              // flags --> open
                                  ACE_DEFAULT_FILE_PERMS); // permissions --> open
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_name.c_str ())));
        goto error;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened file stream \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_name.c_str ())));

continue_:
      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (isOpen_)
      {
        result = stream_.get_local_addr (path_);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_local_addr(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        ACE_TCHAR buffer[PATH_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = path_.addr_to_string (buffer, sizeof (buffer));
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_Addr::addr_to_string(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        ACE_FILE_Info file_info;
        ACE_OS::memset (&file_info, 0, sizeof (file_info));
        result = stream_.get_info (file_info);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::get_info(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        result = stream_.close ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed file stream \"%s\" (wrote: %u byte(s))\n"),
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

template <ACE_SYNCH_DECL,
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
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
Stream_Module_FileWriterH_T<ACE_SYNCH_USE,
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
                            StatisticContainerType,
                            StatisticHandlerType>::initialize (const ConfigurationType& configuration_in,
                                                               Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileWriterH_T::initialize"));

  bool result = false;
  int result_2 = -1;

  if (inherited::initialized_)
  {
    if (isOpen_)
    {
      result_2 = stream_.close ();
      if (unlikely (result_2 == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
    } // end IF
    isOpen_ = false;
  } // end IF

  result_2 =
    path_.set (ACE_TEXT (configuration_in.targetFileName.c_str ()));
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_FILE_Addr::set (\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.targetFileName.c_str ())));
    return false;
  } // end IF

  // sanity check(s)
  // *TODO*: remove type inferences
  //if (Common_File_Tools::isReadable (configuration_in.targetFileName))
  //  ACE_DEBUG ((LM_WARNING,
  //              ACE_TEXT ("%s: target file \"%s\" exists, continuing\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (configuration_in.targetFileName.c_str ())));

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
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
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
Stream_Module_FileWriterH_T<ACE_SYNCH_USE,
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
                            StatisticContainerType,
                            StatisticHandlerType>::collect (StatisticContainerType& data_out)
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
  if (unlikely (!inherited::putStatisticMessage (data_out)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}
