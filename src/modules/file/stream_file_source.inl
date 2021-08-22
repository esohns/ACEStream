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

#include "common_file_common.h"
#include "common_file_tools.h"
#include "common_timer_manager_common.h"

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
          typename TimerManagerType,
          typename UserDataType>
Stream_Module_FileReaderH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            UserDataType>::Stream_Module_FileReaderH_T (ISTREAM_T* stream_in,
#else
                            UserDataType>::Stream_Module_FileReaderH_T (typename inherited::ISTREAM_T* stream_in,
#endif
                                                                        bool autoStart_in,
                                                                        enum Stream_HeadModuleConcurrency concurrency_in,
                                                                        bool generateSessionMessages_in)
 : inherited (stream_in,
              autoStart_in,
              concurrency_in,
              generateSessionMessages_in)
 , directory_ ()
 , isOpen_ (false)
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::Stream_Module_FileReaderH_T"));

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
          typename TimerManagerType,
          typename UserDataType>
Stream_Module_FileReaderH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::~Stream_Module_FileReaderH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::~Stream_Module_FileReaderH_T"));

  int result = -1;

  result = directory_.close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Dirent_Selector::close(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

  if (isOpen_)
  {
    result = stream_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF
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
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_Module_FileReaderH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    result = directory_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Dirent_Selector::close(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));

    if (isOpen_)
    {
      result = stream_.close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));

      isOpen_ = false;
    } // end IF
  } // end IF

  if (configuration_in.fileIdentifier.identifierDiscriminator == Common_File_Identifier::DIRECTORY)
  {
    result =
      directory_.open (ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ()),
                       configuration_in.fileIdentifier.selector,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       NULL);
#else
                       alphasort);
#endif // ACE_WIN32 || ACE_WIN64
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Dirent_Selector::open(\"%s\"): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ())));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: processing %d file(s) in \"%s\"\n"),
                inherited::mod_->name (),
                directory_.length (),
                ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ())));
#endif // _DEBUG
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
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
          typename TimerManagerType,
          typename UserDataType>
int
Stream_Module_FileReaderH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::svc"));

  int result = -1;
  int result_2 = -1;
  int error = 0;
  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;
  int file_index_i = 0;
  std::string file_path_string;
  unsigned int file_size_i = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (!isOpen_);
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
//  ACE_ASSERT (session_data_r.lock);

next:
  file_path_string = inherited::configuration_->fileIdentifier.identifier;
  if ((inherited::configuration_->fileIdentifier.identifierDiscriminator == Common_File_Identifier::DIRECTORY) &&
      directory_.length ())
  {
    file_path_string += ACE_DIRECTORY_SEPARATOR_STR;
    file_path_string += directory_[file_index_i++]->d_name;
  } // end IF
  file_size_i = Common_File_Tools::size (file_path_string);

  if (!Common_File_Tools::open (file_path_string,
                                (O_RDONLY |
                                 O_BINARY), // flags --> open,
                                stream_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_File_Tools::open(\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (file_path_string.c_str ())));

    // signal the controller
    inherited::STATE_MACHINE_T::finished ();

    goto continue_;
  } // end IF
  isOpen_ = true;
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: processing file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (file_path_string.c_str ()),
              Common_File_Tools::size (file_path_string)));
#endif // _DEBUG

  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result >= 0)
    { ACE_ASSERT (message_block_p);
      message_type = message_block_p->msg_type ();
      switch (message_type)
      {
        case ACE_Message_Block::MB_STOP:
        {
          // clean up
          message_block_p->release ();
          message_block_p = NULL;

          // *NOTE*: when close()d manually (i.e. user abort), 'finished' will not
          //         have been set at this stage

          // signal the controller ?
          if (!finished)
          {
            finished = true;
            // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
            //         --> continue
            inherited::STATE_MACHINE_T::finished ();
            // *NOTE*: (if passive,) STREAM_SESSION_END has been processed
            //         --> done
            if (inherited::thr_count_ == 0)
              goto done; // finished processing

            continue;
          } // end IF

done:
          result_2 = 0;

          goto continue_; // STREAM_SESSION_END has been processed
        }
        default:
          break;
      } // end SWITCH

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
      if (stop_processing)
      {
        // *IMPORTANT NOTE*: message_block_p has already been released() !

        finished = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::STATE_MACHINE_T::finished ();

        continue;
      } // end IF
    } // end IF
    else if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

        if (!finished)
        {
          finished = true;
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::STATE_MACHINE_T::finished ();
        } // end IF

        break;
      } // end IF
    } // end ELSE IF

    // session aborted ?
    if (session_data_r.aborted)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("session aborted\n")));

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::STATE_MACHINE_T::finished ();

      continue;
    } // end IF

    // *TODO*: remove type inference
    message_p =
        inherited::allocateMessage (inherited::configuration_->slurpFiles ? file_size_i
                                                                          : inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::STATE_MACHINE_T::finished ();

      continue;
    } // end IF

    bytes_read = stream_.recv (message_p->wr_ptr (),
                               message_p->size ());
    switch (bytes_read)
    {
      case 0:
      {
        message_p->release (); message_p = NULL;

        // more ?
        if (file_index_i &&
            (file_index_i < directory_.length ()))
        {
          result = stream_.close ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          isOpen_ = false;
          goto next;
        } // end IF

        finished = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::STATE_MACHINE_T::finished ();

        result_2 = 0;

        break;
      }
      case -1:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::recv(%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    message_p->size ()));

        message_p->release (); message_p = NULL;

        finished = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::STATE_MACHINE_T::finished ();

        break;
      }
      default:
      {
        message_p->wr_ptr (static_cast<size_t> (bytes_read));
        result = inherited::put_next (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));

          message_p->release (); message_p = NULL;

          finished = true;
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::STATE_MACHINE_T::finished ();
        } // end IF

        break;
      }
    } // end SWITCH
    message_p = NULL;
  } while (true);

continue_:
  if (isOpen_)
  {
    result = stream_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    isOpen_ = false;
  } // end IF

  return result_2;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename UserDataType>
Stream_Module_FileReader_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  UserDataType>::Stream_Module_FileReader_Reader_T (ISTREAM_T* stream_in)
#else
                                  UserDataType>::Stream_Module_FileReader_Reader_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Reader_T::Stream_Module_FileReader_Reader_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename UserDataType>
void
Stream_Module_FileReader_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
                                  UserDataType>::handleControlMessage (ControlMessageType& controlMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Reader_T::handleControlMessage"));

  switch (controlMessage_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_STEP:
    {
      // *TODO*: start reading next file
      ACE_ASSERT (false);
      ACE_NOTSUP;
      ACE_NOTREACHED (break;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown control message type (was: %d), returning\n"),
                  controlMessage_in.msg_type ()));
      return;
    }
  } // end SWITCH
}

// ---------------------------------------

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
Stream_Module_FileReader_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  UserDataType>::Stream_Module_FileReader_Writer_T (ISTREAM_T* stream_in)
#else
                                  UserDataType>::Stream_Module_FileReader_Writer_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , aborted_ (NULL)
 , fileName_ ()
 , isOpen_ (false)
 , passDownstream_ (false)
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Writer_T::Stream_Module_FileReader_Writer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
Stream_Module_FileReader_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
                                  UserDataType>::~Stream_Module_FileReader_Writer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Writer_T::~Stream_Module_FileReader_Writer_T"));

  int result = -1;

  if (isOpen_)
  {
    result = stream_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_File_Stream::close(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
bool
Stream_Module_FileReader_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
                                  UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Writer_T::initialize"));

  int result = -1;

  // sanity check(s)
  // *TODO*: remove type inferences
  if (!Common_File_Tools::isReadable (configuration_in.fileIdentifier.identifier))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: source file \"%s\" does not exist, aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ())));
    return false;
  } // end IF

  if (inherited::isInitialized_)
  {
    aborted_  = NULL;
    //fileName_ = ACE_Addr::sap_any;
    if (isOpen_)
    {
      result = stream_.close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
    } // end IF
    passDownstream_ = false;
  } // end IF

  // *TODO*: remove type inferences
  result = fileName_.set (ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ()));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_FILE_Addr::set (\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ())));
    return false;
  } // end IF
  //passDownstream_ = true;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_Module_FileReader_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
                                  UserDataType>::handleControlMessage (ControlMessageType& controlMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Writer_T::handleControlMessage"));

  switch (controlMessage_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_STEP:
    {
      // *TODO*: start reading next file
      ACE_ASSERT (false);
      ACE_NOTSUP;
      ACE_NOTREACHED (break;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown control message type (was: %d), returning\n"),
                  inherited::mod_->name (),
                  controlMessage_in.type ()));
      return;
    }
  } // end SWITCH
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_Module_FileReader_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
                                  UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Writer_T::handleSessionMessage"));

//  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      aborted_ = &session_data_r.aborted;

//      goto continue_;
//
//error:
//      this->notify (STREAM_SESSION_MESSAGE_ABORT);
//
//      break;
//
//continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      aborted_ = NULL;

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
          typename SessionDataType,
          typename UserDataType>
int
Stream_Module_FileReader_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
                                  UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Writer_T::svc"));

  int result = -1;
  int result_2 = -1;
  int error = 0;
  ACE_FILE_Connector file_connector;
  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  // *IMPORTANT NOTE*: processing has two distinct phases:
  //                   - initial: processing the file
  //                   - post   : processing inbound messages until the
  //                              session ends
  //                   Initially, the file is processed as fast as possible; the
  //                   thread does not wait for queued messages. After file
  //                   processing completes, it will start blocking on the queue
  //                   and process inbound messages instead
  ACE_Time_Value now = COMMON_TIME_NOW;
  ACE_Time_Value* timeout_p = &now;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool stop_processing = false;
  bool file_processing_complete = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inferences
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (!isOpen_);

  result =
    file_connector.connect (stream_,                 // stream
                            fileName_,               // filename
                            NULL,                    // timeout (block)
                            ACE_Addr::sap_any,       // (local) filename: N/A
                            0,                       // reuse_addr: N/A
                            (O_RDONLY |
                             O_BINARY),              // flags --> open
                            ACE_DEFAULT_FILE_PERMS); // permissions --> open
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_FILE_Connector::connect(%s): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_File_Tools::addressToString (fileName_).c_str ())));
    goto close;
  } // end IF
  isOpen_ = true;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: opened file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (Common_File_Tools::addressToString (fileName_).c_str ()),
              Common_File_Tools::size (fileName_)));

  // step1: start processing data
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              timeout_p);
    if (result >= 0)
    {
      ACE_ASSERT (message_block_p);
      message_type = message_block_p->msg_type ();
      switch (message_type)
      {
        case ACE_Message_Block::MB_STOP:
        {
          // clean up
          message_block_p->release (); message_block_p = NULL;

          result_2 = 0;

          goto close; // STREAM_SESSION_END has been processed
        }
        default:
          break;
      } // end SWITCH

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
      if (stop_processing && inherited::thr_count_)
      {
        // *IMPORTANT NOTE*: message_block_p has already been released() !

        inherited::stop (false, // wait ?
                         true); // high priority ?

        // MB_STOP has been enqueued --> process
        continue;
      } // end IF
    } // end IF
    else if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto close;
      } // end IF
    } // end ELSE IF

    if (file_processing_complete) continue;

    // *TODO*: remove type inference
    message_p =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      goto close;
    } // end IF

    bytes_read = stream_.recv (message_p->wr_ptr (),
                               message_p->size ());
    switch (bytes_read)
    {
      case 0:
      {
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("finished reading file...\n")));

        // clean up
        message_p->release ();

        if (!inherited::putControlMessage (STREAM_CONTROL_STEP,
                                           false))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_TaskBase_T::putControlMessage(%d), continuing\n"),
                      inherited::mod_->name (),
                      STREAM_CONTROL_STEP));
        else
          result_2 = 0;

        // *IMPORTANT NOTE*: file processing complete; make the thread block on
        //                   the message queue until MB_STOP arrives
        file_processing_complete = true;
        timeout_p = NULL;

        break;
      }
      case -1:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::recv(%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    message_p->size ()));

        // clean up
        message_p->release ();

        goto close;
      }
      default:
      {
        message_p->wr_ptr (static_cast<size_t> (bytes_read));

        if (passDownstream_)
        {
          message_block_p = message_p->duplicate ();
          if (!message_block_p)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));

            // clean up
            message_p->release ();

            goto close;
          } // end IF
        } // end IF
        result = inherited::reply (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));

          // clean up
          message_p->release ();
          if (message_block_p)
            message_block_p->release ();

          goto close;
        } // end IF

        if (passDownstream_)
        { ACE_ASSERT (message_block_p);
          result = inherited::put_next (message_block_p, NULL);
          if (result == -1)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));

            // clean up
            message_block_p->release ();

            goto close;
          } // end IF
        } // end IF

        break;
      }
    } // end SWITCH
  } while (true);

close:
  if (isOpen_)
  {
    result = stream_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    isOpen_ = false;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: closed file \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_File_Tools::addressToString (fileName_).c_str ())));
  } // end IF

  return result_2;
}
