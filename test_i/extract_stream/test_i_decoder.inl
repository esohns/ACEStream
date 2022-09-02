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

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/channel_layout.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#endif /* __cplusplus */

#include "ace/Log_Msg.h"

#include "common_tools.h"
#if defined (_DEBUG)
#include "common_file_tools.h"
#endif // _DEBUG

#include "common_image_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_lib_common.h"
#include "stream_lib_ffmpeg_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

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
Test_I_Decoder_T<ACE_SYNCH_USE,
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
                 UserDataType>::Test_I_Decoder_T (ISTREAM_T* stream_in)
#else
                 UserDataType>::Test_I_Decoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , context_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Decoder_T::Test_I_Decoder_T"));

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
Test_I_Decoder_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Test_I_Decoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    avformat_free_context (context_); context_ = NULL;
  } // end IF

  context_ = avformat_alloc_context ();
  ACE_ASSERT (context_);

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
Test_I_Decoder_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Test_I_Decoder_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->fileIdentifier.identifierDiscriminator == Common_File_Identifier::FILE);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (context_);

  int result = -1;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool stop_processing = false;
  int stream_index_i = -1;
  struct AVPacket packet_s;
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;

  // read file
  result =
    avformat_open_input (&context_,
                         inherited::configuration_->fileIdentifier.identifier.c_str (),
                         NULL,
                         NULL);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to avformat_open_input(\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->fileIdentifier.identifier.c_str ())));
    return -1;
  } // end IF

  result = avformat_find_stream_info (context_,
                                      NULL);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to avformat_find_stream_info(\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->fileIdentifier.identifier.c_str ())));
    return -1;
  } // end IF
  for (int i = 0;
       i < context_->nb_streams;
       ++i)
  {
    if (context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: audio stream uses codec %d \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  context_->streams[i]->codecpar->codec_id, ACE_TEXT (avcodec_get_name (context_->streams[i]->codecpar->codec_id))));
      stream_index_i = i;
      media_type_s.audio.codec = context_->streams[i]->codecpar->codec_id;
      break;
    } // end IF
  } // end FOR
  if (stream_index_i == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no audio stream in \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->fileIdentifier.identifier.c_str ())));
    return -1;
  } // end IF
  session_data_r.formats.push_back (media_type_s);

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
          if (unlikely (inherited::isHighPriorityStop_))
          {
            if (likely (!inherited::abortSent_))
              inherited::control (STREAM_CONTROL_ABORT,
                                  false); // forward upstream ?
          } // end IF

          bool finished_b = false;
          // *IMPORTANT NOTE*: when close()d manually (i.e. on a user abort),
          //                   the stream may not have finish()ed at this point
          { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
            if (inherited::sessionEndSent_ || inherited::sessionEndProcessed_)
              finished_b = true;
          } // end lock scope
          if (!finished_b)
          {
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished (false); // recurse upstream ?
            message_block_p->release (); message_block_p = NULL;
            continue;
          } // end IF

          inherited::isHighPriorityStop_ = false;

          // clean up
          message_block_p->release (); message_block_p = NULL;

          // --> SESSION_END has been processed; leave
          result_2 = 0;

          goto continue_2; // STREAM_SESSION_END has been processed
        }
        default:
          break;
      } // end SWITCH

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
      if (stop_processing)
      { stop_processing = false;
        bool finished_b = false;
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (unlikely (inherited::sessionEndSent_ || inherited::sessionEndProcessed_))
            finished_b = true;
        } // end lock scope
        if (!finished_b)
        {
          inherited::stop (false, false, false);
          continue;
        } // end IF
      } // end IF
    } // end IF
    else if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (likely (error == EWOULDBLOCK))
        goto continue_;
      if (error != ESHUTDOWN)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF

      ACE_ASSERT (inherited::msg_queue_->state () == ACE_Message_Queue_Base::DEACTIVATED);
      result_2 = 0;

      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
        if (unlikely (inherited::current () != STREAM_STATE_FINISHED))
        {
          // need to reactivate the queue
          result = inherited::msg_queue_->activate ();
          if (unlikely (result == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            break;
          } // end IF

          inherited::stop (false, false, false);
          continue;
        } // end IF
      } // end lock scope
      break;
    } // end ELSE IF

continue_:
    message_p = inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      inherited::stop (false, false, false);
      continue;
    } // end IF
    message_p->size (inherited::configuration_->allocatorConfiguration->defaultBufferSize - inherited::configuration_->allocatorConfiguration->paddingBytes);

    av_init_packet (&packet_s);
    //result =
    //  av_packet_from_data (&packet_s,
    //                       reinterpret_cast<uint8_t*> (message_p->wr_ptr ()),
    //                       message_p->space () - AV_INPUT_BUFFER_PADDING_SIZE);
    //ACE_ASSERT (result == 0);
skip:
    result = av_read_frame (context_, &packet_s);
    if (unlikely (result < 0))
    {
      if (unlikely (result != AVERROR_EOF))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to av_read_frame(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      inherited::stop (false, false, false);
      continue;
    } // end IF
    if (packet_s.stream_index != stream_index_i)
      goto skip;
    result = message_p->copy (reinterpret_cast<char*> (packet_s.data),
                              packet_s.size);
    ACE_ASSERT (result == 0);
    //message_p->wr_ptr (packet_s.size);
    av_packet_unref (&packet_s);

    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      inherited::stop (false, false, false);
    } // end IF
    message_p = NULL;
  } while (true);

continue_2:
  return result_2;
}
