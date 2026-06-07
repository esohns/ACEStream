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
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_defines.h"
#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::Stream_Dev_Target_OpenAL_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , bufferQueue_ (ACE_Message_Queue_Base::DEFAULT_HWM,
                 ACE_Message_Queue_Base::DEFAULT_LWM)
 , buffers_ ()
 , context_ (NULL)
 , device_ (NULL)
 , format_ (-1)
 , queue_ (0,    // max # slots; 0 --> unlimited
           NULL) // notification handle
 , sampleRate_ (0)
 , source_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::Stream_Dev_Target_OpenAL_T"));

  ACE_OS::memset (&buffers_, 0, sizeof (ALuint) * STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS);
  inherited::msg_queue (&queue_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::~Stream_Dev_Target_OpenAL_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::~Stream_Dev_Target_OpenAL_T"));

  alDeleteBuffers (STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS, buffers_);
  if (unlikely (source_))
    alDeleteSources (1, &source_);
  if (unlikely (context_))
    alcDestroyContext (context_);
  if (unlikely (device_))
    alcCloseDevice (device_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::initialize (const ConfigurationType& configuration_in,
                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::initialize"));

  if (inherited::isInitialized_)
  {
    alDeleteBuffers (STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS, buffers_);
    ACE_OS::memset (&buffers_, 0, sizeof (ALuint) * STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS);
    format_ = -1;
    sampleRate_ = 0;
    if (unlikely (source_))
    {
      alDeleteSources (1, &source_); source_ = 0;
    } // end IF
    if (unlikely (context_))
    {
      alcDestroyContext (context_); context_ = NULL;
    } // end IF
    if (unlikely (device_))
    {
      alcCloseDevice (device_); device_ = NULL;
    } // end IF
  } // end IF

  alGetError ();

  // *TODO*: support device selection via configuration
  device_ = alcOpenDevice (NULL);
  if (unlikely (!device_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to alcOpenDevice(NULL), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  alcGetError (device_);

  context_ = alcCreateContext (device_, NULL);
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to alcCreateContext(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  if (!alcMakeContextCurrent (context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to alcMakeContextCurrent(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  alListener3f (AL_POSITION, 0.0f, 0.0f, 1.0f);
  alListener3f (AL_VELOCITY, 0.0f, 0.0f, 0.0f);
  static ALfloat orientation_a[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};
  alListenerfv (AL_ORIENTATION, orientation_a);

  alGenSources (1, &source_);
  ALenum error_code_e = alGetError ();
  ACE_ASSERT ((error_code_e == AL_NO_ERROR) && source_);

  alSourcef (source_, AL_PITCH, 1.0f);
  alSourcef (source_, AL_GAIN, 1.0f);
  alSource3f (source_, AL_POSITION, 0.0f, 0.0f, 0.0f);
  alSource3f (source_, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
  alSourcei (source_, AL_LOOPING, AL_FALSE);

  alGenBuffers (STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS, buffers_);
  error_code_e = alGetError ();
  ACE_ASSERT ((error_code_e == AL_NO_ERROR) && buffers_[0]);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (source_);
  ACE_ASSERT (format_ != -1);
  ACE_ASSERT (sampleRate_ != 0);

  passMessageDownstream_out = false;

  ACE_Message_Block* message_block_p = message_inout;
  int result_2 = -1;
  ALuint* buffer_p = NULL;
  ALenum error_code_e;

continue_:
  // step1: get next free buffer
  result_2 = bufferQueue_.dequeue (buffer_p,
                                   NULL);
  if (unlikely (result_2 == -1) || !buffer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::dequeue(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  ACE_ASSERT (buffer_p && *buffer_p);

  // step2: fill buffer
  alBufferData (*buffer_p,
                format_,
                message_block_p->rd_ptr (),
                message_block_p->length (),
                sampleRate_);
  error_code_e = alGetError ();
  ACE_ASSERT ((error_code_e == AL_NO_ERROR));

  // step3: queue buffer on source
  alSourceQueueBuffers (source_, 1, buffer_p);
  error_code_e = alGetError ();
  ACE_ASSERT ((error_code_e == AL_NO_ERROR));

  message_block_p = message_block_p->cont ();
  if (unlikely (message_block_p))
    goto continue_;

  message_inout->release (); message_inout = NULL;

  return;

error:
  message_inout->release (); message_inout = NULL;
  if (buffer_p)
  {
    result_2 = bufferQueue_.enqueue (buffer_p,
                                     NULL);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::enqueue(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  bool high_priority_b = false;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      high_priority_b = true;
      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);
      // *TODO*: support floating point formats (e.g. IEEE float) as well
      format_ =
        waveformatex_p->nChannels == 1 ? (waveformatex_p->wBitsPerSample == 8 ? AL_FORMAT_MONO8
                                                                              : AL_FORMAT_MONO16)
                                       : (waveformatex_p->wBitsPerSample == 8 ? AL_FORMAT_STEREO8
                                                                              : AL_FORMAT_STEREO16);
      sampleRate_ = waveformatex_p->nSamplesPerSec;
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      unsigned int bits_per_sample_i =
        snd_pcm_format_width (media_type_s.format);
      format_ =
        media_type_s.channels == 1 ? (bits_per_sample_i == 8 ? AL_FORMAT_MONO8
                                                             : AL_FORMAT_MONO16)
                                   : (bits_per_sample_i == 8 ? AL_FORMAT_STEREO8
                                                             : AL_FORMAT_STEREO16);
      sampleRate_ = media_type_s.rate;
#endif // ACE_WIN32 || ACE_WIN64

      alSourcePlay (source_);

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started source\n"),
                  inherited::mod_->name ()));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      if (unlikely (!queueBuffers ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to queueBuffers, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      inherited::threadCount_ = 1;
      inherited::start (NULL);
      inherited::threadCount_ = 0;

      break;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      // render remaining data ?
      if (likely (inherited::thr_count_ > 0))
      {
        if (!high_priority_b && inherited::configuration_->waitForDataOnEnd)
          while (bufferQueue_.message_count () < STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS)
            ACE_OS::sleep (ACE_Time_Value (1, 0));
        Common_ITask* itask_p = this;
        itask_p->stop (true,             // wait ?
                       high_priority_b); // high priority ?
      } // end IF
      bufferQueue_.flush ();

      if (likely (source_))
      {
retry:
        ALint iState = 0;
        alGetSourcei (source_, AL_SOURCE_STATE, &iState);
        if (iState != AL_PLAYING)
        {
          ALint iQueuedBuffers = 0;
          alGetSourcei (source_, AL_BUFFERS_QUEUED, &iQueuedBuffers);
          if (!high_priority_b &&
              inherited::configuration_->waitForDataOnEnd &&
              iQueuedBuffers)
          {
            alSourcePlay (source_);
            ACE_OS::sleep (ACE_Time_Value (1, 0));
            goto retry;
          } // end IF
        } // end IF

        alSourceStop (source_);
        alSourcei (source_, AL_BUFFER, 0);

        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stopped source\n"),
                    inherited::mod_->name ()));
      } // end IF

      alDeleteBuffers (STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS, buffers_);
      ACE_OS::memset (&buffers_, 0, sizeof (ALuint) * STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS);

      if (likely (source_))
      {
        alDeleteSources (1, &source_); source_ = 0;
      } // end IF

      if (unlikely (context_))
      {
        alcDestroyContext (context_); context_ = NULL;
      } // end IF
      if (unlikely (device_))
      {
        alcCloseDevice (device_); device_ = NULL;
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
          typename MediaType>
bool
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::queueBuffers ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::queueBuffers"));

  int result = -1;
  unsigned int i = 0;

  for (;
       i < STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS;
       ++i)
  { ACE_ASSERT (buffers_[i]);
    result = bufferQueue_.enqueue (&buffers_[i],
                                   NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::enqueue(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
  } // end FOR

  return true;

error:
  ALuint* buffer_p = NULL;
  for (unsigned int j = 0;
       j < i;
       ++j)
  {
    result = bufferQueue_.dequeue (buffer_p,
                                   NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::dequeue(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
      continue;
    } // end IF
  } // end FOR

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::stop (bool waitForCompletion_in,
                                             bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::stop"));

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result =
    (highPriority_in ? inherited::msg_queue_->enqueue_head (message_block_p, NULL)
                     : inherited::msg_queue_->enqueue_tail (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::putq(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF

  if (likely (waitForCompletion_in))
    inherited::wait ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
int
Stream_Dev_Target_OpenAL_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_OpenAL_T::svc"));

  // sanity check(s)
  ACE_ASSERT (source_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (ID: %t) starting\n"),
              inherited::mod_->name ()));

  int result = 0;
  ALint buffers_processed_i, state_i;
  ALuint buffer_i = 0;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  unsigned int i = 0;

  // process rendered buffers
  do
  {
    buffers_processed_i = 0;
    alGetSourcei (source_, AL_BUFFERS_PROCESSED, &buffers_processed_i);
    if (buffers_processed_i == 0)
    {
      alGetSourcei (source_, AL_SOURCE_STATE, &state_i);
      if (state_i != AL_PLAYING)
        alSourcePlay (source_);
    } // end IF
    while (buffers_processed_i > 0)
    {
      buffer_i = 0;
      alSourceUnqueueBuffers (source_, 1, &buffer_i);
      ACE_ASSERT (buffer_i);

      // find index of buffer_i in buffers_ and return it to the buffer queue
      for (i = 0; i < STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS; ++i)
        if (buffers_[i] == buffer_i)
          break;
      result = bufferQueue_.enqueue (&buffers_[i],
                                     NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue_Ex::enqueue(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      --buffers_processed_i;
    } // end WHILE

    result = inherited::getq (message_block_p,
                              &no_wait);
    if (unlikely (result == -1))
    {
      int error = ACE_OS::last_error ();
      if (unlikely (error != EWOULDBLOCK)) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      continue; // OK
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        message_block_p->release (); message_block_p = NULL;
        result = 0;
        goto done;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown message type (was: %d), continuing\n"),
                    inherited::mod_->name (),
                    message_block_p->msg_type ()));
        message_block_p->release (); message_block_p = NULL;
        break;
      }
    } // end SWITCH
  } while (true);
error:
  result = -1;

done:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (ID: %t) leaving\n"),
              inherited::mod_->name ()));

  return result;
}
