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
#include <mmreg.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "fmt123.h"

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

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
#endif
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
  int encoding_i = 0, channels_i = 0;
  long rate_l = 0;
  int error_i = MPG123_ERR;
  size_t done_u = 0;
  MediaType media_type_s;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  //ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (handle_);
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
//  ACE_ASSERT (session_data_r.lock);

//next:
  file_path_string = inherited::configuration_->fileIdentifier.identifier;
  file_size_i = Common_File_Tools::size (file_path_string);
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: processing file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (file_path_string.c_str ()),
              file_size_i));
#endif // _DEBUG
  error_i = mpg123_open (handle_, file_path_string.c_str ());
  if (error_i != MPG123_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to  mpg123_open(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (file_path_string.c_str ()),
                ACE_TEXT (mpg123_plain_strerror (error_i))));
    return result_2;
  } // end IF
  error_i = mpg123_getformat (handle_,
                              &rate_l,
                              &channels_i,
                              &encoding_i);
  if (error_i != MPG123_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to  mpg123_getformat(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (file_path_string.c_str ()),
                ACE_TEXT (mpg123_plain_strerror (error_i))));
    goto continue_;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (!session_data_r.formats.empty ());
  inherited2::getMediaType (session_data_r.formats.front (),
                            media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (media_type_s.pbFormat);
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
  ACE_ASSERT (waveformatex_p);
  waveformatex_p->wFormatTag = WAVE_FORMAT_PCM;
  waveformatex_p->nChannels = channels_i;
  waveformatex_p->nSamplesPerSec = rate_l;
  ACE_ASSERT (encoding_i == MPG123_ENC_SIGNED_16);
  waveformatex_p->wBitsPerSample = 16;
  waveformatex_p->nBlockAlign =
    (waveformatex_p->nChannels * waveformatex_p->wBitsPerSample) / 8;
  waveformatex_p->nAvgBytesPerSec =
    (waveformatex_p->nSamplesPerSec * waveformatex_p->nBlockAlign);
#else
  ACE_ASSERT (false); // *TODO*
#endif // ACE_WIN32 || ACE_WIN64

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
    message_p = inherited::allocateMessage (bufferSize_);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  bufferSize_));

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::STATE_MACHINE_T::finished ();

      continue;
    } // end IF

    done_u = 0;
    error_i = mpg123_read (handle_,
                           reinterpret_cast<unsigned char*> (message_p->wr_ptr ()),
                           message_p->size (),
                           &done_u);
    switch (error_i)
    {
      case MPG123_OK:
      case MPG123_DONE:
      { //ACE_ASSERT (done_u);
        message_p->wr_ptr (done_u);
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

        if (error_i == MPG123_DONE)
        {
          result_2 = 0;

          finished = true;
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::STATE_MACHINE_T::finished ();
        }

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

        finished = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::STATE_MACHINE_T::finished ();

        break;
      }
    } // end SWITCH
    message_p = NULL;
  } while (true);

continue_:
  error_i = mpg123_close (handle_);
  if (error_i != MPG123_OK)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mpg123_close(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (mpg123_plain_strerror (error_i))));

  return result_2;
}
