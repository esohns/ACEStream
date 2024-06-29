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
#include "amvideo.h"
#include "mmsystem.h"
#include "aviriff.h"
#include "dvdmedia.h"
//#include "fourcc.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#else
#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_os_tools.h"

#include "common_image_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#include "stream_lib_tools.h"
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
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       MediaType,
                                       UserDataType>::Stream_Decoder_AVIEncoder_ReaderTask_T (ISTREAM_T* stream_in)
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::Stream_Decoder_AVIEncoder_ReaderTask_T"));

  ACE_UNUSED_ARG (stream_in);
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
int
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       MediaType,
                                       UserDataType>::put (ACE_Message_Block* messageBlock_in,
                                                           ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::put"));

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_SESSION:
    {
      SessionMessageType* message_p =
        static_cast<SessionMessageType*> (messageBlock_in);

      switch (message_p->type ())
      {
        case STREAM_SESSION_MESSAGE_END:
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          const SessionDataContainerType& session_data_r = message_p->getR ();
          const SessionDataType& session_data_2 = session_data_r.getR ();

          // *TODO*: remove type inference
          if (!session_data_2.targetFileName.empty ())
            if (unlikely (!postProcessHeader (session_data_2.targetFileName)))
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader(\"%s\"), continuing\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (session_data_2.targetFileName.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
          break;
        }
        default:
          break;
      } // end SWITCH
      break;
    }
    default:
      break;
  } // end SWITCH

  return inherited::put_next (messageBlock_in, timeout_in);
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
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       MediaType,
                                       UserDataType>::postProcessHeader (const std::string& targetFilename_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader"));

  // sanity check(s)
  ACE_ASSERT (!targetFilename_in.empty ());

  bool result = false;
  WRITER_TASK_T* writer_p = static_cast<WRITER_TASK_T*> (inherited::sibling ());
  ACE_ASSERT (writer_p);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: post-processing AVI file header (was: \"%s\")\n"),
              inherited::mod_->name (),
              ACE_TEXT (targetFilename_in.c_str ())));

  // *NOTE*: things left to do (in order):
  //         - correct cb value of 'RIFF' _rifflist(s) (7)
  //         - set dwTotalFrames in _avimainheader     (1)
  //         - set dwLength in _avistreamheader(s)     (2)
  //         - set dwTotalFrames in 'dmlh'             (3)
  //         - correct cb value of 'movi' _rifflist    (4)
  //         - correct cb value of 'idx1' _rifflist    (5)
  //         [- insert version 2 (super-)index]        (6)

  ACE_UINT32 value_i = 0;
  std::streamoff read_offset = 0, write_offset = 0;
  typename WRITER_TASK_T::RIFF_OFFSETS_AND_SIZES_ITERATOR_T iterator;
  char buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (char[BUFSIZ]));
  struct _rifflist* RIFF_list_p = NULL;
  struct _riffchunk* RIFF_chunk_p = NULL;
  struct _avimainheader* AVI_header_avih_p = NULL;
  struct _avistreamheader* AVI_header_strh_p = NULL;

  std::streamoff list_movi_offset = STREAM_DEC_AVI_JUNK_CHUNK_ALIGN - 12;
  std::streamoff list_dmlh_offset = 326;
  // std::ios::streamoff chunk_idx1_offset = 0;
  // *NOTE*: "...A joint file position is maintained for both the input sequence
  //         and the output sequence. ...", i.e. std::fstream::read()/write()
  //         modify both seekg/seekp
  //        --> maintain offsets separately

  std::fstream stream (targetFilename_in.c_str (),
                       //ios_base::in | ios_base::out//,
                       std::ios::in | std::ios::out | std::ios::binary
                       /*(int)ios_base::_Openprot*/);
  if (unlikely (stream.fail ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to open file (was: \"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (targetFilename_in.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (stream.is_open ());

  stream.read (buffer_a, sizeof (struct _rifflist));
  if (stream.eof () || stream.fail ())
    goto error;
  read_offset = sizeof (struct _rifflist);
  RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
  ACE_ASSERT (RIFF_list_p->fcc == FCC ('RIFF'));
  ACE_ASSERT (RIFF_list_p->fccListType == FCC ('AVI '));
  value_i =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (RIFF_list_p->cb)
                                           : RIFF_list_p->cb);
  value_i +=
    (writer_p->lastIndex1FrameOffsetIndex_ * (sizeof (struct _riffchunk) + writer_p->videoFrameSize_)) + // see: #1225
    (writer_p->lastIndex1FrameOffsetIndex_ * sizeof (struct _avioldindex::_avioldindex_entry));     // see: #1228
  RIFF_list_p->cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  stream.seekp (write_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.write (reinterpret_cast<char*> (RIFF_list_p),
                sizeof (struct _rifflist));
  if (unlikely (stream.fail ()))
    goto error;
  stream.seekg (read_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.read (buffer_a, sizeof (struct _rifflist));
  if (unlikely (stream.eof () || stream.fail ()))
    goto error;
  read_offset += sizeof (struct _rifflist);
  RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
  ACE_ASSERT (RIFF_list_p->fccListType == FCC ('hdrl'));
  stream.read (buffer_a, sizeof (struct _avimainheader));
  if (unlikely (stream.eof () || stream.fail ()))
    goto error;
  write_offset = read_offset;
  read_offset += sizeof (struct _avimainheader);
  AVI_header_avih_p = reinterpret_cast<struct _avimainheader*> (buffer_a);
  ACE_ASSERT (AVI_header_avih_p->fcc == FCC ('avih'));
  AVI_header_avih_p->dwTotalFrames =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (writer_p->lastIndex1FrameOffsetIndex_)
                                           : writer_p->lastIndex1FrameOffsetIndex_);
  // *NOTE*: std::ofstream::write() modifies seekp
  stream.seekp (write_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.write (reinterpret_cast<char*> (AVI_header_avih_p),
                sizeof (struct _avimainheader));
  if (unlikely (stream.fail ()))
    goto error;
  stream.seekg (read_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.read (buffer_a, sizeof (struct _rifflist));
  if (unlikely (stream.eof () || stream.fail ()))
    goto error;
  read_offset += sizeof (struct _rifflist);
  RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
  ACE_ASSERT (RIFF_list_p->fccListType == FCC ('strl'));
  stream.read (buffer_a, sizeof (struct _avistreamheader));
  if (unlikely (stream.eof () || stream.fail ()))
    goto error;
  write_offset = read_offset;
  AVI_header_strh_p = reinterpret_cast<struct _avistreamheader*> (buffer_a);
  ACE_ASSERT (AVI_header_strh_p->fcc == FCC ('strh'));
  ACE_ASSERT (AVI_header_strh_p->fccType == FCC ('vids'));
  AVI_header_strh_p->dwLength =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (writer_p->lastIndex1FrameOffsetIndex_)
                                           : writer_p->lastIndex1FrameOffsetIndex_);
  stream.seekp (write_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.write (reinterpret_cast<char*> (AVI_header_strh_p),
                sizeof (struct _avistreamheader));
  if (unlikely (stream.fail ()))
    goto error;

  stream.seekg (list_dmlh_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.read (buffer_a, sizeof (struct _riffchunk));
  if (unlikely (stream.eof () || stream.fail ()))
    goto error;
  RIFF_chunk_p = reinterpret_cast<struct _riffchunk*> (buffer_a);
  ACE_ASSERT (RIFF_chunk_p->fcc == ckidAVIEXTHEADER);
  value_i = static_cast<ACE_UINT32> (writer_p->frameOffsets_.size ());
  value_i =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  write_offset = list_dmlh_offset + 4 + 4;
  stream.seekp (write_offset,
                ios::beg);
                // ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.write (reinterpret_cast<char*> (&value_i),
                sizeof (ACE_UINT32));
  if (unlikely (stream.fail ()))
    goto error;

  stream.seekg (list_movi_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.read (buffer_a, sizeof (struct _rifflist));
  if (unlikely (stream.eof () || stream.fail ()))
    goto error;
  RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
  ACE_ASSERT (RIFF_list_p->fccListType == FCC ('movi'));
  value_i =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (RIFF_list_p->cb)
                                           : RIFF_list_p->cb);
  value_i +=
    (writer_p->lastIndex1FrameOffsetIndex_ ? (writer_p->lastIndex1FrameOffsetIndex_ * (sizeof (struct _riffchunk) + writer_p->videoFrameSize_))
                                           : (writer_p->frameOffsets_.size ()       * (sizeof (struct _riffchunk) + writer_p->videoFrameSize_))); // see: #1568
  value_i +=
    (writer_p->lastIndex1AudioFrames_ ? writer_p->lastIndex1AudioFrames_ * sizeof (struct _riffchunk)
                                      : writer_p->audioFrames_ * sizeof (struct _riffchunk));
  value_i +=
    (writer_p->lastIndex1AudioSamples_ ? writer_p->lastIndex1AudioSamples_ * writer_p->audioFrameSize_
                                       : (writer_p->audioSamples_ * writer_p->audioFrameSize_));
  RIFF_list_p->cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  //chunk_idx1_offset =
  //  list_movi_offset + sizeof (struct _rifflist) + RIFF_list_p->cb;
  stream.seekp (list_movi_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.write (reinterpret_cast<char*> (RIFF_list_p),
                sizeof (struct _rifflist));
  if (unlikely (stream.fail ()))
    goto error;

  // update AVIX/movi lists
  iterator = writer_p->RIFFOffsetsAndSizes_.begin ();
  for (++iterator;
       iterator != writer_p->RIFFOffsetsAndSizes_.end ();
       ++iterator)
  {
    // update RIFF header
    read_offset = (*iterator).first;
    stream.seekg (read_offset,
                  ios::beg);
                  // ios_base::beg);
    if (unlikely (stream.fail ()))
      goto error;
    stream.read (buffer_a, sizeof (struct _rifflist));
    if (unlikely (stream.eof () || stream.fail ()))
      goto error;
    RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
    ACE_ASSERT (RIFF_list_p->fccListType == FCC ('AVIX'));
    value_i = (*iterator).second;
    RIFF_list_p->cb =
      ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                             : value_i);
    write_offset = read_offset;
    stream.seekp (write_offset,
                  ios::beg);
                  //ios_base::beg);
    if (unlikely (stream.fail ()))
      goto error;
    stream.write (reinterpret_cast<char*> (RIFF_list_p),
                  sizeof (struct _rifflist));
    if (unlikely (stream.fail ()))
      goto error;

    // update 'movi' header
    read_offset = (*iterator).first + sizeof (struct _rifflist);
    stream.seekg (read_offset,
                  ios::beg);
                  // ios_base::beg);
    if (unlikely (stream.fail ()))
      goto error;
    stream.read (buffer_a, sizeof (struct _rifflist));
    if (unlikely (stream.eof () || stream.fail ()))
      goto error;
    RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
    ACE_ASSERT (RIFF_list_p->fccListType == FCC ('movi'));
    value_i =
      (*iterator).second - (sizeof (struct _rifflist) + 4 + 4); // subtract RIFF list and 'movi' LIST head
    RIFF_list_p->cb =
      ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                             : value_i);
    write_offset = read_offset;
    stream.seekp (write_offset,
                  ios::beg);
                  //ios_base::beg);
    if (unlikely (stream.fail ()))
      goto error;
    stream.write (reinterpret_cast<char*> (RIFF_list_p),
                  sizeof (struct _rifflist));
    if (unlikely (stream.fail ()))
      goto error;
  } // end FOR

  result = true;

error:
  stream.close ();
  if (unlikely (stream.fail ()))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to close file (was: \"%s\"), continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (targetFilename_in.c_str ())));

  return result;
}
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

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
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       MediaType,
                                       UserDataType>::Stream_Decoder_AVIEncoder_WriterTask_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , isFirst_ (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
 , audioCodecContext_ (NULL)
 , videoCodecContext_ (NULL)
 , formatContext_ (NULL)
 , videoSamples_ (0)
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
 , audioSamples_ (0)
 , audioFrames_ (0)
 , audioFrameSize_ (0)
 , videoFrameSize_ (0)
 , currentOffset_ (0)
 , currentRIFFOffset_ (0)
 , headerWritten_ (false)
 , RIFFOffsetsAndSizes_ ()
 , frameOffsets_ ()
 , currentFrameOffset_ (0)
 , lastIndex1FrameOffsetIndex_ (0)
 , lastIndex1AudioFrames_ (0)
 , lastIndex1AudioSamples_ (0)
 , indexType_ (STREAM_AVI_INDEX_V1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::Stream_Decoder_AVIEncoder_WriterTask_T"));

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
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       MediaType,
                                       UserDataType>::~Stream_Decoder_AVIEncoder_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::~Stream_Decoder_AVIEncoder_WriterTask_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
  if (audioCodecContext_)
    avcodec_free_context (&audioCodecContext_);
  if (videoCodecContext_)
    avcodec_free_context (&videoCodecContext_);
  if (formatContext_)
    avformat_free_context (formatContext_);
#endif // FFMPEG_SUPPORT
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
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::initialize"));

  if (inherited::isInitialized_)
  {
    isFirst_ = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
    if (audioCodecContext_)
    {
      avcodec_free_context (&audioCodecContext_); audioCodecContext_ = NULL;
    } // end IF
    if (videoCodecContext_)
    {
      avcodec_free_context (&videoCodecContext_); videoCodecContext_ = NULL;
    } // end IF
    if (formatContext_)
    {
      avformat_free_context (formatContext_); formatContext_ = NULL;
    } // end IF
    videoSamples_ = 0;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
    audioSamples_ = 0;
    audioFrames_ = 0;
    audioFrameSize_ = 0;
    videoFrameSize_ = 0;
    currentOffset_ = 0;
    currentRIFFOffset_ = 0;
    headerWritten_ = false;
    RIFFOffsetsAndSizes_.clear ();
    frameOffsets_.clear ();
    currentFrameOffset_ = 0;
    lastIndex1FrameOffsetIndex_ = 0;
    lastIndex1AudioFrames_ = 0;
    lastIndex1AudioSamples_ = 0;
    indexType_ = STREAM_AVI_INDEX_V1;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
//  const struct AVOutputFormat* output_format_p =
//      av_guess_format (ACE_TEXT_ALWAYS_CHAR ("avi"), // short name
//                       NULL,                         // file name
//                       NULL);                        // MIME-type
//  if (unlikely (!output_format_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: av_guess_format(\"%s\") failed, aborting\n"),
//                inherited::mod_->name (),
//                ACE_TEXT ("avi")));
//    return false;
//  } // end IF
////  output_format_p->flags |= AVFMT_RAWPICTURE;
//  ACE_ASSERT (!formatContext_);
//  formatContext_ = avformat_alloc_context ();
//  if (unlikely (!formatContext_))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: avformat_alloc_context() failed, aborting\n"),
//                inherited::mod_->name ()));
//    return false;
//  } // end IF
//  formatContext_->oformat = output_format_p;
//  ACE_ASSERT (formatContext_->oformat);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  return inherited::initialize (configuration_in,
                                allocator_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//error:
//  if (formatContext_)
//  {
//    if (formatContext_->streams[0])
//      if (formatContext_->streams[0]->codec)
//      {
//        result = avcodec_close (formatContext_->streams[0]->codec);
//        if (unlikely (result == -1))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
//                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//      } // end IF

//    avformat_free_context (formatContext_); formatContext_ = NULL;
//  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  return false;
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
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along. In this case,
  //         the individual frames are encapsulated and passed as such
  passMessageDownstream_out = false;

  bool is_audio_b = false;
  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
#if defined(ACE_WIN32) || defined(ACE_WIN64)
  ACE_Message_Block* message_block_2 = NULL;
  struct _riffchunk RIFF_chunk;
  size_t avix_header_length_i = 0, offset_i = 0;
#else
  struct AVPacket packet_s = {0};
  struct AVCodecContext* codec_context_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  size_t total_length_i = message_inout->total_length ();

  switch (message_inout->getMediaType ())
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      is_audio_b = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      packet_s.stream_index = 1;
      packet_s.pts = audioSamples_;
      codec_context_p = audioCodecContext_;
      audioSamples_ += total_length_i / audioFrameSize_;
#endif // ACE_WIN32 || ACE_WIN64
      ++audioFrames_;
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    { ACE_ASSERT (total_length_i == videoFrameSize_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      packet_s.flags |= AV_PKT_FLAG_KEY;
      packet_s.pts = videoSamples_++;
      codec_context_p = videoCodecContext_;
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown media type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  message_inout->getMediaType ()));
      message_inout->release (); message_inout = NULL;
      goto error;
    }
  } // end SWITCH
  ACE_UNUSED_ARG (is_audio_b);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  message_block_p =
    inherited::allocateMessage ((isFirst_ ? inherited::configuration_->allocatorConfiguration->defaultBufferSize
                                          : sizeof (struct _riffchunk)));
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), returning\n"),
                inherited::mod_->name (),
                (isFirst_ ? inherited::configuration_->allocatorConfiguration->defaultBufferSize : sizeof (struct _riffchunk))));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF
  message_block_p->cont (message_inout); message_inout = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  if (unlikely (isFirst_))
  {
    isFirst_ = false;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    message_block_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      message_inout->release (); message_inout = NULL;
      goto error;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64
    if (unlikely (!generateHeader (message_block_p)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    headerWritten_ = true;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    RIFFOffsetsAndSizes_.push_back (std::make_pair (0, 0));
#else
    message_block_p->release (); message_block_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (message_block_p);

  // db (--> Uncompressed video frame)
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = is_audio_b ? FCC ('01wb') : FCC ('00db');
  RIFF_chunk.cb =
      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? static_cast<ACE_UINT32> (total_length_i)
                                             : ACE_SWAP_LONG (static_cast<ACE_UINT32> (total_length_i)));
  result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                  sizeof (struct _riffchunk));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
#else
  // sanity check(s)
  ACE_ASSERT (!message_inout->cont ());

//  av_init_packet (&packet_s);
  packet_s.data = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  packet_s.size = message_inout->length ();
//  packet_s.time_base =
//    formatContext_->streams[packet_s.stream_index]->time_base;
  packet_s.pts =
    av_rescale_q_rnd (packet_s.pts,
                      codec_context_p->time_base,
                      formatContext_->streams[packet_s.stream_index]->time_base,
                      static_cast<enum AVRounding> (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
  packet_s.dts =
    av_rescale_q_rnd (packet_s.dts,
                      codec_context_p->time_base,
                      formatContext_->streams[packet_s.stream_index]->time_base,
                      static_cast<enum AVRounding> (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
  packet_s.duration =
    av_rescale_q (packet_s.duration,
                  codec_context_p->time_base,
                  formatContext_->streams[packet_s.stream_index]->time_base);
//  av_packet_rescale_ts (&packet_s,
//                        codec_context_p->time_base,
//                        formatContext_->streams[packet_s.stream_index]->time_base);

  result = av_interleaved_write_frame (formatContext_, &packet_s);
//  result = av_write_frame (formatContext_, &packet_s);
//  av_packet_unref (&packet_s);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to av_interleaved_write_frame(): \"%s\", returning\n"),
//                ACE_TEXT ("%s: failed to av_write_frame(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!is_audio_b)
    frameOffsets_.push_back (currentFrameOffset_);

  if (unlikely (!is_audio_b &&
                (currentRIFFOffset_ + (sizeof (struct _riffchunk) + videoFrameSize_)) >= 1024 * 1024 * 1024)) // 1Gb
  {
    message_block_2 = inherited::allocateMessage (2 * sizeof (struct _rifflist));
    if (unlikely (!message_block_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), returning\n"),
                  inherited::mod_->name (),
                  2 * sizeof (struct _rifflist)));
      goto error;
    } // end IF
    // prepend to next frame
    message_block_2->cont (message_block_p);
    message_block_p = message_block_2;

    if (unlikely (!generateAVIXHeader (message_block_2)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateAVIXHeader(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF

    currentFrameOffset_ += message_block_2->length ();
    offset_i = currentOffset_;
    currentOffset_ += message_block_2->length ();
    avix_header_length_i =
      message_block_2->length () - sizeof (struct _riffchunk); // subtract the riff header, it does not count towards the riff chunks' size
    message_block_2 = NULL;

    if (unlikely (!lastIndex1FrameOffsetIndex_))
    {
      lastIndex1FrameOffsetIndex_ =
        static_cast<ACE_UINT32> (frameOffsets_.size () - 1); // *NOTE*: subtract one; current frame has already been added
      lastIndex1AudioFrames_ = audioFrames_;
      lastIndex1AudioSamples_ = audioSamples_;

      // *TODO*: remove type inference
      message_block_2 = inherited::allocateMessage (STREAM_DEC_AVI_INDEX_ALLOCATION_SIZE);
      if (unlikely (!message_block_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: Stream_TaskBase_T::allocateMessage(%u) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    STREAM_DEC_AVI_INDEX_ALLOCATION_SIZE));
        goto error;
      } // end IF
      // append index v1 to the last frame of first RIFF chunk --> prepend
      message_block_2->cont (message_block_p);
      message_block_p = message_block_2;

      if (unlikely (!generateIndex (STREAM_AVI_INDEX_V1,
                                    message_block_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex(%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    STREAM_AVI_INDEX_V1));
        goto error;
      } // end IF

      currentOffset_ += message_block_2->length ();
      currentRIFFOffset_ += static_cast<ACE_UINT32> (message_block_2->length ());
      currentFrameOffset_ += message_block_2->length ();
      offset_i += message_block_2->length ();
      message_block_2 = NULL;
    } // end IF

    RIFFOffsetsAndSizes_.back ().second = currentRIFFOffset_;
    RIFFOffsetsAndSizes_.push_back (std::make_pair (offset_i, 0));

    currentRIFFOffset_ = static_cast<ACE_UINT32> (avix_header_length_i);
  } // end IF

  currentOffset_ += sizeof (struct _riffchunk) + total_length_i;
  currentRIFFOffset_ += sizeof (struct _riffchunk) + static_cast<ACE_UINT32> (total_length_i);
  currentFrameOffset_ += sizeof (struct _riffchunk) + total_length_i;

  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
#else
  message_inout->release (); message_inout = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  return;

error:
  if (message_block_p)
    message_block_p->release ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  message_inout->release (); message_inout = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
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
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const SessionDataType& session_data_r = inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      struct _AMMediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
#else
#if defined (FFMPEG_SUPPORT)
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      struct Stream_MediaFramework_FFMPEG_AudioMediaType media_type_2;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_2);
      videoFrameSize_ =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        Stream_MediaFramework_DirectShow_Tools::toFramesize (media_type_s);

      audioFrameSize_ = 
        Stream_MediaFramework_DirectShow_Tools::toFramesize (media_type_2);

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: saving AVI in format %s | %s\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (media_type_s, true).c_str ()),
      //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (media_type_2, true).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);
#else
#if defined (FFMPEG_SUPPORT)
        av_image_get_buffer_size (media_type_s.format,
                                  media_type_s.resolution.width, media_type_s.resolution.height,
                                  1); // *TODO*: linesize alignment

      audioFrameSize_ =
        av_get_bytes_per_sample (media_type_2.format) * media_type_2.channels;
#else
        0; ACE_ASSERT (false); ACE_NOTSUP; // *TODO*
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
      enum AVCodecID codec_id = AV_CODEC_ID_RAWVIDEO; // RGB
      const struct AVCodec* codec_p = NULL;
      struct AVStream* stream_p = NULL;
      int result = -1;

      result =
        avformat_alloc_output_context2 (&formatContext_, // return value: format context handle
                                        NULL,            // output format handle
                                        NULL,            // format name
                                        session_data_r.targetFileName.c_str ()); // filename
      if ((result < 0) || !formatContext_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_alloc_output_context2() failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (formatContext_);
      ACE_ASSERT (formatContext_->oformat);

      // fix about global headers
      if(formatContext_->oformat->flags & AVFMT_GLOBALHEADER)
        formatContext_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

//      const_cast<struct AVOutputFormat*> (formatContext_->oformat)->audio_codec =
//        AV_CODEC_ID_NONE;
      switch (media_type_s.format)
      {
        // RGB formats
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGR24:
        case AV_PIX_FMT_ARGB:
        case AV_PIX_FMT_RGBA:
        case AV_PIX_FMT_ABGR:
        case AV_PIX_FMT_BGRA:
        case AV_PIX_FMT_RGB555BE:
        case AV_PIX_FMT_RGB555LE: // WARNING: WMP expects
        case AV_PIX_FMT_RGB565BE:
        case AV_PIX_FMT_RGB565LE:
          break;
        // luminance-chrominance formats
        case AV_PIX_FMT_UYVY422: // 'YUY2'
        case AV_PIX_FMT_YUV420P: // 'YU12'
        case AV_PIX_FMT_YUYV422:
          break;
        case AV_PIX_FMT_YUVA444P:
          codec_id = AV_CODEC_ID_AYUV;
          break;
        // compressed formats
        // *NOTE*: "... MJPEG, or at least the MJPEG in AVIs having the MJPG
        //         fourcc, is restricted JPEG with a fixed -- and *omitted* --
        //         Huffman table. The JPEG must be YCbCr colorspace, it must be
        //         4:2:2, and it must use basic Huffman encoding, not arithmetic
        //         or progressive. . . . You can indeed extract the MJPEG frames
        //         and decode them with a regular JPEG decoder, but you have to
        //         prepend the DHT segment to them, or else the decoder won't
        //         have any idea how to decompress the data. The exact table
        //         necessary is given in the OpenDML spec. ..."
        case AV_PIX_FMT_YUVJ422P:
//        case V4L2_PIX_FMT_MJPEG:
          codec_id = AV_CODEC_ID_MJPEG;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown AVI pixel format (was: %s), aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_s.format).c_str ())));
          goto error;
        }
      } // end SWITCH
//      const_cast<struct AVOutputFormat*> (formatContext_->oformat)->video_codec =
//        codec_id;

      codec_p = avcodec_find_encoder (codec_id);
      if (unlikely (!codec_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) (codec: \"%s\") failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    codec_id, ACE_TEXT (Common_Image_Tools::codecIdToString (codec_id).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (!videoCodecContext_);
      videoCodecContext_ = avcodec_alloc_context3 (codec_p);
      if (unlikely (!videoCodecContext_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
//      result = avcodec_get_context_defaults3 (codec_context_p,
//                                              codec_p);
//      if (result < 0)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: avcodec_get_context_defaults3() failed: \"%s\", aborting\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//        goto error;
//      } // end IF

      videoCodecContext_->codec_id = codec_id;
//      codec_context_p->codec_tag = MKTAG ('B', 'G', 'R', '8');
      videoCodecContext_->bit_rate = videoFrameSize_ * media_type_s.frameRate.num * 8;
//      codec_context_p->bit_rate_tolerance = 5000000;
//      codec_context_p->global_quality = 0;
//      codec_context_p->compression_level = FF_COMPRESSION_DEFAULT;
//      codec_context_p->flags = 0;
//      codec_context_p->flags2 = 0;
      // *TODO*: use something like: av_inv_q(av_d2q(frameRate, 1000));
      videoCodecContext_->time_base.num =
          (media_type_s.frameRate.den ? media_type_s.frameRate.den : 1);
      videoCodecContext_->time_base.den =
          (media_type_s.frameRate.num ? media_type_s.frameRate.num : 1);
//      codec_context_p->ticks_per_frame = 1;
      videoCodecContext_->width = media_type_s.resolution.width;
      videoCodecContext_->height = media_type_s.resolution.height;
//      codec_context_p->gop_size = 0;
      videoCodecContext_->pix_fmt = media_type_s.format;
//      codec_context_p->me_method = 1;
//      codec_context_p->max_b_frames = 0;
//      codec_context_p->b_quant_factor = -1.0F;
//      codec_context_p->b_quant_offset = -1.0F;
//      codec_context_p->mpeg_quant = 0;
//      codec_context_p->i_quant_factor = 0.0F;
//      codec_context_p->i_quant_offset = 0.0F;
//      codec_context_p->lumi_masking = 0;
//      codec_context_p->temporal_cplx_masking = 0;
//      codec_context_p->spatial_cplx_masking = 0;
//      codec_context_p->p_masking = 0.0F;
//      codec_context_p->dark_masking = 0.0F;
//      codec_context_p->prediction_method = 0;
//      codec_context_p->sample_aspect_ratio.num = 0;
//      codec_context_p->sample_aspect_ratio.den = 1;
//      codec_context_p->me_cmp = 0;
//      codec_context_p->me_sub_cmp = 0;
//      codec_context_p->mb_cmp = 0;
//      codec_context_p->ildct_cmp = 0;
//      codec_context_p->dia_size = 0;
//      codec_context_p->last_predictor_count = 0;
//      codec_context_p->pre_me = 0;
//      codec_context_p->me_pre_cmp = 0;
//      codec_context_p->pre_dia_size = 0;
//      codec_context_p->me_subpel_quality = 0;
//      codec_context_p->me_range = 0;
//      codec_context_p->intra_quant_bias = 0;
//      codec_context_p->inter_quant_bias = 0;
//      codec_context_p->mb_decision = 0;
//      codec_context_p->intra_matrix = NULL;
//      codec_context_p->inter_matrix = NULL;
//      codec_context_p->scenechange_threshold = 0;
//      codec_context_p->noise_reduction = 0;
//      codec_context_p->intra_dc_precision = 0;
//      codec_context_p->mb_lmin = 0;
//      codec_context_p->mb_lmax = 0;
//      codec_context_p->me_penalty_compensation = 0;
//      codec_context_p->bidir_refine = 0;
//      codec_context_p->brd_scale = 0;
//      codec_context_p->keyint_min = 0;
//      codec_context_p->refs = 0;
//      codec_context_p->chromaoffset = 0;
//      codec_context_p->mv0_threshold = 0;
//      codec_context_p->b_sensitivity = 0;
//      codec_context_p->color_primaries = 0;
//      codec_context_p->color_trc = 0;
//      codec_context_p->colorspace = 0;
//      codec_context_p->color_range = 0;
//      codec_context_p->chroma_sample_location = 0;
//      codec_context_p->slices = 0;
//      codec_context_p->qmin = 0;
//      codec_context_p->qmax = 0;
//      codec_context_p->max_qdiff = 0;
//      codec_context_p->rc_buffer_size = 0;
//      codec_context_p->rc_override_count = 0;
//      codec_context_p->rc_override = NULL;
//      codec_context_p->rc_max_rate = 0;
//      codec_context_p->rc_min_rate = 0;
//      codec_context_p->rc_max_available_vbv_use = 0.0F;
//      codec_context_p->rc_min_vbv_overflow_use = 0.0F;
//      codec_context_p->rc_initial_buffer_occupancy = 0;
//      codec_context_p->coder_type = 2;
//      codec_context_p->context_model = 0;
//      codec_context_p->frame_skip_threshold = 0;
//      codec_context_p->frame_skip_factor = 0;
//      codec_context_p->frame_skip_exp = 0;
//      codec_context_p->frame_skip_cmp = 0;
//      codec_context_p->trellis = 0;
//      codec_context_p->min_prediction_order = 0;
//      codec_context_p->max_prediction_order = 0;
//      codec_context_p->timecode_frame_start = 0;
//      codec_context_p->stats_in = NULL;
      videoCodecContext_->workaround_bugs = FF_BUG_AUTODETECT;
      videoCodecContext_->strict_std_compliance = FF_COMPLIANCE_NORMAL;
//      codec_context_p->debug = 0;
//      codec_context_p->debug_mv = 0;
//      codec_context_p->dct_algo = FF_DCT_AUTO;
//      codec_context_p->idct_algo = FF_IDCT_AUTO;
//      videoCodecContext_->bits_per_coded_sample =
//        Stream_MediaFramework_Tools::ffmpegFormatToBitDepth (media_type_s.format);
//      codec_context_p->bits_per_raw_sample = 8;
//      codec_context_p->thread_count = 0;
//      codec_context_p->thread_type = 0;
//      codec_context_p->thread_safe_callbacks = 0;
//      codec_context_p->nsse_weight = 0;
//      codec_context_p->profile = FF_PROFILE_UNKNOWN;
//      codec_context_p->level = FF_LEVEL_UNKNOWN;
//      codec_context_p->side_data_only_packets = 1;
//      codec_context_p->chroma_intra_matrix = NULL;
//      codec_context_p->dump_separator = NULL;

      result = avcodec_open2 (videoCodecContext_,
                              videoCodecContext_->codec,
                              NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_open2(%d) failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    codec_id,
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      ACE_ASSERT (!formatContext_->streams);
      stream_p = avformat_new_stream (formatContext_,
                                      videoCodecContext_->codec);
      if (unlikely (!stream_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (stream_p->codecpar);
      formatContext_->streams[0] = stream_p;
      stream_p->time_base = videoCodecContext_->time_base;
      stream_p->avg_frame_rate = media_type_s.frameRate;
      stream_p->codecpar = avcodec_parameters_alloc ();
      avcodec_parameters_from_context (stream_p->codecpar,
                                       videoCodecContext_);

      // audio
      codec_id =
        av_get_pcm_codec (media_type_2.format,
                          ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? 0 : 1));
      codec_p = avcodec_find_encoder (codec_id);
      if (unlikely (!codec_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) (codec: \"%s\") failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    codec_id, ACE_TEXT (Common_Image_Tools::codecIdToString (codec_id).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (!audioCodecContext_);
      audioCodecContext_ = avcodec_alloc_context3 (codec_p);
      if (unlikely (!audioCodecContext_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      audioCodecContext_->codec_id = codec_id;
      //      codec_context_p->codec_tag = MKTAG ('B', 'G', 'R', '8');
      audioCodecContext_->bit_rate =
        audioFrameSize_ * media_type_2.sampleRate * 8;
      //      codec_context_p->bit_rate_tolerance = 5000000;
      //      codec_context_p->global_quality = 0;
      //      codec_context_p->compression_level = FF_COMPRESSION_DEFAULT;
      //      codec_context_p->flags = 0;
      //      codec_context_p->flags2 = 0;
      // *TODO*: use something like: av_inv_q(av_d2q(frameRate, 1000));
      audioCodecContext_->time_base.num = 1;
      audioCodecContext_->time_base.den =
        (media_type_2.sampleRate ? media_type_2.sampleRate : 48000);
      //      codec_context_p->ticks_per_frame = 1;
//      audioCodecContext_->width = media_type_s.resolution.width;
//      audioCodecContext_->height = media_type_s.resolution.height;
      //      codec_context_p->gop_size = 0;
//      audioCodecContext_->pix_fmt = media_type_s.format;
      //      codec_context_p->me_method = 1;
      //      codec_context_p->max_b_frames = 0;
      //      codec_context_p->b_quant_factor = -1.0F;
      //      codec_context_p->b_quant_offset = -1.0F;
      //      codec_context_p->mpeg_quant = 0;
      //      codec_context_p->i_quant_factor = 0.0F;
      //      codec_context_p->i_quant_offset = 0.0F;
      //      codec_context_p->lumi_masking = 0;
      //      codec_context_p->temporal_cplx_masking = 0;
      //      codec_context_p->spatial_cplx_masking = 0;
      //      codec_context_p->p_masking = 0.0F;
      //      codec_context_p->dark_masking = 0.0F;
      //      codec_context_p->prediction_method = 0;
      //      codec_context_p->sample_aspect_ratio.num = 0;
      //      codec_context_p->sample_aspect_ratio.den = 1;
      //      codec_context_p->me_cmp = 0;
      //      codec_context_p->me_sub_cmp = 0;
      //      codec_context_p->mb_cmp = 0;
      //      codec_context_p->ildct_cmp = 0;
      //      codec_context_p->dia_size = 0;
      //      codec_context_p->last_predictor_count = 0;
      //      codec_context_p->pre_me = 0;
      //      codec_context_p->me_pre_cmp = 0;
      //      codec_context_p->pre_dia_size = 0;
      //      codec_context_p->me_subpel_quality = 0;
      //      codec_context_p->me_range = 0;
      //      codec_context_p->intra_quant_bias = 0;
      //      codec_context_p->inter_quant_bias = 0;
      //      codec_context_p->mb_decision = 0;
      //      codec_context_p->intra_matrix = NULL;
      //      codec_context_p->inter_matrix = NULL;
      //      codec_context_p->scenechange_threshold = 0;
      //      codec_context_p->noise_reduction = 0;
      //      codec_context_p->intra_dc_precision = 0;
      //      codec_context_p->mb_lmin = 0;
      //      codec_context_p->mb_lmax = 0;
      //      codec_context_p->me_penalty_compensation = 0;
      //      codec_context_p->bidir_refine = 0;
      //      codec_context_p->brd_scale = 0;
      //      codec_context_p->keyint_min = 0;
      //      codec_context_p->refs = 0;
      //      codec_context_p->chromaoffset = 0;
      //      codec_context_p->mv0_threshold = 0;
      //      codec_context_p->b_sensitivity = 0;
      //      codec_context_p->color_primaries = 0;
      //      codec_context_p->color_trc = 0;
      //      codec_context_p->colorspace = 0;
      //      codec_context_p->color_range = 0;
      //      codec_context_p->chroma_sample_location = 0;
      //      codec_context_p->slices = 0;
      //      codec_context_p->qmin = 0;
      //      codec_context_p->qmax = 0;
      //      codec_context_p->max_qdiff = 0;
      //      codec_context_p->rc_buffer_size = 0;
      //      codec_context_p->rc_override_count = 0;
      //      codec_context_p->rc_override = NULL;
      //      codec_context_p->rc_max_rate = 0;
      //      codec_context_p->rc_min_rate = 0;
      //      codec_context_p->rc_max_available_vbv_use = 0.0F;
      //      codec_context_p->rc_min_vbv_overflow_use = 0.0F;
      //      codec_context_p->rc_initial_buffer_occupancy = 0;
      //      codec_context_p->coder_type = 2;
      //      codec_context_p->context_model = 0;
      //      codec_context_p->frame_skip_threshold = 0;
      //      codec_context_p->frame_skip_factor = 0;
      //      codec_context_p->frame_skip_exp = 0;
      //      codec_context_p->frame_skip_cmp = 0;
      //      codec_context_p->trellis = 0;
      //      codec_context_p->min_prediction_order = 0;
      //      codec_context_p->max_prediction_order = 0;
      //      codec_context_p->timecode_frame_start = 0;
      //      codec_context_p->stats_in = NULL;
      audioCodecContext_->workaround_bugs = FF_BUG_AUTODETECT;
      audioCodecContext_->strict_std_compliance = FF_COMPLIANCE_NORMAL;
      //      codec_context_p->debug = 0;
      //      codec_context_p->debug_mv = 0;
      //      codec_context_p->dct_algo = FF_DCT_AUTO;
      //      codec_context_p->idct_algo = FF_IDCT_AUTO;
      audioCodecContext_->bits_per_coded_sample = audioFrameSize_;
      audioCodecContext_->bits_per_raw_sample =
        av_get_bytes_per_sample (media_type_2.format) * 8;
      //      codec_context_p->thread_count = 0;
      //      codec_context_p->thread_type = 0;
      //      codec_context_p->thread_safe_callbacks = 0;
      //      codec_context_p->nsse_weight = 0;
      //      codec_context_p->profile = FF_PROFILE_UNKNOWN;
      //      codec_context_p->level = FF_LEVEL_UNKNOWN;
      //      codec_context_p->side_data_only_packets = 1;
      //      codec_context_p->chroma_intra_matrix = NULL;
      //      codec_context_p->dump_separator = NULL;
      audioCodecContext_->channels =
        (media_type_2.channels ? media_type_2.channels : 2);
      audioCodecContext_->sample_fmt = media_type_2.format;
      audioCodecContext_->sample_rate =
        (media_type_2.sampleRate ? media_type_2.sampleRate : 48000);
      switch (media_type_2.channels)
      {
        case 1:
          audioCodecContext_->channel_layout = AV_CH_LAYOUT_MONO;
          break;
        case 0: // i.e. N/A
        case 2:
          audioCodecContext_->channel_layout = AV_CH_LAYOUT_STEREO;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown #channels (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      media_type_2.channels));
          goto error;
        }
      } // end SWITCH

      result = avcodec_open2 (audioCodecContext_,
                              audioCodecContext_->codec,
                              NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_open2(%d) failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    codec_id,
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      stream_p = avformat_new_stream (formatContext_,
                                      audioCodecContext_->codec);
      if (unlikely (!stream_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      //ACE_ASSERT (stream_p->codecpar);
      formatContext_->streams[1] = stream_p;

      stream_p->time_base = audioCodecContext_->time_base;
      avcodec_parameters_from_context (stream_p->codecpar,
                                       audioCodecContext_);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

      goto continue_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
error:
#if defined (FFMPEG_SUPPORT)
      if (audioCodecContext_)
      {
        avcodec_free_context (&audioCodecContext_); audioCodecContext_ = NULL;
      } // end IF
      if (videoCodecContext_)
      {
        avcodec_free_context (&videoCodecContext_); videoCodecContext_ = NULL;
      } // end IF
      if (formatContext_)
      {
        avformat_free_context (formatContext_); formatContext_ = NULL;
      } // end IF
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
//      ACE_ASSERT (false); // *TODO*
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (RIFFOffsetsAndSizes_.size () > 1)
        RIFFOffsetsAndSizes_.back ().second = currentRIFFOffset_;
#else
#if defined (FFMPEG_SUPPORT)
      if (likely (formatContext_) && headerWritten_)
      {
        int result = av_write_trailer (formatContext_);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: av_write_trailer() failed: \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        avio_flush (formatContext_->pb);
        avio_closep (&formatContext_->pb);
      } // end IF
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_inout);
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (!session_data_r.formats.empty ());

  // *NOTE*: need to reclaim this memory (see below)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited2::getMediaType (session_data_r.formats.back (),
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_s);
  struct _AMMediaType media_type_2;
  ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
  inherited2::getMediaType (session_data_r.formats.back (),
                            STREAM_MEDIATYPE_AUDIO,
                            media_type_2);
  struct _riffchunk RIFF_chunk;
  struct _rifflist RIFF_list;
  struct _avimainheader AVI_header_avih;
  struct _avistreamheader AVI_header_strh;
  //FOURCCMap fourcc_map;
  struct tagBITMAPINFOHEADER AVI_header_strf_video;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  struct tWAVEFORMATEX* wave_format_ex_p = NULL;
  ACE_UINT32 value_i = 0;
  ACE_INT16 value_2 = 0;
  struct tWAVEFORMATEX AVI_header_strf_audio; // dummy

  if (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo))
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_s.pbFormat);
  else if (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo2))
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_s.pbFormat);
  else
  { // --> no video *TODO*: support audio-only streams
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown media format type (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_OS_Tools::GUIDToString (media_type_s.formattype).c_str ())));
    goto error;
  } // end ELSE

  if (InlineIsEqualGUID (media_type_2.formattype, FORMAT_WaveFormatEx))
    wave_format_ex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_2.pbFormat);
  else
  { // --> no audio *TODO*: write only one strl in this case; adjust offset(s), pad bytes accordingly
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: invalid/unknown media format type (was: \"%s\"), continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_OS_Tools::GUIDToString (media_type_2.formattype).c_str ())));
  } // end ELSE

  // RIFF
  ACE_OS::memset (&RIFF_list, 0, sizeof (struct _rifflist));
  RIFF_list.fcc = FCC ('RIFF');                   //                     [0]
  // sizeof (fccListType) [4] + sizeof (RIFF) [== total (file) size - 8]
  value_i = 4                                   + // 'AVI '              [12]
            // sizeof (LIST hdrl)
            sizeof (struct _rifflist)           + // 'hdrl'              [12]
            sizeof (struct _avimainheader)      + // 'avih'              [16x4=64]
            // sizeof (LIST strl)
            sizeof (struct _rifflist)           + // 'strl'              [12]
            sizeof (struct _avistreamheader)    + // 'strh' / 'vids'     [16x4=64]
            sizeof (struct _riffchunk)          + // 'strf'              [8]
            sizeof (struct tagBITMAPINFOHEADER) + //                     [10x4=40]
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
            sizeof (struct _rifflist)           + // 'strl'              [12]
            sizeof (struct _avistreamheader)    + // 'strh' / 'auds'     [16x4=64]
            sizeof (struct _riffchunk)          + // 'strf'              [8]
            sizeof (struct tWAVEFORMATEX)       + //                     [18]
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
            //sizeof (struct _riffchunk)          + // 'indx' // *TODO*: index version 2.0; post-processing
            sizeof (struct _rifflist)           + // 'odml'              [12]
            sizeof (struct _riffchunk)          + // 'dmlh'              [8]
            4                                   + // dwTotalFrames       [4]
            sizeof (struct _riffchunk)          + // 'JUNK'              [8] = 346
            1690                                + // pad bytes           [1690]
            // sizeof (LIST movi)
            sizeof (struct _rifflist)           + // 'movi'              [12]
            //sizeof (struct _riffchunk)        + // 'ix00' // *TODO*: index version 2.0; post-processing
            //(sizeof (struct _riffchunk) + videoFrameSize_) * #frames + // '00db'; post-processing
            sizeof (struct _riffchunk);           // 'idx1'
            //sizeof (struct _avioldindex::_avioldindex_entry) * #frames; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('AVI ');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     4 + 4 + 4);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // hdrl
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (avih) + sizeof (LIST strl)
  value_i = 4                                   + // 'hdrl'
            sizeof (struct _avimainheader)      + // 'avih'
            // sizeof (LIST strl)
            sizeof (struct _rifflist)           + // 'strl'
            sizeof (struct _avistreamheader)    + // 'strh' / 'vids'
            sizeof (struct _riffchunk)          + // 'strf'
            sizeof (struct tagBITMAPINFOHEADER) +
            sizeof (struct _rifflist)           + // 'strl'
            sizeof (struct _avistreamheader)    + // 'strh' / 'auds'
            sizeof (struct _riffchunk)          + // 'strf'
            sizeof (struct tWAVEFORMATEX);
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
            //sizeof (struct _riffchunk)          + // 'indx' // *TODO*: index version 2.0; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('hdrl');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     4 + 4 + 4);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // *NOTE*: "...the 'hdrl' list begins with the main AVI header, which is
  //         contained in an 'avih' chunk. ..."
  ACE_OS::memset (&AVI_header_avih, 0, sizeof (struct _avimainheader));
  AVI_header_avih.fcc = ckidMAINAVIHEADER;
  value_i = sizeof (struct _avimainheader) - 8;
  AVI_header_avih.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  // *NOTE*: tagVIDEOINFOHEADER.AvgTimePerFrame gives 100ns units
  value_i = 
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame / 10)
                                                                   : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame / 10));
  AVI_header_avih.dwMicroSecPerFrame =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? video_info_header_p->dwBitRate / 8
                                                                   : video_info_header2_p->dwBitRate) / 8;
  AVI_header_avih.dwMaxBytesPerSec =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  AVI_header_avih.dwPaddingGranularity =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (STREAM_DEC_AVI_JUNK_CHUNK_ALIGN)
                                           : STREAM_DEC_AVI_JUNK_CHUNK_ALIGN);
  value_i = (AVIF_HASINDEX       |
             AVIF_MUSTUSEINDEX   |
             AVIF_WASCAPTUREFILE);
  AVI_header_avih.dwFlags = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  //AVI_header_avih.dwTotalFrames = 0; // post-processing; unreliable
  //AVI_header_avih.dwInitialFrames = 0;
  value_i = 2;
  AVI_header_avih.dwStreams =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = 8 + videoFrameSize_; // *NOTE*: unreliable
  AVI_header_avih.dwSuggestedBufferSize =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = 
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biWidth
                                                                   : video_info_header2_p->bmiHeader.biWidth);
  AVI_header_avih.dwWidth =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biHeight
                                                                   : video_info_header2_p->bmiHeader.biHeight);
  AVI_header_avih.dwHeight =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  //AVI_header_avih.dwReserved = {0, 0, 0, 0};
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_avih),
                              (12 * 4) + (4 * 4));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // *NOTE*: "One or more 'strl' lists follow the main header. A 'strl' list
  //         is required for each data stream. Each 'strl' list contains
  //         information about one stream in the file, and must contain a
  //         stream header chunk ('strh') and a stream format chunk ('strf').
  //         ..."
  // strl (video)
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST strl)
  value_i = 4                                 +  // 'strl'
            sizeof (struct _avistreamheader)  +  // 'strh' / 'vids'
            sizeof (struct _riffchunk)        +  // 'strf'
            sizeof (struct tagBITMAPINFOHEADER);
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = ckidSTREAMLIST;
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     4 + 4 + 4);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // strl --> strh
  ACE_OS::memset (&AVI_header_strh, 0, sizeof (struct _avistreamheader));
  AVI_header_strh.fcc = ckidSTREAMHEADER;
  value_i = sizeof (struct _avistreamheader) - 8;
  AVI_header_strh.cb = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  AVI_header_strh.fccType = streamtypeVIDEO;
  // *NOTE*: BGR(!)24 --> 0
  //fourcc_map.SetFOURCC (&media_type_s.subtype);
  //AVI_header_strh.fccHandler = fourcc_map.GetFOURCC ();
  //AVI_header_strh.fccHandler = FCC ('raw ');
  //AVI_header_strh.dwFlags = 0;
  //AVI_header_strh.wPriority = 0;
  //AVI_header_strh.wLanguage = 0;
  //AVI_header_strh.dwInitialFrames = 0;
  // *NOTE*: dwRate / dwScale == fps
  value_i = 10000; // 100th nanoseconds --> milliseconds
  AVI_header_strh.dwScale =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
                                                                   : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame));
  AVI_header_strh.dwRate =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  //AVI_header_strh.dwStart = 0;
  //AVI_header_strh.dwLength = 0; // post-prcessing
  value_i = 8 + videoFrameSize_; // *NOTE*: unreliable
  AVI_header_strh.dwSuggestedBufferSize = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = -1; // default
  AVI_header_strh.dwQuality = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = videoFrameSize_;
  AVI_header_strh.dwSampleSize = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<ACE_INT16> (video_info_header_p->rcTarget.left)
                                                                   : static_cast<ACE_INT16> (video_info_header2_p->rcTarget.left));
  AVI_header_strh.rcFrame.left =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<ACE_INT16> (video_info_header_p->rcTarget.right)
                                                                   : static_cast<ACE_INT16> (video_info_header2_p->rcTarget.right));
  AVI_header_strh.rcFrame.right =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<ACE_INT16> (video_info_header_p->rcTarget.top)
                                                                   : static_cast<ACE_INT16> (video_info_header2_p->rcTarget.top));
  AVI_header_strh.rcFrame.top =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<ACE_INT16> (video_info_header_p->rcTarget.bottom)
                                                                   : static_cast<ACE_INT16> (video_info_header2_p->rcTarget.bottom));
  AVI_header_strh.rcFrame.bottom =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strh),
                              (5 * 4) + (2 * 2) + (8 * 4) + (4 * 2));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // strl --> strf
  // *NOTE*: there is no definition for AVI stream chunk format; their content
  //         differs, depending on the type of stream
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = ckidSTREAMFORMAT;
  value_i = sizeof (struct tagBITMAPINFOHEADER);
  RIFF_chunk.cb = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                     4 + 4);
  ACE_OS::memset (&AVI_header_strf_video, 0, sizeof (struct tagBITMAPINFOHEADER));
  AVI_header_strf_video =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? video_info_header_p->bmiHeader
                                                                   : video_info_header2_p->bmiHeader);
  // *NOTE*: the negative header size is microsoft-specific rubbish (specifies
  //         bottom-up versus top-down scanlines). Also, it seems vlc cannot
  //         handle it properly
  //AVI_header_strf_video.biHeight = -AVI_header_strf_video.biHeight;
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strf_video),
                              (3 * 4) + (2 * 2) + (6 * 4));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // strl --> strd *TODO*
  // strl --> strn *TODO*
  // strl --> indx *TODO*

  // --> END strl (video)

  // strl (audio)
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST strl)
  value_i = 4                                 +  // 'strl'
            sizeof (struct _avistreamheader)  +  // 'strh' / 'auds'
            sizeof (struct _riffchunk)        +  // 'strf'
            sizeof (struct tWAVEFORMATEX);
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
            //sizeof (struct _riffchunk);           // 'indx' // *TODO*: index version 2.0; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = ckidSTREAMLIST;
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     4 + 4 + 4);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // strl --> strh
  ACE_OS::memset (&AVI_header_strh, 0, sizeof (struct _avistreamheader));
  AVI_header_strh.fcc = ckidSTREAMHEADER;
  value_i = sizeof (struct _avistreamheader) - 8;
  AVI_header_strh.cb = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  AVI_header_strh.fccType = streamtypeAUDIO;
  // *NOTE*: PCM --> 0
  //fourcc_map.SetFOURCC (&media_type_2.subtype);
  //AVI_header_strh.fccHandler = fourcc_map.GetFOURCC ();
  //AVI_header_strh.dwFlags = 0;
  //AVI_header_strh.wPriority = 0;
  //AVI_header_strh.wLanguage = 0;
  //AVI_header_strh.dwInitialFrames = 0;
  // *NOTE*: dwRate / dwScale == sps
  value_i = 1;
  AVI_header_strh.dwScale =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i =
    (wave_format_ex_p ? wave_format_ex_p->nSamplesPerSec : 48000);
  AVI_header_strh.dwRate =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  //AVI_header_strh.dwStart = 0;
  //AVI_header_strh.dwLength = 0; // post-prcessing
  value_i =
    8 + STREAM_DEC_AVI_AUDIO_STRH_SUGGESTED_BUFFER_SIZE; // *NOTE*: unreliable
  AVI_header_strh.dwSuggestedBufferSize =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = -1; // default
  AVI_header_strh.dwQuality = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i =
    (wave_format_ex_p ? (wave_format_ex_p->wBitsPerSample / 8) * wave_format_ex_p->nChannels
                      : 4);
  AVI_header_strh.dwSampleSize =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  //AVI_header_strh.rcFrame.left = 0;
  //AVI_header_strh.rcFrame.right = 0;
  //AVI_header_strh.rcFrame.top = 0;
  //AVI_header_strh.rcFrame.bottom = 0;
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strh),
                              (5 * 4) + (2 * 2) + (8 * 4) + (4 * 2));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // strl --> strf
  // *NOTE*: there is no definition for AVI stream chunk format; their content
  //         differs, depending on the type of stream
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = ckidSTREAMFORMAT;
  value_i = sizeof (struct tWAVEFORMATEX);
  RIFF_chunk.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                     4 + 4);
  ACE_OS::memset (&AVI_header_strf_audio, 0, sizeof (struct tWAVEFORMATEX));
  AVI_header_strf_audio.cbSize = sizeof (struct tWAVEFORMATEX);
  AVI_header_strf_audio.wFormatTag = WAVE_FORMAT_PCM;
  AVI_header_strf_audio.nChannels = 2;
  AVI_header_strf_audio.nSamplesPerSec = 48000;
  AVI_header_strf_audio.wBitsPerSample = 16;
  AVI_header_strf_audio.nBlockAlign =
    (AVI_header_strf_audio.wBitsPerSample / 8) * AVI_header_strf_audio.nChannels;
  AVI_header_strf_audio.nAvgBytesPerSec =
    AVI_header_strf_audio.nSamplesPerSec * AVI_header_strf_audio.nBlockAlign;
  wave_format_ex_p =
    (wave_format_ex_p ? wave_format_ex_p : &AVI_header_strf_audio);
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (wave_format_ex_p),
                              (2 * 2) + (2 * 4) + (3 * 2));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // strl --> strd *TODO*
  // strl --> strn *TODO*
  // strl --> indx *TODO*

  // --> END strl (audio)

  //if (indexType_ == STREAM_AVI_INDEX_V2)
  //{
    // odml
    RIFF_list.fcc = FCC ('LIST');
    // sizeof (fccListType) [4] + sizeof (LIST odml)
    value_i = 4 +                          // 'odml'
              sizeof (struct _riffchunk) + // 'dmlh'
              4;                           // 'dwTotalFrames'
    RIFF_list.cb =
      ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                             : value_i);
    RIFF_list.fccListType = ckidODML;
    result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                       (4 + 4 + 4));
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    RIFF_chunk.fcc = ckidAVIEXTHEADER;
    value_i = 4; // 'dwTotalFrames' *TODO*: use struct _aviextheader here ?
    RIFF_chunk.cb =
      ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                             : value_i);
    result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                       (4 + 4));
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    ACE_OS::memset (messageBlock_inout->wr_ptr (), 0, value_i);
    messageBlock_inout->wr_ptr (value_i);
  //} // end IF

  // insert JUNK chunk to align the 'movi' list to 2048 bytes
  // --> should speed up CD-ROM access
  value_i = static_cast<ACE_UINT32> (AVI_header_avih.dwPaddingGranularity -
                                     (messageBlock_inout->length () + 20));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: inserting JUNK chunk (%u pad byte(s))\n"),
  //            inherited::mod_->name (),
  //            value_i));
  RIFF_chunk.fcc = FCC ('JUNK');
  RIFF_chunk.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                     (4 + 4));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  ACE_OS::memset (messageBlock_inout->wr_ptr (), 0, value_i);
  messageBlock_inout->wr_ptr (value_i);

  // movi
  // *NOTE*: this is the origin of the frame index; increment the corresponding
  //         offsets from here
  ACE_ASSERT (!currentFrameOffset_ && frameOffsets_.empty ());
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST movi)
  value_i = 4;               //                                   +
            //sizeof (struct _riffchunk) + ???                    + // 'ix00' // *TODO*: index version 2.0; post-processing
            //(sizeof (struct _riffchunk) + videoFrameSize_) * #frames + // '00db'; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('movi');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     (4 + 4 + 4));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  currentOffset_ = messageBlock_inout->length ();
  currentRIFFOffset_ = static_cast<ACE_UINT32> (messageBlock_inout->length ());
  currentFrameOffset_ = (4 + 4 + 4);

  // clean up
  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  goto continue_2;

error:
  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  return false;

continue_2:
#else
#if defined (FFMPEG_SUPPORT)
  ACE_ASSERT (!formatContext_->pb);
  result = avio_open (&formatContext_->pb,
                      session_data_r.targetFileName.c_str (),
                      AVIO_FLAG_WRITE);
//  formatContext_->pb =
//    avio_alloc_context (reinterpret_cast<unsigned char*> (messageBlock_inout->wr_ptr ()), // buffer handle
//                        messageBlock_inout->capacity (),                                  // buffer size
//                        1,                                                                // write flag
//                        messageBlock_inout,                                               // act
//                        NULL,                                                             // read callback
//                        stream_decoder_aviencoder_libav_write_cb,                         // write callback
//                        NULL);                                                            // seek callback
  if (unlikely (result < 0 || !formatContext_->pb))
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: avio_alloc_context() failed: \"%m\", aborting\n"),
//                inherited::mod_->name ()));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avio_open() failed: \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  // *NOTE*: on windows, vlc does not show an image at all, unless this is set (the images appear upside down though :-()
  // *TODO*: is this required on linux too ?
  result =
    av_opt_set (formatContext_->priv_data,
                ACE_TEXT_ALWAYS_CHAR ("flipped_raw_rgb"), ACE_TEXT_ALWAYS_CHAR ("true"),
                0);
  ACE_ASSERT (result >= 0);

  try {
    result = avformat_write_header (formatContext_, // context handle
                                    NULL);          // options
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in avformat_write_header(), continuing\n"),
                inherited::mod_->name ()));
  }
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avformat_write_header() failed: \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  avio_flush (formatContext_->pb);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  return true;
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
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       MediaType,
                                       UserDataType>::generateAVIXHeader (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateAVIXHeader"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_inout);

  int result = -1;
  struct _rifflist RIFF_list;
  ACE_OS::memset (&RIFF_list, 0, sizeof (struct _rifflist));
  // RIFF
  RIFF_list.fcc = FCC ('RIFF');
  ACE_UINT32 value_i = 0; // 4 + sizeof (struct _rifflist) + ((sizeof (struct _riffchunk) + videoFrameSize_) * #frames) // '00db'; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('AVIX');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     sizeof (struct _rifflist));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // movi
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST movi)
  value_i = 0; // 4 +                                                  // 'movi'; post-processing
               //sizeof (struct _riffchunk) + ???                    + // 'ix00' // *TODO*: index version 2.0; post-processing
               //(sizeof (struct _riffchunk) + videoFrameSize_) * #frames + // '00db'; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('movi');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     (4 + 4 + 4));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}
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
bool
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       MediaType,
                                       UserDataType>::generateIndex (enum Stream_Decoder_AVIIndexType indexType_in,
                                                                     ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_inout);

  int result = -1;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (indexType_in)
  {
    case STREAM_AVI_INDEX_V1:
    {
      struct _avioldindex AVI_header_index;
      ACE_UINT32 value_i = 0;
      ACE_OS::memset (&AVI_header_index, 0, sizeof (struct _avioldindex));
      AVI_header_index.fcc = ckidAVIOLDINDEX;
      value_i = static_cast<ACE_UINT32> ((4 * 4) * frameOffsets_.size ());
      AVI_header_index.cb =
        ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                               : value_i);
      result =
        messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_index),
                                  (4 + 4));
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    (4 + 4)));
        return false;
      } // end IF
      struct _avioldindex::_avioldindex_entry _avioldindex_entry_s;
      for (FRAMEOFFSETSITERATOR_T iterator = frameOffsets_.begin ();
           iterator != frameOffsets_.end ();
           ++iterator)
      {
        ACE_OS::memset (&_avioldindex_entry_s, 0, sizeof (struct _avioldindex::_avioldindex_entry));
        _avioldindex_entry_s.dwChunkId = FCC ('00db');
        _avioldindex_entry_s.dwFlags = (AVIIF_KEYFRAME// |
                                        //AVIIF_LIST     |
                                        /*AVIIF_NO_TIME*/);
        value_i = static_cast<ACE_UINT32> (*iterator);
        _avioldindex_entry_s.dwOffset =
          ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                                 : value_i);
        value_i = videoFrameSize_;
        _avioldindex_entry_s.dwSize =
          ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                                 : value_i);
        result =
          messageBlock_inout->copy (reinterpret_cast<char*> (&_avioldindex_entry_s),
                                    (4 * 4));
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      (4 * 4)));
          return false;
        } // end IF
      } // end FOR

      break;
    }
    case STREAM_AVI_INDEX_V2:
    {
      ACE_ASSERT (false); // *TODO*
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
      //struct _avisuperindex AVI_header_index;
      //ACE_OS::memset (&AVI_header_index, 0, sizeof (struct _avisuperindex));
      //  AVI_header_index.fcc = 0;
      //  AVI_header_index.cb = 0;
      //  AVI_header_index.wLongsPerEntry = 0;
      //  AVI_header_index.bIndexSubType = 0;
      //  AVI_header_index.bIndexType = 0;
      //  AVI_header_index.nEntriesInUse = 0;
      //  AVI_header_index.dwChunkId = 0;
      //  AVI_header_index.dwReserved = 0;
      //  AVI_header_index.aIndex = 0;

      //  struct _avisuperindex_entry AVI_header_index_entry_0;
      //  ACE_OS::memset (&AVI_header_index_entry_0, 0, sizeof (struct _avisuperindex_entry));
      //  struct _avisuperindex_entry AVI_header_index_entry_1;
      //  ACE_OS::memset (&AVI_header_index_entry_1, 0, sizeof (struct _avisuperindex_entry));
      //  struct _avisuperindex_entry AVI_header_index_entry_2;
      //  ACE_OS::memset (&AVI_header_index_entry_2, 0, sizeof (struct _avisuperindex_entry));
      //  struct _avisuperindex_entry AVI_header_index_entry_3;
      //  ACE_OS::memset (&AVI_header_index_entry_3, 0, sizeof (struct _avisuperindex_entry));
      //result =
      //  messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_index),
      //                            sizeof (struct _avioldindex));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown index type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  indexType_in));
      return false;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_UNUSED_ARG (indexType_in);
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}
