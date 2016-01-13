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

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::Stream_Module_FileReader_T (bool isActive_in,
                                                                                bool autoStart_in)
 : inherited (NULL,         // lock handle
              isActive_in,  // active ?
              autoStart_in, // auto-start ?
              true)         // *NOTE*: when working in 'passive' mode, enabling
                            //         this utilizes the calling thread. Note
                            //         that this potentially renders state
                            //         transitions during processing a tricky
                            //         affair, as the calling thread may be
                            //         holding the lock --> check carefully
 , isInitialized_ (false)
 , isOpen_ (false)
 , stream_ ()
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::Stream_Module_FileReader_T"));

}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::~Stream_Module_FileReader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::~Stream_Module_FileReader_T"));

  int result = -1;

  if (timerID_ != -1)
  {
    const void* act_p = NULL;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                  &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  timerID_));
    else
      ACE_DEBUG ((LM_WARNING, // this should happen in END_SESSION
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  timerID_));
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
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::initialize"));

  int result = -1;
  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (timerID_ != -1)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                    &act_p);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    timerID_));
    } // end IF
    timerID_ = -1;
    isInitialized_ = false;

    if (isOpen_)
    {
      result = stream_.close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_File_Stream::close(): \"%m\", continuing\n")));

      isOpen_ = false;
    } // end IF
  } // end IF

  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

  return isInitialized_;
}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_FileReader_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::handleDataMessage"));

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_FileReader_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
//                                                                          bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::handleSessionMessage"));

//  int result = -1;

//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);

//  // sanity check(s)
//  // *TODO*: remove type inference
//  ACE_ASSERT (inherited::configuration_.streamConfiguration);
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);

//  switch (message_inout->type ())
//  {
//    case SESSION_BEGIN:
//    {
//      if (inherited::configuration_.streamConfiguration->statisticReportingInterval)
//      {
//        // schedule regular statistics collection...
//        ACE_Time_Value interval (STREAM_STATISTICS_COLLECTION, 0);
//        ACE_ASSERT (timerID_ == -1);
//        ACE_Event_Handler* handler_p = &statisticCollectionHandler_;
//        timerID_ =
//            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
//                                                                        NULL,                       // argument
//                                                                        COMMON_TIME_NOW + interval, // first wakeup time
//                                                                        interval);                  // interval
//        if (timerID_ == -1)
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
//          return;
//        } // end IF
//        //        ACE_DEBUG ((LM_DEBUG,
//        //                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for interval %#T...\n"),
//        //                    timerID_,
//        //                    &interval));
//      } // end IF

////      // start profile timer...
////      profile_.start ();

//      break;
//    }
//    case SESSION_END:
//    {
//      if (timerID_ != -1)
//      {
//        const void* act_p = NULL;
//        result =
//            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
//                                                                      &act_p);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
//                      timerID_));
//        timerID_ = -1;
//      } // end IF

//      break;
//    }
//    default:
//      break;
//  } // end SWITCH
//}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::upStream (Stream_Base_t* streamBase_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::upStream"));

  ACE_UNUSED_ARG (streamBase_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Base_t*
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::upStream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::upStream"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL;)
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::svc"));

  int result = -1;
  int result_2 = -1;
  ACE_FILE_Addr file_address;
  ACE_FILE_Connector file_connector;
  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  ProtocolMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (!isOpen_);
//  ACE_ASSERT (inherited::sessionData_);
//  const SessionDataType& session_data_r = inherited::sessionData_->get ();
//  ACE_ASSERT (session_data_r.lock);

  result = file_address.set (inherited::configuration_->fileName.c_str ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (inherited::configuration_->fileName.c_str ())));

    // signal the controller
    inherited::finished ();

    goto done;
  } // end IF
  result =
    file_connector.connect (stream_,                 // stream
                            file_address,            // filename
                            NULL,                    // timeout (block)
                            ACE_Addr::sap_any,       // (local) filename: N/A
                            0,                       // reuse_addr: N/A
                            (O_RDONLY | O_BINARY),   // flags --> open
                            ACE_DEFAULT_FILE_PERMS); // permissions --> open
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (inherited::configuration_->fileName.c_str ())));

    // signal the controller
    inherited::finished ();

    goto done;
  } // end IF
  isOpen_ = true;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened file stream \"%s\" (%u byte(s))...\n"),
              ACE_TEXT (inherited::configuration_->fileName.c_str ()),
              Common_File_Tools::size (inherited::configuration_->fileName)));

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
        allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));

      // signal the controller
      finished = true;
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

        // signal the controller
        finished = true;
        inherited::finished ();

        result_2 = 0;

        break;
      }
      case -1:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_IO::recv(%d): \"%m\", aborting\n"),
                    message_p->size ()));

        // signal the controller
        finished = true;
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

          // signal the controller
          finished = true;
          inherited::finished ();
        } // end IF

        break;
      }
    } // end SWITCH
  } while (true);

  result = stream_.close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                ACE_TEXT (inherited::configuration_->fileName.c_str ())));
  isOpen_ = false;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("closed file stream \"%s\"...\n"),
              ACE_TEXT (inherited::configuration_->fileName.c_str ())));

done:
  return result_2;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ProtocolMessageType*
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_out = NULL;

  if (inherited::configuration_->streamConfiguration->messageAllocator)
  {
    try
    {
      // *TODO*: remove type inference
      message_out =
          static_cast<ProtocolMessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_out = NULL;
    }
  } // end IF
  else
  {
    ACE_NEW_NORETURN (message_out,
                      ProtocolMessageType (requestedSize_in));
  } // end ELSE
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_FileReader_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_FileReader_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // step1: update session state
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

//  // step2: create session data object container
//  SessionDataContainerType* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    SessionDataContainerType (inherited::sessionData_,
//                                              false));
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
//    return false;
//  } // end IF

  // step3: send the statistic data downstream
//  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       *inherited::sessionData_,
                                       inherited::configuration_->streamConfiguration->messageAllocator);
}
