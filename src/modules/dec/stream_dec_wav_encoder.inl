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

#include <fstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include <xiosbase>

#include <amvideo.h>
//#include <mmiscapi.h>
#include <MMSystem.h>
#include <aviriff.h>
#include <dvdmedia.h>
#include <fourcc.h>
#include <mfobjects.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (SOX_SUPPORT)
inline static sox_bool
sox_overwrite_permitted (char const* filename_in) { ACE_UNUSED_ARG (filename_in); return sox_true; }
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            MediaType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            UserDataType>::Stream_Decoder_WAVEncoder_T (ISTREAM_T* stream_in)
#else
                            UserDataType>::Stream_Decoder_WAVEncoder_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (SOX_SUPPORT)
 , encodingInfo_ ()
 , signalInfo_ ()
 , outputFile_ (NULL)
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::Stream_Decoder_WAVEncoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (SOX_SUPPORT)
  ACE_OS::memset (&encodingInfo_, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&signalInfo_, 0, sizeof (struct sox_signalinfo_t));
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            MediaType,
                            UserDataType>::~Stream_Decoder_WAVEncoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::~Stream_Decoder_WAVEncoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (SOX_SUPPORT)
  int result = -1;
  if (outputFile_)
  {
    result = sox_close (outputFile_);
    if (unlikely (result != SOX_SUCCESS))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF

  result = sox_quit ();
  if (unlikely (result != SOX_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_quit(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
bool
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            MediaType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (SOX_SUPPORT)
    result = sox_quit ();
    if (unlikely (result != SOX_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_quit(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
      return false;
    } // end IF
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (SOX_SUPPORT)
  result = sox_init ();
  if (unlikely (result != SOX_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_init(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
    return false;
  } // end IF
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
void
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            MediaType,
                            UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::handleDataMessage"));

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!inherited::isActive_))
    return;
#else
#if defined (SOX_SUPPORT)
  if (unlikely (!outputFile_))
    return; // nothing to do
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_Message_Block* message_block_p = NULL;
  //struct _riffchunk RIFF_chunk;
#else
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (inherited::isFirst_))
  {
    inherited::isFirst_ = false;

    // sanity check(s)
    ACE_ASSERT (inherited::configuration_);
    // *TODO*: remove type inferences
    ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

    message_block_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: Stream_Decoder_AVIEncoder_WriterTask_T::allocateMessage(%d) failed: \"%m\", returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      goto error;
    } // end IF

    if (unlikely (!generateHeader (message_block_p)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_WAVEncoder_T::generateHeader(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF

    message_block_p->cont (message_inout);
  } // end IF
  else
    message_block_p = message_inout;
  ACE_ASSERT (message_block_p);

  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  message_inout = NULL;

  return;

error:
  if (message_block_p)
    message_block_p->release ();

  return;
#else
  passMessageDownstream_out = true;

#if defined (SOX_SUPPORT)
  // *IMPORTANT NOTE*: sox_write() expects signed 32-bit samples
  //                   --> convert the data in memory first
  size_t samples_read, samples_written = 0;
  /* Temporary store whilst copying. */
  sox_sample_t samples[STREAM_DEC_SOX_SAMPLE_BUFFERS];
  sox_format_t* memory_buffer_p =
      sox_open_mem_read (message_inout->rd_ptr (),
                         message_inout->length (),
                         &signalInfo_,
                         &encodingInfo_,
                         ACE_TEXT_ALWAYS_CHAR ("raw"));
  if (unlikely (!memory_buffer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_open_mem_read(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  do
  {
    samples_read = sox_read (memory_buffer_p,
                             samples,
                             STREAM_DEC_SOX_SAMPLE_BUFFERS);
    if (!samples_read)
      break;
    samples_written = sox_write (outputFile_,
                                 samples,
                                 samples_read);
    if (unlikely (samples_written != samples_read))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_write(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
  } while (true);

error:
  if (likely (memory_buffer_p))
  {
    result = sox_close (memory_buffer_p);
    if (unlikely (result != SOX_SUCCESS))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
void
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            MediaType,
                            UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!inherited::sessionData_))
    return;

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
#else
  ACE_ASSERT (inherited::sessionData_);
#endif // ACE_WIN32 || ACE_WIN64

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      if (unlikely (inherited::configuration_->targetFileName.empty ()))
        return;

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      struct Stream_MediaFramework_ALSA_MediaType& media_type_r =
          session_data_r.formats.back ();

#if defined (SOX_SUPPORT)
//      sox_comments_t comments = ;
      struct sox_oob_t oob_data;
      ACE_OS::memset (&oob_data, 0, sizeof (struct sox_oob_t));
//      oob_data.comments = comments;
//      oob_data.instr;
//      oob_data.loops;
      Stream_MediaFramework_Tools::ALSAToSoX (media_type_r.format,
                                              media_type_r.rate,
                                              media_type_r.channels,
                                              encodingInfo_,
                                              signalInfo_);
      ACE_ASSERT (!outputFile_);
      outputFile_ =
          sox_open_write (inherited::configuration_->targetFileName.c_str (),
                          &signalInfo_,
                          &encodingInfo_,
                          //ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_SOX_WAV_MediaType_STRING),
                          NULL,
                          &oob_data,
                          sox_overwrite_permitted);
      if (unlikely (!outputFile_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_open_write(\"%s\"): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->targetFileName.c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened file stream \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->targetFileName.c_str ())));
#endif // SOX_SUPPORT
      goto continue_;

error:
#if defined (SOX_SUPPORT)
      if (outputFile_)
      {
        result = sox_close (outputFile_);
        if (unlikely (result != SOX_SUCCESS))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
        outputFile_ = NULL;
      } // end IF
#endif // SOX_SUPPORT

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      bool close_file = false;
      ACE_FILE_IO file_IO;
      struct _AMMediaType media_type_s;
      unsigned char* wave_header_p = NULL;
      unsigned int wave_header_size = 0;
      ssize_t result_2 = -1;
      struct _rifflist* RIFF_wave_p = NULL;
      struct _riffchunk* RIFF_chunk_fmt_p = NULL;
      struct _riffchunk* RIFF_chunk_data_p = NULL;
      unsigned int file_size =
        Common_File_Tools::size (session_data_r.targetFileName);

      if (unlikely (session_data_r.targetFileName.empty ()))
        goto continue_2;

      if (unlikely (!Common_File_Tools::open (session_data_r.targetFileName, // FQ file name
                                              O_RDWR | O_BINARY,             // flags
                                              file_IO)))                     // return value: stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (session_data_r.targetFileName.c_str ())));
        break;
      } // end IF
      close_file = true;

      // sanity check(s)
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited::getMediaType (session_data_r.formats.back (),
                               media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));

      wave_header_size =
        (sizeof (struct _rifflist)  +
         sizeof (struct _riffchunk) +
         16                         +
         sizeof (struct _riffchunk));
      ACE_NEW_NORETURN (wave_header_p,
                        unsigned char[wave_header_size]);
      if (unlikely (!wave_header_p))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF
      result_2 = file_IO.recv_n (wave_header_p, wave_header_size);
      if (unlikely (result_2 != wave_header_size))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::recv_n(%d): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    wave_header_size));
        goto error_2;
      } // end IF

      RIFF_wave_p = reinterpret_cast<struct _rifflist*> (wave_header_p);
      RIFF_chunk_fmt_p =
        reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
      RIFF_chunk_data_p =
        reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
                                              16);

      // update RIFF header sizes
      RIFF_wave_p->cb = file_size - 8;
      RIFF_chunk_data_p->cb = file_size - 44;

      file_IO.seek (0, SEEK_SET);

      result_2 = file_IO.send_n (wave_header_p, wave_header_size);
      if (unlikely (result_2 != wave_header_size))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::send_n(%d): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    wave_header_size));
        goto error_2;
      } // end IF

      // clean up
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      delete [] wave_header_p; wave_header_p = NULL;
      result = file_IO.close ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      close_file = false;

      goto continue_2;

error_2:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      if (wave_header_p)
      {
        delete [] wave_header_p; wave_header_p = NULL;
      } // end IF
      if (close_file)
      {
        result = file_IO.close ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
      } // end IF

continue_2:
#else
#if defined (SOX_SUPPORT)
      if (outputFile_)
      {
        sox_uint64_t bytes_written = outputFile_->tell_off;
        result = sox_close (outputFile_);
        if (unlikely (result != SOX_SUCCESS))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
        outputFile_ = NULL;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed file stream \"%s\" (wrote: %Q byte(s))\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->targetFileName.c_str ()),
                    bytes_written));
      } // end IF
#endif // SOX_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    default:
      break;
  } // end SWITCH
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
bool
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            MediaType,
                            UserDataType>::generateHeader (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::generateHeader"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (messageBlock_inout);

  const SessionDataType& session_data_r = inherited::sessionData_->getR ();

  // sanity check(s)
  ACE_ASSERT (!session_data_r.formats.empty ());
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited::getMediaType (session_data_r.formats.back (),
                           media_type_s);
  ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
  //ACE_ASSERT (media_type_s.pbFormat);
  //struct tWAVEFORMATEX* waveformatex_p =
  //  reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
  //ACE_ASSERT (waveformatex_p);

  struct _rifflist* RIFF_wave_p =
    reinterpret_cast<struct _rifflist*> (messageBlock_inout->wr_ptr ());
  struct _riffchunk* RIFF_chunk_fmt_p =
    reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
  struct _riffchunk* RIFF_chunk_data_p =
    reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
                                          16);

  RIFF_wave_p->fcc = FCC ('RIFF');
  RIFF_wave_p->cb = 0 + // update here
                    (sizeof (struct _rifflist)  +
                     sizeof (struct _riffchunk) +
                     16                         +
                     sizeof (struct _riffchunk));
  RIFF_wave_p->fccListType = FCC ('WAVE');

  RIFF_chunk_fmt_p->fcc = FCC ('fmt ');
  RIFF_chunk_fmt_p->cb = 16;
  ACE_OS::memcpy (RIFF_chunk_fmt_p + 1,
                  media_type_s.pbFormat,
                  16);

  RIFF_chunk_data_p->fcc = FCC ('data');
  RIFF_chunk_data_p->cb = 0; // ... and here

  messageBlock_inout->wr_ptr (RIFF_wave_p->cb);

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  return true;
}
#endif // ACE_WIN32 || ACE_WIN64
