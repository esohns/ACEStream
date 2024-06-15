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
Stream_LibAV_Source_T<ACE_SYNCH_USE,
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
                      UserDataType>::Stream_LibAV_Source_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , context_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_LibAV_Source_T::Stream_LibAV_Source_T"));

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
Stream_LibAV_Source_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_LibAV_Source_T::initialize"));

  if (inherited::isInitialized_)
  {
    avformat_free_context (context_); context_ = NULL;
  } // end IF

  context_ = avformat_alloc_context ();
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to avformat_alloc_context(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
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
Stream_LibAV_Source_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_LibAV_Source_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->fileIdentifier.identifierDiscriminator == Common_File_Identifier::FILE);
  ACE_ASSERT (inherited::configuration_->streamIndex > -1);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (context_);

  int result = -1;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool stop_processing = false;
  struct AVPacket packet_s;
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;
  enum Stream_MediaType_Type message_media_type_s = STREAM_MEDIATYPE_INVALID;

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

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: processing stream %d: codec %d \"%s\", continuing\n"),
              inherited::mod_->name (),
              inherited::configuration_->streamIndex,
              context_->streams[inherited::configuration_->streamIndex]->codecpar->codec_id,
              ACE_TEXT (avcodec_get_name (context_->streams[inherited::configuration_->streamIndex]->codecpar->codec_id))));
  inherited::configuration_->codecId =
    context_->streams[inherited::configuration_->streamIndex]->codecpar->codec_id;
  
  switch (context_->streams[inherited::configuration_->streamIndex]->codecpar->codec_type)
  {
    case AVMEDIA_TYPE_AUDIO:
    {
      message_media_type_s = STREAM_MEDIATYPE_AUDIO;
      media_type_s.audio.channels =
        context_->streams[inherited::configuration_->streamIndex]->codecpar->ch_layout.nb_channels;
      media_type_s.audio.format =
        static_cast<enum AVSampleFormat> (context_->streams[inherited::configuration_->streamIndex]->codecpar->format);
      media_type_s.audio.sampleRate =
        context_->streams[inherited::configuration_->streamIndex]->codecpar->sample_rate;
      media_type_s.audio.codec =
        context_->streams[inherited::configuration_->streamIndex]->codecpar->codec_id;
      break;
    }
    case AVMEDIA_TYPE_VIDEO:
    {
      message_media_type_s = STREAM_MEDIATYPE_VIDEO;
      media_type_s.video.codec =
        context_->streams[inherited::configuration_->streamIndex]->codecpar->codec_id;
      if (context_->streams[inherited::configuration_->streamIndex]->codecpar->extradata_size)
      { ACE_ASSERT (!session_data_r.codecConfigurationDataSize);
        session_data_r.codecConfigurationDataSize =
          context_->streams[inherited::configuration_->streamIndex]->codecpar->extradata_size;
        session_data_r.codecConfigurationData =
          reinterpret_cast<ACE_UINT8*> (av_malloc (session_data_r.codecConfigurationDataSize + AV_INPUT_BUFFER_PADDING_SIZE));
        ACE_ASSERT (session_data_r.codecConfigurationData);
        ACE_OS::memset (session_data_r.codecConfigurationData,
                        0,
                        session_data_r.codecConfigurationDataSize + AV_INPUT_BUFFER_PADDING_SIZE);
        ACE_OS::memcpy (session_data_r.codecConfigurationData,
                        context_->streams[inherited::configuration_->streamIndex]->codecpar->extradata,
                        session_data_r.codecConfigurationDataSize);
      } // end IF
      media_type_s.video.format =
        static_cast<enum AVPixelFormat> (context_->streams[inherited::configuration_->streamIndex]->codecpar->format);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      media_type_s.video.resolution =
        { context_->streams[inherited::configuration_->streamIndex]->codecpar->width,
          context_->streams[inherited::configuration_->streamIndex]->codecpar->height };
#else
      media_type_s.video.resolution =
        { static_cast<unsigned int> (context_->streams[inherited::configuration_->streamIndex]->codecpar->width),
          static_cast<unsigned int> (context_->streams[inherited::configuration_->streamIndex]->codecpar->height) };
#endif // ACE_WIN32 || ACE_WIN64
      media_type_s.video.frameRate =
        context_->streams[inherited::configuration_->streamIndex]->avg_frame_rate;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown codec type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  context_->streams[inherited::configuration_->streamIndex]->codecpar->codec_type));
      return -1;
    }
  } // end SWITCH
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
      inherited::stop (false, false, false);
      continue;
    } // end IF
    if (packet_s.stream_index != inherited::configuration_->streamIndex)
      goto skip;
    
    ACE_ASSERT (packet_s.size);
    message_p = inherited::allocateMessage (packet_s.size + inherited::configuration_->allocatorConfiguration->paddingBytes);
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      inherited::stop (false, false, false);
      continue;
    } // end IF
    message_p->size (packet_s.size);

    result = message_p->copy (reinterpret_cast<char*> (packet_s.data),
                              packet_s.size);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%u): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  packet_s.size));
      message_p->release (); message_p = NULL;
      inherited::stop (false, false, false);
      continue;
    } // end IF
    //message_p->wr_ptr (packet_s.size);
    message_p->setMediaType (message_media_type_s);
    av_packet_unref (&packet_s);

    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      inherited::stop (false, false, false);
      continue;
    } // end IF
    message_p = NULL;
  } while (true);

continue_2:
  return result_2;
}
