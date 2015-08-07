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

  size_t bytes_transferred = -1;
  bytes_written = stream_.send_n (message_inout,       // (chained) message
                                  NULL,                // timeout
                                  &bytes_transferred); // bytes transferred
  switch (bytes_written)
  {
    case -1:
    {
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
      // *TODO*: remove type inferences
      const typename SessionMessageType::SESSION_DATA_TYPE& session_data_container_r =
          message_inout->get ();
      const SessionDataType* session_data_p = session_data_container_r.getData ();
      ACE_ASSERT (session_data_p);

      std::string directory, filename;
      if (configuration_.targetFilename.empty ())
      {
        directory = Common_File_Tools::getDumpDirectory ();
        filename = ACE::basename (session_data_p->filename.c_str ());
      } // end IF
      else if (Common_File_Tools::isDirectory (configuration_.targetFilename))
      {
        directory = configuration_.targetFilename;
        filename = ACE::basename (session_data_p->filename.c_str ());
      } // end IF
      else if (Common_File_Tools::isValid (configuration_.targetFilename))
      {
        directory = ACE::dirname (configuration_.targetFilename.c_str ());
        filename = configuration_.targetFilename;
      } // end ELSE
      else
      {
        directory = Common_File_Tools::getDumpDirectory ();
        filename = ACE::basename (session_data_p->filename.c_str ());
      } // end IF
      filename = directory +
                 ACE_DIRECTORY_SEPARATOR_CHAR_A +
                 filename;

      ACE_FILE_Addr file_address;
      result = file_address.set (filename.c_str ());
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", returning\n"),
                    ACE_TEXT (filename.c_str ())));
        return;
      } // end IF
      ACE_FILE_Connector file_connector;
      result = file_connector.connect (stream_,
                                       file_address);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", returning\n"),
                    ACE_TEXT (configuration_.targetFilename.c_str ())));
        return;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("opened file stream \"%s\"...\n"),
                  ACE_TEXT (filename.c_str ())));

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
                    ACE_TEXT ("closed file stream \"%s\" (wrote: %d byte(s))...\n"),
                    ACE_TEXT (buffer),
                    file_info.size_));
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

  // sanity check(s)
  // *TODO*: remove type inferences
  if (Common_File_Tools::isReadable (configuration_.targetFilename))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("target file \"%s\" exists, continuing\n")));

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
