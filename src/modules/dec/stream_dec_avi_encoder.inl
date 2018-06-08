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
#include <mmiscapi.h>
#include <aviriff.h>
#include <dvdmedia.h>
#include <fourcc.h>
#include <mfobjects.h>
// *WARNING*: <uuids.h> doesn't have double include protection
//#include <uuids.h>
#else
#include "linux/videodev2.h"

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
#include "libswscale/swscale.h"
}

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#else
#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
          typename UserDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       FormatType,
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
          typename FormatType,
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
                                       FormatType,
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
            if (!postProcessHeader (session_data_p->targetFileName))
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
          typename FormatType,
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
                                       FormatType,
                                       UserDataType>::postProcessHeader (const std::string& targetFilename_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader"));

  // sanity check(s)
  ACE_ASSERT (!targetFilename_in.empty ());

  bool result = false;
  ACE_Task_Base* task_base_p = inherited::sibling ();
  ACE_ASSERT (task_base_p);
  WRITER_TASK_T* writer_p = dynamic_cast<WRITER_TASK_T*> (task_base_p);
  if (!writer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Decoder_AVIEncoder_WriterTask_T>: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: post-processing AVI file header (was: \"%s\")...\n"),
              inherited::mod_->name (),
              ACE_TEXT (targetFilename_in.c_str ())));

  // *NOTE*: things left to do (in order):
  //         - correct cb value of 'RIFF' _rifflist (6)
  //         - set dwTotalFrames in _avimainheader  (1)
  //         - set dwLength in _avistreamheader     (2)
  //         - correct cb value of 'movi' _rifflist (3)
  //         - correct cb value of 'idx1' _rifflist (4)
  //         [- insert version 2 (super-)index]     (5)
  // *NOTE*: how (4) implies correction of cb values for LISTs RIFF, hdrl, strl
  //         and movi
  unsigned int value_i;
  char buffer_a[BUFSIZ];
  std::ios::streamoff list_movi_offset = STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN;
  //std::ios::streamoff chunk_idx1_offset = 0;
  // *NOTE*: "...A joint file position is maintained for both the input sequence
  //         and the output sequence. ...", i.e. std::fstream::read()/write()
  //         modify both seekg/seekp
  //        --> maintain offsets separately
  std::ios::streamoff read_offset = 0, write_offset = 0;
  std::fstream stream (targetFilename_in.c_str (),
                       //ios_base::in | ios_base::out//,
                       std::ios::in | std::ios::out | std::ios::binary
                       /*(int)ios_base::_Openprot*/);
  if (stream.fail ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to open file (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (targetFilename_in.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (stream.is_open ());
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
  if (stream.fail ())
    goto error;
  stream.write (reinterpret_cast<char*> (RIFF_list_p),
                sizeof (struct _rifflist));
  if (stream.fail ())
    goto error;
  stream.seekg (read_offset,
                ios::beg);
                //ios_base::beg);
  if (stream.fail ())
    goto error;
  stream.read (buffer_a, sizeof (struct _rifflist));
  if (stream.eof () || stream.fail ())
    goto error;
  read_offset += sizeof (struct _rifflist);
  RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
  ACE_ASSERT (RIFF_list_p->fccListType == FCC ('hdrl'));
  stream.read (buffer_a, sizeof (struct _avimainheader));
  if (stream.eof () || stream.fail ())
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
  if (stream.fail ())
    goto error;
  stream.write (reinterpret_cast<char*> (AVI_header_avih_p),
                sizeof (struct _avimainheader));
  if (stream.fail ())
    goto error;
  stream.seekg (read_offset,
                ios::beg);
                //ios_base::beg);
  if (stream.fail ())
    goto error;
  stream.read (buffer_a, sizeof (struct _rifflist));
  if (stream.eof () || stream.fail ())
    goto error;
  read_offset += sizeof (struct _rifflist);
  RIFF_list_p = reinterpret_cast<struct _rifflist*> (buffer_a);
  ACE_ASSERT (RIFF_list_p->fccListType == FCC ('strl'));
  stream.read (buffer_a, sizeof (struct _avistreamheader));
  if (stream.eof () || stream.fail ())
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
  if (stream.fail ())
    goto error;
  stream.write (reinterpret_cast<char*> (AVI_header_strh_p),
                sizeof (struct _avistreamheader));
  if (stream.fail ())
    goto error;
  stream.seekg (list_movi_offset,
                ios::beg);
                //ios_base::beg);
  if (stream.fail ())
    goto error;
  stream.read (buffer_a, sizeof (struct _rifflist));
  if (stream.eof () || stream.fail ())
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
  if (stream.fail ())
    goto error;
  stream.write (reinterpret_cast<char*> (RIFF_list_p),
                sizeof (struct _rifflist));
  if (stream.fail ())
    goto error;


#else
  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif // ACE_WIN32 || ACE_WIN64

  result = true;

error:
  stream.close ();
  if (stream.fail ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to close file (was: \"%s\"), continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (targetFilename_in.c_str ())));

  return result;
}

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
          typename UserDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       FormatType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                       UserDataType>::Stream_Decoder_AVIEncoder_WriterTask_T (ISTREAM_T* stream_in)
#else
                                       UserDataType>::Stream_Decoder_AVIEncoder_WriterTask_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , isActive_ (true)
 , isFirst_ (true)
 , format_ (AV_PIX_FMT_NONE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , formatContext_ (NULL)
#endif // ACE_WIN32 || ACE_WIN64
 , frameSize_ (0)
 , height_ (0)
 , width_ (0)
 , transformContext_ (NULL)
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
          typename FormatType,
          typename UserDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       FormatType,
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
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      } // end IF

    avformat_free_context (formatContext_);
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  if (transformContext_)
    sws_freeContext (transformContext_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
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
                                       FormatType,
                                       UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::initialize"));

  if (inherited::isInitialized_)
  {
    isActive_ = true;
    isFirst_ = true;
    format_ = AV_PIX_FMT_NONE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    int result = -1;
    if (formatContext_)
    {
      if (formatContext_->streams)
        if (formatContext_->streams[0]->codec)
        {
          result = avcodec_close (formatContext_->streams[0]->codec);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        } // end IF


      avformat_free_context (formatContext_);
      formatContext_ = NULL;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64
    frameSize_ = 0;
    height_ = 0;
    width_ = 0;
    if (transformContext_)
    {
      sws_freeContext (transformContext_);
      transformContext_ = NULL;
    } // end IF
    currentFrameOffset_ = 0;
    frameOffsets_.clear ();
    writeAVI1Index_ = true;
    writeAVI2Index_ = false;
  } // end IF

  // *TODO*: remove type inference
  isActive_ = !configuration_in.targetFileName.empty ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  av_register_all ();
//  avcodec_register_all ();

  struct AVOutputFormat* output_format_p =
      av_guess_format (ACE_TEXT_ALWAYS_CHAR ("avi"), // short name
                       NULL,                         // file name
                       NULL);                        // MIME-type
  if (!output_format_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: av_guess_format(\"%s\") failed, aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT ("avi")));
    return false;
  } // end IF
  output_format_p->flags |= AVFMT_RAWPICTURE;
  ACE_ASSERT (!formatContext_);
  formatContext_ = avformat_alloc_context ();
  if (!formatContext_)
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
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
//                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//      } // end IF

//    avformat_free_context (formatContext_);
//    formatContext_ = NULL;
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
          typename FormatType,
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
                                       FormatType,
                                       UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleDataMessage"));

  // sanity check(s)
  if (!isActive_)
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
    inherited::allocateMessage ((transformContext_ ? (isFirst_ ? STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN + sizeof (struct _rifflist) + sizeof (struct _riffchunk) + frameSize_
                                                               : sizeof (struct _riffchunk) + frameSize_)
                                                   : (isFirst_ ? STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN + sizeof (struct _rifflist) + sizeof (struct _riffchunk)
                                                               : sizeof (struct _riffchunk))));
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), returning\n"),
                inherited::mod_->name (),
                (transformContext_ ? (isFirst_ ? STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN + sizeof (struct _rifflist) + sizeof (struct _riffchunk) + frameSize_
                                               : sizeof (struct _riffchunk) + frameSize_)
                                   : (isFirst_ ? STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN + sizeof (struct _rifflist) + sizeof (struct _riffchunk)
                                               : sizeof (struct _riffchunk)))));
    goto error;
  } // end IF

  if (isFirst_)
  {
    isFirst_ = false;

    if (!generateHeader (message_block_p))
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
  RIFF_chunk.cb = frameSize_;
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                  sizeof (struct _riffchunk));
#else
  result = message_block_p->copy (ACE_TEXT_ALWAYS_CHAR ("00db"),
                                  4);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  riff_chunk_size = frameSize_;
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    riff_chunk_size = ACE_SWAP_LONG (riff_chunk_size);
  result = message_block_p->copy (reinterpret_cast<char*> (&riff_chunk_size),
                                  4);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  if (transformContext_)
  { ACE_ASSERT (message_block_p->space () >= frameSize_);
    uint8_t* in_data[AV_NUM_DATA_POINTERS];
    uint8_t* out_data[AV_NUM_DATA_POINTERS];
    ACE_OS::memset (in_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
    ACE_OS::memset (out_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
    in_data[0] = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
    out_data[0] = reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ());
    if (!Stream_Module_Decoder_Tools::convert (transformContext_,
                                               width_, height_, format_,
                                               in_data,
                                               width_, height_, AV_PIX_FMT_RGB24,
                                               out_data))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    message_block_p->wr_ptr (frameSize_);
    message_inout->release ();
  } // end IF
  else
    message_block_p->cont (message_inout);
  frameOffsets_.push_back (currentFrameOffset_);
  currentFrameOffset_ += (sizeof (struct _riffchunk) + frameSize_);

  result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
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
          typename FormatType,
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
                                       FormatType,
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
      struct AVRational frame_rate_s;
      unsigned int bits_per_sample = 24;
      int result = -1;
      enum AVCodecID codec_id = AV_CODEC_ID_RAWVIDEO; // RGB
      struct AVCodec* codec_p = NULL;
      struct AVCodecContext* codec_context_p = NULL;
      struct AVStream* stream_p = NULL;
      int flags = (SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
      //                 SWS_LANCZOS | SWS_ACCURATE_RND);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType* format_p = &getFormat (session_data_r.inputFormat);
      format_ =
        Stream_Module_Decoder_Tools::mediaSubTypeToAVPixelFormat (format_p->subtype,
                                                                  STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
      frame_rate_s = getFrameRate (session_data_r,
                                   format_p);
      getResolution (session_data_r,
                     format_p,
                     width_, height_);
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (format_p);
#else
      format_ = getFormat (&session_data_r.format);
      frame_rate_s = getFrameRate (session_data_r,
                                   &format_);
      getResolution (session_data_r,
                     &format_,
                     width_, height_);
#endif // ACE_WIN32 || ACE_WIN64
      frameSize_ = av_image_get_buffer_size (AV_PIX_FMT_RGB24,
                                             width_, height_,
                                             1); // *TODO*: linesize alignment

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // sanity check(s)
      ACE_ASSERT (formatContext_);
      ACE_ASSERT (formatContext_->oformat);

      formatContext_->oformat->audio_codec = AV_CODEC_ID_NONE;
      switch (format_)
      {
        // RGB formats
        case AV_PIX_FMT_BGR24:
        case AV_PIX_FMT_RGB24:
        case AV_PIX_FMT_BGRA:
        case AV_PIX_FMT_RGBA:
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
                      ACE_TEXT ("%s: invalid/unknown pixel format (was: %s), returning\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (format_).c_str ())));
          goto error;
        }
      } // end SWITCH
      formatContext_->oformat->video_codec = codec_id;

      codec_p = avcodec_find_encoder (codec_id);
      if (!codec_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) failed: \"%m\", returning\n"),
                    inherited::mod_->name (),
                    codec_id));
        goto error;
      } // end IF
      ACE_ASSERT (!codec_context_p);
      codec_context_p = avcodec_alloc_context3 (codec_p);
      if (!codec_context_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result = avcodec_get_context_defaults3 (codec_context_p,
                                              codec_p);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_get_context_defaults3() failed: \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      codec_context_p->codec_id = codec_id;
//      codec_context_p->codec_tag = MKTAG ('B', 'G', 'R', '8');
      codec_context_p->bit_rate = frameSize_ * frame_rate_s.num * 8;
//      codec_context_p->bit_rate_tolerance = 0;
//      codec_context_p->global_quality = 0;
//      codec_context_p->compression_level = FF_COMPRESSION_DEFAULT;
//      codec_context_p->flags = 0;
//      codec_context_p->flags2 = 0;
      codec_context_p->time_base.num = frame_rate_s.den;
      codec_context_p->time_base.den = frame_rate_s.num;
//      codec_context_p->ticks_per_frame = 1;
      codec_context_p->width = width_;
      codec_context_p->height = height_;
//      codec_context_p->gop_size = 0;
      codec_context_p->pix_fmt = AV_PIX_FMT_RGB24;
      codec_context_p->me_method = 1;
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
      codec_context_p->coder_type = 2;
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
      codec_context_p->bits_per_raw_sample = bits_per_sample;
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
                              codec_p,
                              NULL);
      if (result < 0)
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
      if (!stream_p)
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
      stream_p->codec->bit_rate = frameSize_ * frame_rate_s.num * 8;
      stream_p->codec->codec_id = codec_id;
//      stream_p->codec->codec_tag = MKTAG ('B', 'G', 'R', 'A');
    //  stream_p->codec->codec_type = codec_->type;

      stream_p->codec->pix_fmt = AV_PIX_FMT_RGB24;
      stream_p->codec->width = width_;
      stream_p->codec->height = height_;
      stream_p->time_base.num = frame_rate_s.den;
      stream_p->time_base.den = frame_rate_s.num;
//      stream_p->sample_aspect_ratio = 0;
      stream_p->avg_frame_rate.num = frame_rate_s.num;
      stream_p->avg_frame_rate.den = frame_rate_s.den;
//      stream_p->side_data = NULL;
//      stream_p->nb_side_data = 0;
//      stream_p->event_flags = AVSTREAM_EVENT_FLAG_METADATA_UPDATED;
#endif // ACE_WIN32 || ACE_WIN64

      if (format_ != AV_PIX_FMT_RGB24)
      {
        transformContext_ =
            sws_getCachedContext (NULL,
                                  width_, height_, format_,
                                  width_, height_, AV_PIX_FMT_RGB24,
                                  flags,                            // flags
                                  NULL, NULL,
                                  0);                               // parameters
        if (!transformContext_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: converting %s pixel format to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (format_).c_str ()),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (AV_PIX_FMT_RGB24).c_str ())));
      } // end IF

      goto continue_;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      if (codec_context_p)
        avcodec_free_context (&codec_context_p);
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
      if (!isActive_)
        goto continue_2; // nothing to do
      if (!writeAVI1Index_)
        goto continue_2;

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

      // *TODO*: remove type inference
      message_block_p =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
      if (!message_block_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: Stream_TaskBase_T::allocateMessage(%d) failed: \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->allocatorConfiguration->defaultBufferSize));
        goto continue_2;
      } // end IF
      ACE_ASSERT (message_block_p);
      if (!generateIndex (STREAM_AVI_INDEX_V1,
                          message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex(%d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    STREAM_AVI_INDEX_V1));
        message_block_p->release ();
        goto continue_2;
      } // end IF

      result = inherited::put_next (message_block_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
        message_block_p->release ();
        goto continue_2;
      } // end IF

continue_2:
      if (transformContext_)
      {
        sws_freeContext (transformContext_); transformContext_ = NULL;
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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
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
                                       FormatType,
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
  ACE_ASSERT (session_data_r.inputFormat);

  // *NOTE*: need to reclaim this memory (see below)
  struct _AMMediaType* media_type_p = &getFormat (session_data_r.inputFormat);
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

  if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_p->pbFormat);
  else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_p->pbFormat);
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown media format type (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::GUIDToString (media_type_p->formattype).c_str ())));
    goto error;
  } // end ELSE

  // *NOTE*: the input format is converted to BGR24
  //         --> adjust header accordingly
  if (InlineIsEqualGUID (media_type_p->subtype, MEDIASUBTYPE_RGB24))
    goto continue_;

  ACE_ASSERT (InlineIsEqualGUID (media_type_p->majortype, MEDIATYPE_Video));
  media_type_p->subtype = MEDIASUBTYPE_RGB24;
  ACE_ASSERT (media_type_p->bFixedSizeSamples == TRUE);
  ACE_ASSERT (media_type_p->bTemporalCompression == FALSE);
  media_type_p->lSampleSize = frameSize_;
  if (video_info_header_p)
  {
    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
    video_info_header_p->bmiHeader.biBitCount = 24;
    ACE_ASSERT (video_info_header_p->bmiHeader.biCompression == BI_RGB);
    video_info_header_p->bmiHeader.biSizeImage = frameSize_;
      //DIBSIZE (video_info_header_p->bmiHeader);
    video_info_header_p->dwBitRate =
      (frameSize_ * 8) *                                                      // bits / frame
      (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps
  } // end IF
  else
  {
    ACE_ASSERT (video_info_header2_p->bmiHeader.biPlanes == 1);
    video_info_header2_p->bmiHeader.biBitCount = 24;
    ACE_ASSERT (video_info_header2_p->bmiHeader.biCompression == BI_RGB);
    video_info_header2_p->bmiHeader.biSizeImage = frameSize_;
      //DIBSIZE (video_info_header_p->bmiHeader);
    video_info_header2_p->dwBitRate =
      (frameSize_ * 8) *                                                       // bits / frame
      (10000000 / static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame)); // fps
  } // end ELSE

continue_:
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
  if (result == -1)
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
  if (result == -1)
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
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame / 10)
                                                                    : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame / 10));
  AVI_header_avih.dwMicroSecPerFrame =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i =
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? video_info_header_p->dwBitRate
                                                                    : video_info_header2_p->dwBitRate) / 8;
  AVI_header_avih.dwMaxBytesPerSec =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  AVI_header_avih.dwPaddingGranularity =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN)
                                           : STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN);
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
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biWidth
                                                                    : video_info_header2_p->bmiHeader.biWidth);
  AVI_header_avih.dwWidth =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  value_i =
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biHeight
                                                                    : video_info_header2_p->bmiHeader.biHeight);
  AVI_header_avih.dwHeight =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  //AVI_header_avih.dwReserved = {0, 0, 0, 0};
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_avih),
                              sizeof (struct _avimainheader));
  if (result == -1)
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
  if (result == -1)
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
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
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
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.left)
                                                                    : static_cast<short int> (video_info_header2_p->rcTarget.left));
  AVI_header_strh.rcFrame.left =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.right)
                                                                    : static_cast<short int> (video_info_header2_p->rcTarget.right));
  AVI_header_strh.rcFrame.right =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.top)
                                                                    : static_cast<short int> (video_info_header2_p->rcTarget.top));
  AVI_header_strh.rcFrame.top =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  value_2 =
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? static_cast<short int> (video_info_header_p->rcTarget.bottom)
                                                                    : static_cast<short int> (video_info_header2_p->rcTarget.bottom));
  AVI_header_strh.rcFrame.bottom =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_WORD (value_2)
                                           : value_2);
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strh),
                              sizeof (struct _avistreamheader));
  if (result == -1)
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
    (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) ? video_info_header_p->bmiHeader
                                                                    : video_info_header2_p->bmiHeader);
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strf),
                              sizeof (struct tagBITMAPINFOHEADER));
  if (result == -1)
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
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: inserting JUNK chunk (%u pad byte(s))\n"),
              inherited::mod_->name (),
              value_i));
  RIFF_chunk.fcc = FCC ('JUNK');
  RIFF_chunk.cb =
    ((ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN) ? ACE_SWAP_LONG (value_i)
                                           : value_i);
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                     sizeof (struct _riffchunk));
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
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  currentFrameOffset_ += sizeof (struct _rifflist);

  // clean up
  Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);

  goto continue_2;

error:
  Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);

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
  if (!formatContext_->pb)
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
  if (result < 0)
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
          typename UserDataType>
AM_MEDIA_TYPE&
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       FormatType,
                                       UserDataType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;
  if (!Stream_MediaFramework_DirectShow_Tools::copyMediaType (*format_in,
                                                              result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::copyMediaType(), aborting\n"),
                inherited::mod_->name ()));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
          typename UserDataType>
AM_MEDIA_TYPE&
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       FormatType,
                                       UserDataType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;

  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (format_in),
                                        GUID_NULL,
                                        &result_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
          typename UserDataType>
AVRational&
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       FormatType,
                                       UserDataType>::getFrameRate_impl (const SessionDataType& sessionData_in,
                                                                         const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getFrameRate_impl"));

  // sanity check(s)
  ACE_UNUSED_ARG (sessionData_in);
  ACE_ASSERT (format_in);

  struct AVRational result;
  ACE_OS::memset (&result, 0, sizeof (struct AVRational));
  result.num = 1;

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  if (format_in->formattype == FORMAT_VideoInfo)
  {
    video_info_header_p =
      (struct tagVIDEOINFOHEADER*)format_in->pbFormat;
    result.den =
      10000000 / static_cast<int> (video_info_header_p->AvgTimePerFrame);
  } // end IF
  else if (format_in->formattype == FORMAT_VideoInfo2)
  {
    video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)format_in ->pbFormat;
    result.den =
      10000000 / static_cast<int> (video_info_header2_p->AvgTimePerFrame);
  } // end ELSE
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown media type format (was: %d), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::GUIDToString (format_in->formattype).c_str ())));

  return result;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FormatType,
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
                                       FormatType,
                                       UserDataType>::getResolution_impl (const SessionDataType& sessionData_in,
                                                                          const struct _AMMediaType* format_in,
                                                                          unsigned int& width_out,
                                                                          unsigned int& height_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getResolution_impl"));

  // sanity check(s)
  ACE_UNUSED_ARG (sessionData_in);
  ACE_ASSERT (format_in);

  // initialize return value(s)
  width_out = 0;
  height_out = 0;

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  if (format_in->formattype == FORMAT_VideoInfo)
  {
    video_info_header_p =
      (struct tagVIDEOINFOHEADER*)format_in->pbFormat;
    width_out = video_info_header_p->bmiHeader.biWidth;
    height_out = abs (video_info_header_p->bmiHeader.biHeight);
  } // end IF
  else if (format_in->formattype == FORMAT_VideoInfo2)
  {
    video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)format_in ->pbFormat;
    width_out = video_info_header2_p->bmiHeader.biWidth;
    height_out = abs (video_info_header2_p->bmiHeader.biHeight);
  } // end ELSE
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown media type format (was: %d), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::GUIDToString (format_in->formattype).c_str ())));
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
          typename FormatType,
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
                                       FormatType,
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
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: av_write_trailer() failed: \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}

////////////////////////////////////////////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename UserDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       struct v4l2_format,
                                       UserDataType>::Stream_Decoder_AVIEncoder_WriterTask_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , isFirst_ (true)
 , formatContext_ (NULL)
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
          typename UserDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType,
                                       struct v4l2_format,
                                       UserDataType>::~Stream_Decoder_AVIEncoder_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::~Stream_Decoder_AVIEncoder_WriterTask_T"));

  int result = -1;

  if (formatContext_)
  {
    if (formatContext_->streams)
      if (formatContext_->streams[0]->codec)
      {
        result = avcodec_close (formatContext_->streams[0]->codec);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      } // end IF

    avformat_free_context (formatContext_);
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
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
                                       struct v4l2_format,
                                       UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::initialize"));

  ACE_UNUSED_ARG (allocator_in);

  if (inherited::isInitialized_)
  {
    isFirst_ = true;
    int result = -1;
    if (formatContext_)
    {
      if (formatContext_->streams)
        if (formatContext_->streams[0]->codec)
        {
          result = avcodec_close (formatContext_->streams[0]->codec);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        } // end IF


      avformat_free_context (formatContext_);
      formatContext_ = NULL;
    } // end IF
  } // end IF

  av_register_all ();
//  avcodec_register_all ();

  struct AVOutputFormat* output_format_p =
      av_guess_format (ACE_TEXT_ALWAYS_CHAR ("avi"), // short name
                       NULL,                         // file name
                       NULL);                        // MIME-type
  if (!output_format_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: av_guess_format(\"%s\") failed, aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT ("avi")));
    return false;
  } // end IF
  output_format_p->flags |= AVFMT_RAWPICTURE;
  ACE_ASSERT (!formatContext_);
  formatContext_ = avformat_alloc_context ();
  if (!formatContext_)
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

  return inherited::initialize (configuration_in,
                                allocator_in);

//error:
//  if (formatContext_)
//  {
//    if (formatContext_->streams[0])
//      if (formatContext_->streams[0]->codec)
//      {
//        result = avcodec_close (formatContext_->streams[0]->codec);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
//                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//      } // end IF

//    avformat_free_context (formatContext_);
//    formatContext_ = NULL;
//  } // end IF

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
                                       struct v4l2_format,
                                       UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  if (session_data_r.targetFileName.empty ())
    return; // nothing to do

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _riffchunk RIFF_chunk;
#else
  unsigned int riff_chunk_size = 0;
#endif

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  // *TODO*: remove type inference
  message_block_p =
    inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), returning\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    goto error;
  } // end IF

  if (isFirst_)
  {
    isFirst_ = false;

    if (!generateHeader (message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (message_block_p);

  result = message_block_p->copy (ACE_TEXT_ALWAYS_CHAR ("00db"),
                                  4);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  riff_chunk_size = message_inout->length ();
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    riff_chunk_size = ACE_SWAP_LONG (riff_chunk_size);
  result = message_block_p->copy (reinterpret_cast<char*> (&riff_chunk_size),
                                  4);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  message_block_p->cont (message_inout);
  result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
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
                                       struct v4l2_format,
                                       UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  if (!inherited::sessionData_)
    return;

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      struct v4l2_fract frame_rate_s =
          getFrameRate (session_data_r,
                        session_data_r.inputFormat);

      int result = -1;
      enum AVCodecID codec_id = AV_CODEC_ID_RAWVIDEO; // RGB
      struct AVCodec* codec_p = NULL;
      struct AVCodecContext* codec_context_p = NULL;
      struct AVStream* stream_p = NULL;

      ACE_ASSERT (formatContext_);
      ACE_ASSERT (formatContext_->oformat);

      formatContext_->oformat->audio_codec = AV_CODEC_ID_NONE;
      switch (session_data_r.inputFormat)
      {
        // RGB formats
        case AV_PIX_FMT_BGR24:
        case AV_PIX_FMT_RGB24:
//        case V4L2_PIX_FMT_BGR24:
//        case V4L2_PIX_FMT_RGB24:
//          codec_id = AV_CODEC_ID_RAWVIDEO;
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
        case V4L2_PIX_FMT_MJPEG:
          codec_id = AV_CODEC_ID_MJPEG;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown pixel format (was: %s), returning\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (session_data_r.inputFormat).c_str ())));
          goto error;
        }
      } // end SWITCH
      formatContext_->oformat->video_codec = codec_id;

      codec_p = avcodec_find_encoder (codec_id);
      if (!codec_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) failed: \"%m\", returning\n"),
                    inherited::mod_->name (),
                    codec_id));
        goto error;
      } // end IF
      ACE_ASSERT (!codec_context_p);
      codec_context_p = avcodec_alloc_context3 (codec_p);
      if (!codec_context_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result = avcodec_get_context_defaults3 (codec_context_p,
                                              codec_p);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_get_context_defaults3() failed: \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      codec_context_p->bit_rate =
          (av_image_get_buffer_size (session_data_r.inputFormat,
                                     session_data_r.sourceFormat.width,
                                     session_data_r.sourceFormat.height,
                                     1) * // *TODO*: linesize alignment
           frame_rate_s.numerator       *
           8);
      codec_context_p->codec_id = codec_id;
      codec_context_p->width = session_data_r.sourceFormat.width;
      codec_context_p->height = session_data_r.sourceFormat.height;
      codec_context_p->time_base.num = frame_rate_s.denominator;
      codec_context_p->time_base.den = frame_rate_s.numerator;
//      codec_context_p->gop_size = 10;
//      codec_context_p->max_b_frames = 1;
      codec_context_p->pix_fmt = session_data_r.inputFormat;

      // transform v4l format to libavformat type (AVPixelFormat)
//      switch (format_p->fmt.pix.pixelformat)
//      {
//        // RGB formats
//        case V4L2_PIX_FMT_BGR24:
//          codec_context_p->pix_fmt = AV_PIX_FMT_RGB24;
//          break;
//        case V4L2_PIX_FMT_RGB24:
//          codec_context_p->pix_fmt = AV_PIX_FMT_RGB24;
//          break;
//        // luminance-chrominance formats
//        case V4L2_PIX_FMT_YUV420: // 'YU12'
//          codec_context_p->pix_fmt = AV_PIX_FMT_YUV420P;
//          break;
//        case V4L2_PIX_FMT_YUYV:
//          codec_context_p->pix_fmt = AV_PIX_FMT_YUYV422;
//          break;
//        // compressed formats
//        // *NOTE*: "... MJPEG, or at least the MJPEG in AVIs having the MJPG
//        //         fourcc, is restricted JPEG with a fixed -- and *omitted* --
//        //         Huffman table. The JPEG must be YCbCr colorspace, it must be
//        //         4:2:2, and it must use basic Huffman encoding, not arithmetic
//        //         or progressive. . . . You can indeed extract the MJPEG frames
//        //         and decode them with a regular JPEG decoder, but you have to
//        //         prepend the DHT segment to them, or else the decoder won't
//        //         have any idea how to decompress the data. The exact table
//        //         necessary is given in the OpenDML spec. ..."
//        case V4L2_PIX_FMT_MJPEG:
//          codec_context_p->pix_fmt = AV_PIX_FMT_YUVJ422P;
//          break;
//        // *TODO*: ATM, libav cannot handle YVU formats
//        case V4L2_PIX_FMT_YVU420: // 'YV12'
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown pixel format (was: %d), returning\n"),
//                      format_p->fmt.pix.pixelformat));
//          break;
//        }
//      } // end SWITCH
//      codec_context_p->pix_fmt = codec_->pix_fmts[0];

      result = avcodec_open2 (codec_context_p,
                              codec_p,
                              NULL);
      if (result < 0)
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
      if (!stream_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (stream_p->codec);
      formatContext_->streams[0] = stream_p;

      // *TODO*: why does this need to be reset ?
      stream_p->codec->bit_rate =
          (av_image_get_buffer_size (session_data_r.inputFormat,
                                     session_data_r.sourceFormat.width,
                                     session_data_r.sourceFormat.height,
                                     1) * // *TODO*: linesize alignment
           frame_rate_s.numerator       *
           8);
      stream_p->codec->codec_id = codec_id;
      // stream_p->codec->codec_tag = 0;
    //  stream_p->codec->codec_type = codec_->type;

      stream_p->codec->pix_fmt = codec_context_p->pix_fmt;
      stream_p->codec->width = session_data_r.sourceFormat.width;
      stream_p->codec->height = session_data_r.sourceFormat.height;

      stream_p->time_base.num = frame_rate_s.denominator;
      stream_p->time_base.den = frame_rate_s.numerator;
//      stream_p->codec->time_base.num =
//          session_data_r.frameRate.numerator;
//      stream_p->codec->time_base.den =
//          session_data_r.frameRate.denominator;

      goto continue_;

error:
      if (codec_context_p)
        avcodec_free_context (&codec_context_p);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      int result = -1;
      ACE_Message_Block* message_block_p = NULL;

      if (session_data_r.targetFileName.empty ())
        goto continue_2; // nothing to do

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      // *TODO*: remove type inferences
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

      // *TODO*: remove type inference
      message_block_p =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
      if (!message_block_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), continuing\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->allocatorConfiguration->defaultBufferSize));
        goto continue_2;
      } // end IF
      ACE_ASSERT (message_block_p);

      if (!generateIndex (message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));

        // clean up
        message_block_p->release ();

        goto continue_2;
      } // end IF

      result = inherited::put_next (message_block_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));

        // clean up
        message_block_p->release ();

        goto continue_2;
      } // end IF

continue_2:
      avformat_free_context (formatContext_);
      formatContext_ = NULL;

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
                                       struct v4l2_format,
                                       UserDataType>::generateHeader (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_inout);

  int result = -1;
  ACE_ASSERT (!formatContext_->pb);
  formatContext_->pb =
    avio_alloc_context (reinterpret_cast<unsigned char*> (messageBlock_inout->wr_ptr ()), // buffer handle
                        messageBlock_inout->capacity (),          // buffer size
                        1,                                        // write flag
                        messageBlock_inout,                       // act
                        NULL,                                     // read callback
                        stream_decoder_aviencoder_libav_write_cb, // write callback
                        NULL);                                    // seek callback
  if (!formatContext_->pb)
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
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avformat_write_header() failed: \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  avio_flush (formatContext_->pb);

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
                                       struct v4l2_format,
                                       UserDataType>::generateIndex (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_inout);

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (formatContext_);

  result = av_write_trailer (formatContext_);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: av_write_trailer() failed: \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));

  return true;
}
#endif // ACE_WIN32 || ACE_WIN64

////////////////////////////////////////////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
static sox_bool
sox_overwrite_permitted (char const * filename_in)
{
  return sox_true;
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
          typename FormatType,
          typename UserDataType>
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            FormatType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            UserDataType>::Stream_Decoder_WAVEncoder_T (ISTREAM_T* stream_in)
#else
                            UserDataType>::Stream_Decoder_WAVEncoder_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , encodingInfo_ ()
 , signalInfo_ ()
 , outputFile_ (NULL)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::Stream_Decoder_WAVEncoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_OS::memset (&encodingInfo_, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&signalInfo_, 0, sizeof (struct sox_signalinfo_t));
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
          typename FormatType,
          typename UserDataType>
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType,
                            FormatType,
                            UserDataType>::~Stream_Decoder_WAVEncoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::~Stream_Decoder_WAVEncoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  int result = -1;
  if (outputFile_)
  {
    result = sox_close (outputFile_);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF

  result = sox_quit ();
  if (result != SOX_SUCCESS)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_quit(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
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
          typename FormatType,
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
                            FormatType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    result = sox_quit ();
    if (result != SOX_SUCCESS)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_quit(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
      return false;
    } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  result = sox_init ();
  if (result != SOX_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_init(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (sox_strerror (result))));
    return false;
  } // end IF
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
          typename FormatType,
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
                            FormatType,
                            UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::handleDataMessage"));

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!inherited::isActive_)
    return;
#else
  if (!outputFile_)
    return; // nothing to do
#endif // ACE_WIN32 || ACE_WIN64

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_Message_Block* message_block_p = NULL;
  struct _riffchunk RIFF_chunk;
#else
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inferences
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  message_block_p =
    inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: Stream_Decoder_AVIEncoder_WriterTask_T::allocateMessage(%d) failed: \"%m\", returning\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    goto error;
  } // end IF

  if (inherited::isFirst_)
  {
    inherited::isFirst_ = false;

    if (!generateHeader (message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Decoder_WAVEncoder_T::generateHeader(), returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (message_block_p);

  // db (--> Uncompressed video frame)
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = FCC ('00db');
  RIFF_chunk.cb = message_inout->length ();
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                  sizeof (struct _riffchunk));

  message_block_p->cont (message_inout);
  result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
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

  // *IMPORTANT NOTE*: sox_write() expects signed 32-bit samples
  //                   --> convert the data in memory first
  size_t samples_read, samples_written = 0;
  /* Temporary store whilst copying. */
  sox_sample_t samples[STREAM_DECODER_SOX_SAMPLE_BUFFERS];
  sox_format_t* memory_buffer_p =
      sox_open_mem_read (message_inout->rd_ptr (),
                         message_inout->length (),
                         &signalInfo_,
                         &encodingInfo_,
                         ACE_TEXT_ALWAYS_CHAR ("raw"));
  if (!memory_buffer_p)
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
                             STREAM_DECODER_SOX_SAMPLE_BUFFERS);
    if (!samples_read)
      break;
    samples_written = sox_write (outputFile_,
                                 samples,
                                 samples_read);
    if (samples_written != samples_read)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_write(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
  } while (true);

error:
  if (memory_buffer_p)
  {
    result = sox_close (memory_buffer_p);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (sox_strerror (result))));
  } // end IF

  return;
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
          typename FormatType,
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
                            FormatType,
                            UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!inherited::sessionData_)
    return;

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
#else
  ACE_ASSERT (inherited::configuration_);
#endif // ACE_WIN32 || ACE_WIN64

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // sanity check(s)
      if (inherited::configuration_->targetFileName.empty ())
        return;
      ACE_ASSERT (inherited::configuration_->format);

//      sox_comments_t comments = ;
      struct sox_oob_t oob_data;
      ACE_OS::memset (&oob_data, 0, sizeof (struct sox_oob_t));
//      oob_data.comments = comments;
//      oob_data.instr;
//      oob_data.loops;
      Stream_Module_Decoder_Tools::ALSAToSoX (inherited::configuration_->format->format,
                                              inherited::configuration_->format->rate,
                                              inherited::configuration_->format->channels,
                                              encodingInfo_,
                                              signalInfo_);
      ACE_ASSERT (!outputFile_);
      outputFile_ =
          sox_open_write (inherited::configuration_->targetFileName.c_str (),
                          &signalInfo_,
                          &encodingInfo_,
                          //ACE_TEXT_ALWAYS_CHAR (STREAM_DECODER_SOX_WAV_FORMATTYPE_STRING),
                          NULL,
                          &oob_data,
                          sox_overwrite_permitted);
      if (!outputFile_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sox_open_write(\"%s\"): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->targetFileName.c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened file stream \"%s\"...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->targetFileName.c_str ())));

      goto continue_;

error:
      if (outputFile_)
      {
        result = sox_close (outputFile_);
        if (result != SOX_SUCCESS)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sox_close(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (sox_strerror (result))));
        outputFile_ = NULL;
      } // end IF

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
      struct _AMMediaType* media_type_p = NULL;
      unsigned char* wave_header_p = NULL;
      unsigned int wave_header_size = 0;
      ssize_t result_2 = -1;
      struct _rifflist* RIFF_wave_p = NULL;
      struct _riffchunk* RIFF_chunk_fmt_p = NULL;
      struct _riffchunk* RIFF_chunk_data_p = NULL;

      if (session_data_r.targetFileName.empty ())
        goto continue_2;

      if (!Common_File_Tools::open (session_data_r.targetFileName, // FQ file name
                                    O_RDWR | O_BINARY,             // flags
                                    file_IO))                      // return value: stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (session_data_r.targetFileName.c_str ())));
        break;
      } // end IF
      close_file = true;

      // sanity check(s)
      ACE_ASSERT (session_data_r.inputFormat);

      struct _AMMediaType& media_type_r =
        inherited::getFormat (session_data_r.inputFormat);
      ACE_ASSERT (media_type_r.formattype == FORMAT_WaveFormatEx);

      wave_header_size =
        (sizeof (struct _rifflist)  +
         sizeof (struct _riffchunk) +
         media_type_r.cbFormat      +
         sizeof (struct _riffchunk));
      ACE_NEW_NORETURN (wave_header_p,
                        unsigned char[wave_header_size]);
      if (!wave_header_p)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF
      result_2 = file_IO.recv_n (wave_header_p, wave_header_size);
      if (result_2 != wave_header_size)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::recv_n(%d): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    wave_header_size));
        goto error_2;
      } // end IF

      RIFF_wave_p = reinterpret_cast<struct _rifflist*> (wave_header_p);
      RIFF_chunk_fmt_p = reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
      RIFF_chunk_data_p =
        reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
                                              media_type_r.cbFormat);

      // update RIFF header sizes
      RIFF_chunk_data_p->cb =
        static_cast<DWORD> (session_data_r.statistic.bytes);
      RIFF_wave_p->cb =
        (static_cast<DWORD> (session_data_r.statistic.bytes) +
         wave_header_size         -
         sizeof (struct _riffchunk));

      result_2 = file_IO.send_n (wave_header_p, wave_header_size);
      if (result_2 != wave_header_size)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::send_n(%d): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    wave_header_size));
        goto error_2;
      } // end IF

      // clean up
      media_type_p = &media_type_r;
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      delete [] wave_header_p;
      result = file_IO.close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      close_file = false;

      goto continue_2;

error_2:
      media_type_p = &media_type_r;
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);
      if (wave_header_p)
        delete [] wave_header_p;
      if (close_file)
      {
        result = file_IO.close ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_FILE_IO::close(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
      } // end IF

continue_2:
#else
      if (outputFile_)
      {
        sox_uint64_t bytes_written = outputFile_->tell_off;
        result = sox_close (outputFile_);
        if (result != SOX_SUCCESS)
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
          typename FormatType,
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
                            FormatType,
                            UserDataType>::generateHeader (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::generateHeader"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (messageBlock_inout);

  const SessionDataType& session_data_r = inherited::sessionData_->getR ();

  // sanity check(s)
  ACE_ASSERT (session_data_r.inputFormat);

  struct _AMMediaType& media_type_r =
    inherited::getFormat (session_data_r.inputFormat);
  ACE_ASSERT (media_type_r.formattype == FORMAT_WaveFormatEx);

  //ACE_ASSERT (media_type_p->pbFormat);
  //struct tWAVEFORMATEX* waveformatex_p =
  //  reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
  //ACE_ASSERT (waveformatex_p);

  struct _rifflist* RIFF_wave_p =
    reinterpret_cast<struct _rifflist*> (messageBlock_inout->wr_ptr ());
  struct _riffchunk* RIFF_chunk_fmt_p =
    reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
  struct _riffchunk* RIFF_chunk_data_p =
    reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
                                          media_type_r.cbFormat);

  RIFF_chunk_data_p->fcc = FCC ('data');
  RIFF_chunk_data_p->cb = 0; // update here

  RIFF_chunk_fmt_p->fcc = FCC ('fmt ');
  RIFF_chunk_fmt_p->cb = media_type_r.cbFormat;
  ACE_OS::memcpy (RIFF_chunk_fmt_p + 1,
                  media_type_r.pbFormat,
                  RIFF_chunk_fmt_p->cb);

  RIFF_wave_p->fcc = FCC ('RIFF');
  RIFF_wave_p->cb = 0 + // ... and here
                    (sizeof (struct _rifflist)  +
                     sizeof (struct _riffchunk) +
                     media_type_r.cbFormat      +
                     sizeof (struct _riffchunk)) -
                     sizeof (struct _riffchunk);
  RIFF_wave_p->fccListType = FCC ('WAVE');

  messageBlock_inout->wr_ptr (RIFF_wave_p->cb + sizeof (struct _riffchunk));

  // clean up
  struct _AMMediaType* media_type_p = &media_type_r;
  Stream_MediaFramework_DirectShow_Tools::deleteMediaType (media_type_p);

  return true;
}
#endif // ACE_WIN32 || ACE_WIN64
