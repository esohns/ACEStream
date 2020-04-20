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
#else
extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
//#include "libavformat/raw.h"
//#include "libavformat/riff.h"
}
#endif // ACE_WIN32 || ACE_WIN64
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/rational.h"
}

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#else
#include "stream_dev_tools.h"

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
    case STREAM_MESSAGE_CONTROL:
    {
      ControlMessageType* message_p =
        dynamic_cast<ControlMessageType*> (messageBlock_in);
      ACE_ASSERT (message_p);

      switch (message_p->type ())
      {
        case STREAM_CONTROL_MESSAGE_END:
        {
          SessionDataType* session_data_p =
              reinterpret_cast<SessionDataType*> (messageBlock_in->rd_ptr ());
          ACE_ASSERT (session_data_p);

          // *TODO*: remove type inference
          if (!session_data_p->targetFileName.empty ())
            if (unlikely (!postProcessHeader (session_data_p->targetFileName)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader(\"%s\"), aborting\n"),
                          inherited::mod_->name (),
                          ACE_TEXT (session_data_p->targetFileName.c_str ())));
              return -1;
            } // end IF
          break;
        }
        default:
          break;
      } // end SWITCH
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  messageBlock_in->msg_type ()));
      return -1;
    }
  } // end SWITCH

  return 0;
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
  ACE_Task_Base* task_base_p = inherited::sibling ();
  ACE_ASSERT (task_base_p);
  WRITER_TASK_T* writer_p = dynamic_cast<WRITER_TASK_T*> (task_base_p);
  if (unlikely (!writer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Decoder_AVIEncoder_WriterTask_T>: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: post-processing AVI file header (was: \"%s\")\n"),
              inherited::mod_->name (),
              ACE_TEXT (targetFilename_in.c_str ())));
#endif // _DEBUG
  // *NOTE*: things left to do (in order):
  //         - correct cb value of 'RIFF' _rifflist (6)
  //         - set dwTotalFrames in _avimainheader  (1)
  //         - set dwLength in _avistreamheader     (2)
  //         - correct cb value of 'movi' _rifflist (3)
  //         - correct cb value of 'idx1' _rifflist (4)
  //         [- insert version 2 (super-)index]     (5)
  // *NOTE*: how (4) implies correction of cb values for LISTs RIFF, hdrl, strl
  //         and movi
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  unsigned int value_i;
  std::ios::streamoff list_movi_offset = STREAM_DEC_AVI_JUNK_CHUNK_ALIGN;
  //std::ios::streamoff chunk_idx1_offset = 0;
  // *NOTE*: "...A joint file position is maintained for both the input sequence
  //         and the output sequence. ...", i.e. std::fstream::read()/write()
  //         modify both seekg/seekp
  //        --> maintain offsets separately
  std::ios::streamoff read_offset = 0, write_offset = 0;
#endif // ACE_WIN32 || ACE_WIN64
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
  char buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (char[BUFSIZ]));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _rifflist* RIFF_list_p = NULL;
  struct _riffchunk* RIFF_chunk_p = NULL;
  struct _avimainheader* AVI_header_avih_p = NULL;
  struct _avistreamheader* AVI_header_strh_p = NULL;
  stream.read (buffer_a, sizeof (struct _rifflist));
  if (stream.eof () || stream.fail ())
    goto error;
  read_offset = sizeof (struct _rifflist);
  RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
  ACE_ASSERT (RIFF_list_p->fcc == FCC ('RIFF'));
  ACE_ASSERT (RIFF_list_p->fccListType == FCC ('AVI '));
  // *TODO*: support index version 2.0
  value_i =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (RIFF_list_p->cb)
                                           : RIFF_list_p->cb);
  value_i +=
    (writer_p->frameOffsets_.size () * (sizeof (struct _riffchunk) + writer_p->frameSize_)) + // see: #1225
    (writer_p->frameOffsets_.size () * sizeof (struct _avioldindex::_avioldindex_entry));     // see: #1228
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
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (writer_p->frameOffsets_.size ())
                                           : writer_p->frameOffsets_.size ());
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
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (writer_p->frameOffsets_.size ())
                                           : writer_p->frameOffsets_.size ());
  stream.seekp (write_offset,
                ios::beg);
                //ios_base::beg);
  if (unlikely (stream.fail ()))
    goto error;
  stream.write (reinterpret_cast<char*> (AVI_header_strh_p),
                sizeof (struct _avistreamheader));
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
  // *TODO*: support index version 2.0
  value_i =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (RIFF_list_p->cb)
                                           : RIFF_list_p->cb);
  value_i +=
    (writer_p->frameOffsets_.size () * (sizeof (struct _riffchunk) + writer_p->frameSize_)); // see: #1440
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
#endif // ACE_WIN32 || ACE_WIN64

  result = true;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
error:
#endif // ACE_WIN32 || ACE_WIN64
  stream.close ();
  if (unlikely (stream.fail ()))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to close file (was: \"%s\"), continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (targetFilename_in.c_str ())));

  return result;
}

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                       UserDataType>::Stream_Decoder_AVIEncoder_WriterTask_T (ISTREAM_T* stream_in)
#else
                                       UserDataType>::Stream_Decoder_AVIEncoder_WriterTask_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , isActive_ (true)
 , isFirst_ (true)
 , format_ ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , formatContext_ (NULL)
#endif // ACE_WIN32 || ACE_WIN64
 , frameSize_ (0)
 , currentFrameOffset_ (0)
 , frameOffsets_ ()
 , writeAVI1Index_ (true)
 , writeAVI2Index_ (false)
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
  int result = -1;

  if (formatContext_)
  {
    if (formatContext_->streams)
      if (formatContext_->streams[0]->codec)
      {
        result = avcodec_close (formatContext_->streams[0]->codec);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      } // end IF
    avformat_free_context (formatContext_); formatContext_ = NULL;
  } // end IF
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
    isActive_ = true;
    isFirst_ = true;
    format_.format = AV_PIX_FMT_NONE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    format_.resolution.cx = 0;
    format_.resolution.cy = 0;
#else
    int result = -1;
    if (formatContext_)
    {
      if (formatContext_->streams)
        if (formatContext_->streams[0]->codec)
        {
          result = avcodec_close (formatContext_->streams[0]->codec);
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        } // end IF

      avformat_free_context (formatContext_); formatContext_ = NULL;
    } // end IF
    format_.resolution.width = 0;
    format_.resolution.height = 0;
#endif // ACE_WIN32 || ACE_WIN64
    frameSize_ = 0;
    currentFrameOffset_ = 0;
    frameOffsets_.clear ();
    writeAVI1Index_ = true;
    writeAVI2Index_ = false;
  } // end IF

  // *TODO*: remove type inference
  isActive_ = true;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  av_register_all ();
//  avcodec_register_all ();

  struct AVOutputFormat* output_format_p =
      av_guess_format (ACE_TEXT_ALWAYS_CHAR ("avi"), // short name
                       NULL,                         // file name
                       NULL);                        // MIME-type
  if (unlikely (!output_format_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: av_guess_format(\"%s\") failed, aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT ("avi")));
    return false;
  } // end IF
//  output_format_p->flags |= AVFMT_RAWPICTURE;
  ACE_ASSERT (!formatContext_);
  formatContext_ = avformat_alloc_context ();
  if (unlikely (!formatContext_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avformat_alloc_context() failed, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  formatContext_->oformat = output_format_p;
//  result =
//      avformat_alloc_output_context2 (&formatContext_, // return value: format context handle
//                                      output_format_p, // output format handle
//                                      NULL,            // format name
//                                      NULL);           // filename
//  if ((result < 0) || !formatContext_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("avformat_alloc_output_context2() failed: \"%s\", aborting\n"),
//                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
  ACE_ASSERT (formatContext_->oformat);
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
  if (unlikely (!isActive_))
    return; // nothing to do

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along. In this case,
  //         the individual frames are encapsulated and passed as such
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _riffchunk RIFF_chunk;
#else
  unsigned int riff_chunk_size = 0;
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inferences
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  message_block_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    inherited::allocateMessage ((isFirst_ ? STREAM_DEC_AVI_JUNK_CHUNK_ALIGN + sizeof (struct _rifflist) + sizeof (struct _riffchunk)
                                          : sizeof (struct _riffchunk)));
#else
    inherited::allocateMessage ((isFirst_ ? STREAM_DEC_AVI_JUNK_CHUNK_ALIGN + (4 + 4 + 4) + (4 + 4)
                                          : (4 + 4)));
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (!message_block_p))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), returning\n"),
                inherited::mod_->name (),
                (isFirst_ ? STREAM_DEC_AVI_JUNK_CHUNK_ALIGN + sizeof (struct _rifflist) + sizeof (struct _riffchunk)
                          : sizeof (struct _riffchunk))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), returning\n"),
                inherited::mod_->name (),
                (isFirst_ ? STREAM_DEC_AVI_JUNK_CHUNK_ALIGN + (4 + 4 + 4) + (4 + 4)
                          : (4 + 4))));
#endif // ACE_WIN32 || ACE_WIN64
    goto error;
  } // end IF

  if (unlikely (isFirst_))
  {
    isFirst_ = false;

    if (unlikely (!generateHeader (message_block_p)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (message_block_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // db (--> Uncompressed video frame)
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = FCC ('00db');
  RIFF_chunk.cb =
      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? frameSize_
                                             : ACE_SWAP_LONG (frameSize_));
  result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                  sizeof (struct _riffchunk));
#else
  result = message_block_p->copy (ACE_TEXT_ALWAYS_CHAR ("00db"),
                                  4);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  riff_chunk_size =
      ((ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN) ? frameSize_
                                             : ACE_SWAP_LONG (frameSize_));
  result = message_block_p->copy (reinterpret_cast<char*> (&riff_chunk_size),
                                  4);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  message_block_p->cont (message_inout);
  frameOffsets_.push_back (currentFrameOffset_);
  currentFrameOffset_ +=
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          (sizeof (struct _riffchunk) + frameSize_);
#else
          ((4 + 4) + frameSize_);
#endif // ACE_WIN32 || ACE_WIN64

  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  return;

error:
  if (message_block_p)
    message_block_p->release ();
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

      MediaType media_type_s; 
      inherited2:: getMediaType (session_data_r.formats.front (),
                                 media_type_s);
//      unsigned int bits_per_sample = 24;
      int result = -1;
      enum AVCodecID codec_id = AV_CODEC_ID_RAWVIDEO; // RGB
      struct AVCodec* codec_p = NULL;
      struct AVCodecContext* codec_context_p = NULL;
      struct AVStream* stream_p = NULL;
      int flags = (SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
      //                 SWS_LANCZOS | SWS_ACCURATE_RND);

      inherited2::getMediaType (media_type_s,
                                format_);
      frameSize_ =
        av_image_get_buffer_size (format_.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  format_.resolution.cx, format_.resolution.cy,
#else
                                  format_.resolution.width, format_.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                                  1); // *TODO*: linesize alignment

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // sanity check(s)
      ACE_ASSERT (formatContext_);
      ACE_ASSERT (formatContext_->oformat);

      formatContext_->oformat->audio_codec = AV_CODEC_ID_NONE;
      switch (format_.format)
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
        case AV_PIX_FMT_YUV420P: // 'YU12'
        case AV_PIX_FMT_YUYV422:
//        case V4L2_PIX_FMT_YUV420: // 'YU12'
//        case V4L2_PIX_FMT_YVU420: // 'YV12'
//        case V4L2_PIX_FMT_YUYV:
          codec_id = AV_CODEC_ID_CYUV; // AV_CODEC_ID_YUV4 ?
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
                      ACE_TEXT ("%s: invalid/unknown AVI pixel format (was: %s), returning\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (format_.format).c_str ())));
          goto error;
        }
      } // end SWITCH
      formatContext_->oformat->video_codec = codec_id;

      codec_p = avcodec_find_encoder (codec_id);
      if (unlikely (!codec_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) failed: \"%m\", returning\n"),
                    inherited::mod_->name (),
                    codec_id));
        goto error;
      } // end IF
      ACE_ASSERT (!codec_context_p);
      codec_context_p = avcodec_alloc_context3 (codec_p);
      if (unlikely (!codec_context_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
//      result = avcodec_get_context_defaults3 (codec_context_p,
//                                              codec_p);
//      if (result < 0)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: avcodec_get_context_defaults3() failed: \"%s\", returning\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//        goto error;
//      } // end IF

      codec_context_p->codec_id = codec_id;
//      codec_context_p->codec_tag = MKTAG ('B', 'G', 'R', '8');
//      codec_context_p->bit_rate = frameSize_; //* format_.frameRate.num * 8;
//      codec_context_p->bit_rate_tolerance = 5000000;
//      codec_context_p->global_quality = 0;
//      codec_context_p->compression_level = FF_COMPRESSION_DEFAULT;
//      codec_context_p->flags = 0;
//      codec_context_p->flags2 = 0;
      codec_context_p->time_base.num =
          (format_.frameRate.den ? format_.frameRate.den : 1);
      codec_context_p->time_base.den =
          (format_.frameRate.num ? format_.frameRate.num : 1);
//      codec_context_p->ticks_per_frame = 1;
      codec_context_p->width = format_.resolution.width;
      codec_context_p->height = format_.resolution.height;
//      codec_context_p->gop_size = 0;
      codec_context_p->pix_fmt = format_.format;
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
      codec_context_p->workaround_bugs = FF_BUG_AUTODETECT;
      codec_context_p->strict_std_compliance = FF_COMPLIANCE_VERY_STRICT;
//      codec_context_p->debug = 0;
//      codec_context_p->debug_mv = 0;
//      codec_context_p->dct_algo = FF_DCT_AUTO;
//      codec_context_p->idct_algo = FF_IDCT_AUTO;
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

      result = avcodec_open2 (codec_context_p,
                              codec_context_p->codec,
                              NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_open2(%d) failed: \"%s\", returning\n"),
                    inherited::mod_->name (),
                    codec_id,
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      ACE_ASSERT (!formatContext_->streams);
      stream_p = avformat_new_stream (formatContext_,
                                      codec_p);
      if (unlikely (!stream_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (stream_p->codec);
      formatContext_->streams[0] = stream_p;

//      stream_p->id = 0;
      // *TODO*: why does this need to be reset ?
      stream_p->codec->bit_rate = frameSize_ * format_.frameRate.num * 8;
      stream_p->codec->codec_id = codec_id;
//      stream_p->codec->codec_tag = MKTAG ('B', 'G', 'R', 'A');
    //  stream_p->codec->codec_type = codec_->type;

      stream_p->codec->pix_fmt = format_.format;
      stream_p->codec->width = format_.resolution.width;
      stream_p->codec->height = format_.resolution.height;
      stream_p->time_base.num = format_.frameRate.den;
      stream_p->time_base.den = format_.frameRate.num;
//      stream_p->sample_aspect_ratio = 0;
      stream_p->avg_frame_rate.num = format_.frameRate.num;
      stream_p->avg_frame_rate.den = format_.frameRate.den;
//      stream_p->side_data = NULL;
//      stream_p->nb_side_data = 0;
//      stream_p->event_flags = AVSTREAM_EVENT_FLAG_METADATA_UPDATED;
#endif // ACE_WIN32 || ACE_WIN64

      goto continue_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
error:
      if (codec_context_p)
      {
        avcodec_free_context (&codec_context_p); codec_context_p = NULL;
      } // end IF
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      ACE_ASSERT (false); // *TODO*
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      int result = -1;
      ACE_Message_Block* message_block_p = NULL;

      // sanity check(s)
      if (unlikely (!isActive_))
        goto continue_2; // nothing to do
      if (!writeAVI1Index_)
        goto continue_2;

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

      // *TODO*: remove type inference
      message_block_p =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: Stream_TaskBase_T::allocateMessage(%d) failed: \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->allocatorConfiguration->defaultBufferSize));
        goto continue_2;
      } // end IF
      ACE_ASSERT (message_block_p);
      if (unlikely (!generateIndex (STREAM_AVI_INDEX_V1,
                                    message_block_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex(%d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    STREAM_AVI_INDEX_V1));
        message_block_p->release (); message_block_p = NULL;
        goto continue_2;
      } // end IF

      result = inherited::put_next (message_block_p, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
        message_block_p->release (); message_block_p = NULL;
        goto continue_2;
      } // end IF

continue_2:
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

  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  const SessionDataType& session_data_r = inherited::sessionData_->getR ();

  // sanity check(s)
  ACE_ASSERT (!session_data_r.formats.empty ());

  // *NOTE*: need to reclaim this memory (see below)
  struct _AMMediaType media_type_s;
    inherited2::getMediaType (session_data_r.formats.back (),
                              media_type_s);
  struct _riffchunk RIFF_chunk;
  struct _rifflist RIFF_list;
  struct _avimainheader AVI_header_avih;
  struct _avistreamheader AVI_header_strh;
  FOURCCMap fourcc_map;
  struct tagBITMAPINFOHEADER AVI_header_strf;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  unsigned int value_i = 0;
  short int value_2 = 0;

  if (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo))
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_s.pbFormat);
  else if (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo2))
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_s.pbFormat);
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown media format type (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::GUIDToString (media_type_s.formattype).c_str ())));
    goto error;
  } // end ELSE

//  // *NOTE*: the input format is converted to BGR24
//  //         --> adjust header accordingly
//  if (InlineIsEqualGUID (media_type_s.subtype, MEDIASUBTYPE_RGB24))
//    goto continue_;
//
//  ACE_ASSERT (InlineIsEqualGUID (media_type_s.majortype, MEDIATYPE_Video));
//  media_type_s.subtype = MEDIASUBTYPE_RGB24;
//  ACE_ASSERT (media_type_s.bFixedSizeSamples == TRUE);
//  ACE_ASSERT (media_type_s.bTemporalCompression == FALSE);
//  media_type_s.lSampleSize = frameSize_;
//  if (video_info_header_p)
//  {
//    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
//    video_info_header_p->bmiHeader.biBitCount = 24;
//    ACE_ASSERT (video_info_header_p->bmiHeader.biCompression == BI_RGB);
//    video_info_header_p->bmiHeader.biSizeImage = frameSize_;
//      //DIBSIZE (video_info_header_p->bmiHeader);
//    video_info_header_p->dwBitRate =
//      (frameSize_ * 8) *                                                      // bits / frame
//      (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
//  } // end IF
//  else
//  {
//    ACE_ASSERT (video_info_header2_p->bmiHeader.biPlanes == 1);
//    video_info_header2_p->bmiHeader.biBitCount = 24;
//    ACE_ASSERT (video_info_header2_p->bmiHeader.biCompression == BI_RGB);
//    video_info_header2_p->bmiHeader.biSizeImage = frameSize_;
//      //DIBSIZE (video_info_header_p->bmiHeader);
//    video_info_header2_p->dwBitRate =
//      (frameSize_ * 8) *                                                       // bits / frame
//      (10000000 / static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame)); // fps
//  } // end ELSE
//
//continue_:
  // RIFF
  ACE_OS::memset (&RIFF_list, 0, sizeof (struct _rifflist));
  RIFF_list.fcc = FCC ('RIFF');
  // sizeof (fccListType) [4] + sizeof (RIFF) [== total (file) size - 8]
  value_i = 4                                   + // 'AVI '
            // sizeof (LIST hdrl)
            sizeof (struct _rifflist)           + // 'hdrl'
            sizeof (struct _avimainheader)      + // 'avih'
            // sizeof (LIST strl)
            sizeof (struct _rifflist)           + // 'strl'
            sizeof (struct _avistreamheader)    + // 'strh' / 'vids'
            sizeof (struct _riffchunk)          + // 'strf'
            sizeof (struct tagBITMAPINFOHEADER) +
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
            //sizeof (struct _riffchunk)          + // 'indx' // *TODO*: index version 2.0; post-processing
            sizeof (struct _riffchunk)          + // 'JUNK'
            1820                                + // pad bytes
            // sizeof (LIST movi)
            sizeof (struct _rifflist)           + // 'movi'
            //sizeof (struct _riffchunk)        + // 'ix00' // *TODO*: index version 2.0; post-processing
            //(sizeof (struct _riffchunk) + frameSize_) * #frames + // '00db'; post-processing
            // sizeof (idx1)
            sizeof (struct _riffchunk);           // 'idx1'
            //sizeof (struct _avioldindex::_avioldindex_entry) * #frames; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('AVI ');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     sizeof (struct _rifflist));
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
            sizeof (struct tagBITMAPINFOHEADER);
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
            //sizeof (struct _riffchunk)          + // 'indx' // *TODO*: index version 2.0; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('hdrl');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     sizeof (struct _rifflist));
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
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? video_info_header_p->dwBitRate
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
  value_i = 1;
  AVI_header_avih.dwStreams =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = 8 + frameSize_; // *NOTE*: unreliable
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
                              sizeof (struct _avimainheader));
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
  // strl
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST strl)
  value_i = 4                                 +  // 'strl'
            sizeof (struct _avistreamheader)  +  // 'strh' / 'vids'
            sizeof (struct _riffchunk)        +  // 'strf'
            sizeof (struct tagBITMAPINFOHEADER);
            //sizeof (struct _riffchunk)          + // 'strd' // *TODO*
            //sizeof (struct _riffchunk)          + // 'strn' // *TODO*
            //sizeof (struct _riffchunk);           // 'indx' // *TODO*: index version 2.0; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = ckidSTREAMLIST;
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     sizeof (struct _rifflist));
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
  // *NOTE*: RGB24 --> 0
  //fourcc_map.SetFOURCC (&media_type_p->subtype);
  //AVI_header_strh.fccHandler = fourcc_map.GetFOURCC ();
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
  value_i = 8 + frameSize_; // *NOTE*: unreliable
  AVI_header_strh.dwSuggestedBufferSize = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = -1; // default
  AVI_header_strh.dwQuality = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i = frameSize_;
  AVI_header_strh.dwSampleSize = 
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.left)
                                                                   : static_cast<short int> (video_info_header2_p->rcTarget.left));
  AVI_header_strh.rcFrame.left =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.right)
                                                                   : static_cast<short int> (video_info_header2_p->rcTarget.right));
  AVI_header_strh.rcFrame.right =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.top)
                                                                   : static_cast<short int> (video_info_header2_p->rcTarget.top));
  AVI_header_strh.rcFrame.top =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.bottom)
                                                                   : static_cast<short int> (video_info_header2_p->rcTarget.bottom));
  AVI_header_strh.rcFrame.bottom =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strh),
                              sizeof (struct _avistreamheader));
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
                                     sizeof (struct _riffchunk));
  ACE_OS::memset (&AVI_header_strf, 0, sizeof (struct tagBITMAPINFOHEADER));
  AVI_header_strf =
    (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo) ? video_info_header_p->bmiHeader
                                                                   : video_info_header2_p->bmiHeader);
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strf),
                              sizeof (struct tagBITMAPINFOHEADER));
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

  // --> END strl

  // insert JUNK chunk to align the 'movi' list to 2048 bytes
  // --> should speed up CD-ROM access
  value_i = (AVI_header_avih.dwPaddingGranularity -
             (messageBlock_inout->length () + 8));
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: inserting JUNK chunk (%u pad byte(s))\n"),
              inherited::mod_->name (),
              value_i));
#endif // _DEBUG
  RIFF_chunk.fcc = FCC ('JUNK');
  RIFF_chunk.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                     sizeof (struct _riffchunk));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  ACE_OS::memset (messageBlock_inout->wr_ptr (), 0, value_i);
  messageBlock_inout->wr_ptr (RIFF_chunk.cb);

  // movi
  // *NOTE*: this is the origin of the frame index; increment the corresponding
  //         offsets from here
  ACE_ASSERT (!currentFrameOffset_ && frameOffsets_.empty ());
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST movi)
  value_i = 4;               //                                   +
            //sizeof (struct _riffchunk) + ???                    + // 'ix00' // *TODO*: index version 2.0; post-processing
            //(sizeof (struct _riffchunk) + frameSize_) * #frames + // '00db'; post-processing
  RIFF_list.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  RIFF_list.fccListType = FCC ('movi');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     sizeof (struct _rifflist));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  currentFrameOffset_ += sizeof (struct _rifflist);

  // clean up
  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  goto continue_2;

error:
  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  return false;

continue_2:
#else
  ACE_ASSERT (!formatContext_->pb);
  formatContext_->pb =
    avio_alloc_context (reinterpret_cast<unsigned char*> (messageBlock_inout->wr_ptr ()), // buffer handle
                        messageBlock_inout->capacity (),                                  // buffer size
                        1,                                                                // write flag
                        messageBlock_inout,                                               // act
                        NULL,                                                             // read callback
                        stream_decoder_aviencoder_libav_write_cb,                         // write callback
                        NULL);                                                            // seek callback
  if (unlikely (!formatContext_->pb))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avio_alloc_context() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

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
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  avio_flush (formatContext_->pb);
#endif // ACE_WIN32 || ACE_WIN64

  return true;
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
      unsigned int value_i = 0;
      ACE_OS::memset (&AVI_header_index, 0, sizeof (struct _avioldindex));
      AVI_header_index.fcc = ckidAVIOLDINDEX;
      value_i =
        (sizeof (struct _avioldindex::_avioldindex_entry) *
         frameOffsets_.size ());
      AVI_header_index.cb =
        ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                               : value_i);
      result =
        messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_index),
                                  sizeof (struct _avioldindex));
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    sizeof (struct _avioldindex)));
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
        value_i = *iterator;
        _avioldindex_entry_s.dwOffset =
          ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                                 : value_i);
        value_i = frameSize_;
        _avioldindex_entry_s.dwSize =
          ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                                 : value_i);
        result =
          messageBlock_inout->copy (reinterpret_cast<char*> (&_avioldindex_entry_s),
                                    sizeof (struct _avioldindex::_avioldindex_entry));
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      sizeof (struct _avioldindex::_avioldindex_entry)));
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
  ACE_ASSERT (formatContext_);

  result = av_write_trailer (formatContext_);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: av_write_trailer() failed: \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}
