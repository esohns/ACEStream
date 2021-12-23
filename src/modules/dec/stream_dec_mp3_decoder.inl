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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mmreg.h"
#include "mtype.h"
 // *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "fmt123.h"

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
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
          typename UserDataType,
          typename MediaType>
  Stream_Decoder_MP3Decoder_T<ACE_SYNCH_USE,
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
                            UserDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            MediaType>::Stream_Decoder_MP3Decoder_T (ISTREAM_T* stream_in,
#else
                            MediaType>::Stream_Decoder_MP3Decoder_T (typename inherited::ISTREAM_T* stream_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                                     bool autoStart_in,
                                                                     enum Stream_HeadModuleConcurrency concurrency_in,
                                                                     bool generateSessionMessages_in)
 : inherited (stream_in,
              autoStart_in,
              concurrency_in,
              generateSessionMessages_in)
 , bufferSize_ (0)
 , handle_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MP3Decoder_T::Stream_Decoder_MP3Decoder_T"));

  int error_i = mpg123_init ();
  if (error_i != MPG123_OK)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mpg123_init(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (mpg123_plain_strerror (error_i))));
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
          typename UserDataType,
          typename MediaType>
Stream_Decoder_MP3Decoder_T<ACE_SYNCH_USE,
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
                            UserDataType,
                            MediaType>::~Stream_Decoder_MP3Decoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MP3Decoder_T::~Stream_Decoder_MP3Decoder_T"));

  if (handle_)
  {
    int error_i = mpg123_close (handle_);
    if (error_i != MPG123_OK)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to mpg123_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (mpg123_plain_strerror (error_i))));
    mpg123_delete (handle_);
  } // end IF
  mpg123_exit ();
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
          typename UserDataType,
          typename MediaType>
bool
Stream_Decoder_MP3Decoder_T<ACE_SYNCH_USE,
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
                            UserDataType,
                            MediaType>::initialize (const ConfigurationType& configuration_in,
                                                    Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MP3Decoder_T::initialize"));

  int error_i = MPG123_ERR;

  if (inherited::isInitialized_)
  {
    bufferSize_ = 0;
    if (handle_)
    {
      error_i = mpg123_close (handle_);
      ACE_ASSERT (error_i == MPG123_OK);
      mpg123_delete (handle_); handle_ = NULL;
    } // end IF
  } // end IF
  ACE_ASSERT (!bufferSize_);
  ACE_ASSERT (!handle_);

  error_i = MPG123_ERR;
  handle_ = mpg123_new (NULL, &error_i);
  if (!handle_              ||
      (error_i != MPG123_OK))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mpg123_new(NULL): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (mpg123_plain_strerror (error_i))));
    return false;
  } // end IF
  bufferSize_ = mpg123_outblock (handle_);

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
          typename UserDataType,
          typename MediaType>
int
Stream_Decoder_MP3Decoder_T<ACE_SYNCH_USE,
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
                            UserDataType,
                            MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MP3Decoder_T::svc"));

  int result = -1;
  int result_2 = -1;
  int error = 0;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool done_b = false;
  bool stop_processing_b = false;
  std::string file_path_string;
  unsigned int file_size_i = 0;
  int encoding_i = 0, channels_i = 0;
  long rate_l = 0;
  int error_i = MPG123_ERR;
  size_t done_u = 0;
  MediaType media_type_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&media_type_s, 0, sizeof (MediaType));
  struct _AMMediaType media_type_2;
  struct tWAVEFORMATEX format_s;
  HRESULT result_3 = E_FAIL;
#else
  struct Stream_MediaFramework_ALSA_MediaType media_type_2;
#endif // ACE_WIN32 || ACE_WIN64
  SessionDataContainerType* session_data_container_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (handle_);
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());

//next:
  file_path_string = inherited::configuration_->fileIdentifier.identifier;
  file_size_i = Common_File_Tools::size (file_path_string);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: processing file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (file_path_string.c_str ()),
              file_size_i));
  error_i = mpg123_open (handle_, file_path_string.c_str ());
  if (unlikely (error_i != MPG123_OK))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to  mpg123_open(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (file_path_string.c_str ()),
                ACE_TEXT (mpg123_plain_strerror (error_i))));
    return -1;
  } // end IF
  error_i = mpg123_getformat (handle_,
                              &rate_l,
                              &channels_i,
                              &encoding_i);
  if (unlikely (error_i != MPG123_OK))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to  mpg123_getformat(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (file_path_string.c_str ()),
                ACE_TEXT (mpg123_plain_strerror (error_i))));
    goto continue_;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
  ACE_OS::memset (&format_s, 0, sizeof (struct tWAVEFORMATEX));
  format_s.wFormatTag = WAVE_FORMAT_PCM;
  format_s.nChannels = channels_i;
  format_s.nSamplesPerSec = rate_l;
  format_s.wBitsPerSample = mpg123_encsize (encoding_i) * 8;
  format_s.nBlockAlign = (format_s.nChannels * (format_s.wBitsPerSample / 8));
  format_s.nAvgBytesPerSec = (format_s.nSamplesPerSec * format_s.nBlockAlign);
  result_3 = CreateAudioMediaType (&format_s, &media_type_2, TRUE);
  if (FAILED (result_3))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CreateAudioMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_3, true).c_str ())));
    goto continue_;
  } // end IF
#else
  ACE_ASSERT (encoding_i == MPG123_ENC_SIGNED_16); // *TODO*
  media_type_2.format = SND_PCM_FORMAT_S16;
  media_type_2.channels = channels_i;
  media_type_2.rate = rate_l;
#endif // ACE_WIN32 || ACE_WIN64
  inherited2::getMediaType (media_type_2,
                            media_type_s);
  session_data_r.formats.push_back (media_type_s);
  session_data_container_p = inherited::sessionData_->clone ();
  ACE_ASSERT (session_data_container_p);
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP,
                                               session_data_container_p,
                                               inherited::streamState_->userData)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), aborting\n"),
                inherited::mod_->name (),
                STREAM_SESSION_MESSAGE_STEP));
    goto continue_;
  } // end IF

  do
  {
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p,
                                &no_wait);
    if (result_2 == -1)
    { error = ACE_OS::last_error ();
      if (unlikely (error != EWOULDBLOCK)) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

        if (unlikely (inherited::current () != STREAM_STATE_FINISHED))
        {
          inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
          message_block_p->release (); message_block_p = NULL;
          continue;
        } // end IF

        break;
      } // end IF
    } // end IF

    if (message_block_p)
    {
      message_type = message_block_p->msg_type ();
      switch (message_type)
      {
        case ACE_Message_Block::MB_STOP:
        {
          if (unlikely (inherited::isHighPriorityStop_ && !inherited::abortSent_))
            inherited::control (STREAM_CONTROL_ABORT,
                                false); // forward upstream ?

          // *IMPORTANT NOTE*: when close()d manually (i.e. on a user abort),
          //                   the stream may not have finish()ed
          if (unlikely (inherited::current () != STREAM_STATE_FINISHED))
          {
            inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
            message_block_p->release (); message_block_p = NULL;
            continue;
          } // end IF

          // *NOTE*: this is racy; the penultimate thread may have left svc() and
          //         not have decremented thr_count_ yet. In this case, the
          //         stop-message might remain in the queue during shutdown (or,
          //         even worse-) during re-initialization...
          // *TODO*: ward against this scenario
          if (unlikely (inherited::thr_count_ > 1))
          {
            result_2 =
              (inherited::isHighPriorityStop_ ? inherited::ungetq (message_block_p, NULL)
                                              : inherited::putq (message_block_p, NULL));
            if (unlikely (result_2 == -1))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Task::putq(): \"%m\", aborting\n"),
                          inherited::mod_->name ()));
              message_block_p->release (); message_block_p = NULL;
              done_b = true;
              break;
            } // end IF
          } // end IF
          else
          {
            message_block_p->release (); message_block_p = NULL;
          } // end ELSE

          // --> SESSION_END has been processed; leave
          done_b = true;
          break;
        }
        default:
          break;
      } // end SWITCH
      // sanity check(s)
      if (unlikely (done_b))
        break;

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing_b);
      if (unlikely (stop_processing_b)) // <-- SESSION_END has been processed || serious error
      { stop_processing_b = false; // reset, just in case...
        // *IMPORTANT NOTE*: message_block_p has already been released() !

        if (likely (inherited::current () != STREAM_STATE_FINISHED))
        {
          inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
          continue;
        } // end IF
      } // end IF
    } // end IF

    // session aborted ?
    if (unlikely (session_data_r.aborted &&
                  (inherited::current () != STREAM_STATE_FINISHED)))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: session (id was: %u) aborted\n"),
                  inherited::mod_->name (),
                  session_data_r.sessionId));
      inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
      continue;
    } // end IF

    // *TODO*: remove type inference
    message_p = inherited::allocateMessage (bufferSize_);
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  bufferSize_));
      inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
      continue;
    } // end IF

    done_u = 0;
    error_i = mpg123_read (handle_,
                           reinterpret_cast<unsigned char*> (message_p->wr_ptr ()),
                           message_p->size (),
                           &done_u);
    switch (error_i)
    {
      case MPG123_DONE:
      { ACE_ASSERT (!done_u);
        message_p->release (); message_p = NULL;
        result = 0;

        inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
        continue;
      }
      case MPG123_OK:
      { ACE_ASSERT (done_u);
        message_p->wr_ptr (done_u);
        result_2 = inherited::put_next (message_p, NULL);
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          message_p->release (); message_p = NULL;
          inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
          continue;
        } // end IF
        message_p = NULL;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to mpg123_read(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_path_string.c_str ()),
                    ACE_TEXT (mpg123_plain_strerror (error_i))));
        message_p->release (); message_p = NULL;
        inherited::finished (); // *NOTE*: enqueues SESSION_END --> continue
        continue;
      }
    } // end SWITCH
  } while (true);

continue_:
  error_i = mpg123_close (handle_);
  if (error_i != MPG123_OK)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mpg123_close(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (mpg123_plain_strerror (error_i))));
  mpg123_delete (handle_); handle_ = NULL;

  return result;
}
