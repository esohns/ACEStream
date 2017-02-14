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

#include <ace/FILE_Addr.h>
#include <ace/FILE_Connector.h>
#include <ace/Log_Msg.h>

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
                            UserDataType>::Stream_Module_FileReaderH_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                        bool autoStart_in,
                                                                        enum Stream_HeadModuleConcurrency concurrency_in,
                                                                        bool generateSessionMessages_in)
 : inherited (lock_in,
              autoStart_in,
              concurrency_in,
              generateSessionMessages_in)
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
                            UserDataType>::~Stream_Module_FileReaderH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::~Stream_Module_FileReaderH_T"));

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
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    if (isOpen_)
    {
      result = stream_.close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));

      isOpen_ = false;
    } // end IF

    inherited::isInitialized_ = false;
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
                            UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

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
//Stream_Module_FileReaderH_T<ACE_SYNCH_USE,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
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
                            UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::svc"));

  int result = -1;
  int result_2 = -1;
  int error = 0;
  ACE_FILE_Addr file_address;
  ACE_FILE_Connector file_connector;
  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::configuration_->streamConfiguration->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (!isOpen_);
  const SessionDataType& session_data_r = inherited::sessionData_->get ();
//  ACE_ASSERT (session_data_r.lock);

  result = file_address.set (inherited::configuration_->fileName.c_str ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (inherited::configuration_->fileName.c_str ())));

    // signal the controller
    inherited::finished ();

    goto continue_;
  } // end IF
  result =
    file_connector.connect (stream_,                 // stream
                            file_address,            // filename
                            NULL,                    // timeout (block)
                            ACE_Addr::sap_any,       // (local) filename: N/A
                            0,                       // reuse_addr: N/A
                            (O_RDONLY |
                             O_BINARY),              // flags --> open
                            ACE_DEFAULT_FILE_PERMS); // permissions --> open
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (inherited::configuration_->fileName.c_str ())));

    // signal the controller
    inherited::finished ();

    goto continue_;
  } // end IF
  isOpen_ = true;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: opened file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::configuration_->fileName.c_str ()),
              Common_File_Tools::size (inherited::configuration_->fileName)));

  // step1: start processing data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result >= 0)
    {
      ACE_ASSERT (message_block_p);
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
            inherited::finished ();
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
        inherited::finished ();

        continue;
      } // end IF
    } // end IF
    else if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));

        if (!finished)
        {
          finished = true;
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::finished ();
        } // end IF

        break;
      } // end IF
    } // end ELSE IF

    // session aborted ?
    if (session_data_r.aborted)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("session aborted...\n")));

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::finished ();

      continue;
    } // end IF

    // *TODO*: remove type inference
    message_p =
        inherited::allocateMessage (inherited::configuration_->streamConfiguration->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%u) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->allocatorConfiguration->defaultBufferSize));

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::finished ();

      continue;
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

        finished = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::finished ();

        result_2 = 0;

        break;
      }
      case -1:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_IO::recv(%d): \"%m\", aborting\n"),
                    message_p->size ()));


        // clean up
        message_p->release ();

        finished = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::finished ();

        break;
      }
      default:
      {
        message_p->wr_ptr (static_cast<size_t> (bytes_read));
        result = inherited::put_next (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));

          // clean up
          message_p->release ();

          finished = true;
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::finished ();
        } // end IF

        break;
      }
    } // end SWITCH
  } while (true);

continue_:
  if (isOpen_)
  {
    result = stream_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n")));
    isOpen_ = false;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: closed file \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->fileName.c_str ())));
  } // end IF

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
//bool
//Stream_Module_FileReaderH_T<ACE_SYNCH_USE,
//                           SessionMessageType,
//                           DataMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReaderH_T::putStatisticMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::sessionData_);
//  // *TODO*: remove type inference
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
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
//                                       inherited::configuration_->streamConfiguration->messageAllocator);
//}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename UserDataType>
Stream_Module_FileReader_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
                                  UserDataType>::Stream_Module_FileReader_Reader_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Reader_T::Stream_Module_FileReader_Reader_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename UserDataType>
Stream_Module_FileReader_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ControlMessageType,
                                  UserDataType>::~Stream_Module_FileReader_Reader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Reader_T::~Stream_Module_FileReader_Reader_T"));

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
      break;
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
                                  UserDataType>::Stream_Module_FileReader_Writer_T ()
 : inherited ()
 , aborted_ (false)
 , allocator_ (NULL)
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
                  ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));
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
  if (!Common_File_Tools::isReadable (configuration_in.fileName))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("source file \"%s\" does not exist, aborting\n"),
                ACE_TEXT (configuration_in.fileName.c_str ())));
    return false;
  } // end IF

  if (inherited::isInitialized_)
  {
    aborted_  = false;
    allocator_ = NULL;
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

  allocator_ = allocator_in;
  // *TODO*: remove type inferences
  result = fileName_.set (ACE_TEXT (configuration_in.fileName.c_str ()));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_Addr::set (\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (configuration_in.fileName.c_str ())));
    return false;
  } // end IF
  passDownstream_ = configuration_in.pushStatisticMessages;

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
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown control message type (was: %d), returning\n"),
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
    const_cast<SessionDataType&> (inherited::sessionData_->get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      aborted_ = &session_data_r.aborted;
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      aborted_ = NULL;

      //// *NOTE*: in passive 'concurrent' scenarios, there is no 'worker' thread
      ////         running svc()
      ////         --> do not signal completion in this case
      //// *TODO*: remove type inference
      //if (inherited::thr_count_)
      //  inherited::stop (false,  // wait for completion ?
      //                   false); // N/A

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
DataMessageType*
Stream_Module_FileReader_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
                                  UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_Writer_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_p = NULL;

  // *TODO*: remove type inference
  if (allocator_)
  {
allocate:
    try {
      // *TODO*: remove type inference
      message_p =
          static_cast<DataMessageType*> (allocator_->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      DataMessageType (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                    requestedSize_in));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                  requestedSize_in));
  } // end IF

  return message_p;
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
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::configuration_->streamConfiguration->allocatorConfiguration);
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
                ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (Common_File_Tools::Address2String (fileName_).c_str ())));
    goto close;
  } // end IF
  isOpen_ = true;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: opened file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (Common_File_Tools::Address2String (fileName_).c_str ()),
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
          message_block_p->release ();
          message_block_p = NULL;

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

        inherited::stop (false,  // wait ?
                         false); // N/A

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
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
        goto close;
      } // end IF
    } // end ELSE IF

    if (file_processing_complete) continue;

    // *TODO*: remove type inference
    message_p =
        allocateMessage (inherited::configuration_->streamConfiguration->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->allocatorConfiguration->defaultBufferSize));
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
                      ACE_TEXT ("failed to Stream_TaskBase_T::putControlMessage(%d), continuing\n"),
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
                    ACE_TEXT ("failed to ACE_FILE_IO::recv(%d): \"%m\", aborting\n"),
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
                        ACE_TEXT ("failed to DataMessageType::duplicate(): \"%m\", aborting\n")));

            // clean up
            message_p->release ();

            goto close;
          } // end IF
        } // end IF
        result = inherited::reply (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::reply(): \"%m\", aborting\n")));

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
                        ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));

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
                  ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n")));
    isOpen_ = false;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: closed file \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_File_Tools::Address2String (fileName_).c_str ())));
  } // end IF

  return result_2;
}
