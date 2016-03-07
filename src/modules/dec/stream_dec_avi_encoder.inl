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
#include "aviriff.h"
#include "dvdmedia.h"
#include "fourcc.h"
//#include "Streams.h"
#else
#endif

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_ReaderTask_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_ReaderTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::~Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
int
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::put (ACE_Message_Block* messageBlock_in,
                                                              ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::put"));

  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_IOCTL:
    {
      SessionDataType* session_data_p =
          reinterpret_cast<SessionDataType*> (messageBlock_in->base ());
      ACE_ASSERT (session_data_p);

      // *TODO*: remove type inference
      if (!postProcessHeader (session_data_p->targetFileName))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader(\"%s\"), aborting\n"),
                    ACE_TEXT (session_data_p->targetFileName.c_str ())));
        return -1;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), aborting\n"),
                  messageBlock_in->msg_type ()));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::postProcessHeader (const std::string& filename_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader"));

  ACE_UNUSED_ARG (filename_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
}

////////////////////////////////////////////////////////////////////////////////

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_WriterTask_T ()
 : inherited ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , isFirst_ (true)
 , isInitialized_ (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaType_ (NULL)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::Stream_Decoder_AVIEncoder_WriterTask_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::~Stream_Decoder_AVIEncoder_WriterTask_T"));

  if (sessionData_)
    sessionData_->decrease ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::initialize"));

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    configuration_ = NULL;
    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    isFirst_ = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    mediaType_ = NULL;
#endif

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isInitialized_ = true;

  return isInitialized_;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
const ConfigurationType&
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleDataMessage"));

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _riffchunk RIFF_chunk;
#endif

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);
  ACE_ASSERT (isInitialized_);

  // initialize driver ?
  if (isFirst_)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // sanity check(s)
    ACE_ASSERT (mediaType_);

    if ((mediaType_->formattype != FORMAT_VideoInfo) &&
        (mediaType_->formattype != FORMAT_VideoInfo2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media format type (was: %d), returning\n"),
                  mediaType_->formattype));
      return;
    } // end IF
    struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
    struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
    if (mediaType_->formattype == FORMAT_VideoInfo)
      video_info_header_p = (struct tagVIDEOINFOHEADER*)mediaType_->pbFormat;
    else if (mediaType_->formattype == FORMAT_VideoInfo2)
      video_info_header2_p = (struct tagVIDEOINFOHEADER2*)mediaType_->pbFormat;

    // *TODO*: remove type inference
    message_block_p =
      allocateMessage (configuration_->streamConfiguration->bufferSize);
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", returning\n"),
                  configuration_->streamConfiguration->bufferSize));
      return;
    } // end IF
    ACE_ASSERT (message_block_p);

    // RIFF header
    struct _rifflist RIFF_list;
    ACE_OS::memset (&RIFF_list, 0, sizeof (struct _rifflist));
    RIFF_list.fcc = FCC ('RIFF');
    // *NOTE*: in a streaming scenario, this would need to be added AFTER the
    //         file has been written (or the disc runs out of space), which is
    //         impossible until/unless this value is preconfigured in some way.
    //         Notice how this oversight confounds the whole standard
    // sizeof (fccListType) [4] + sizeof (data) --> == total (file) size - 8
    RIFF_list.cb = sizeof (FOURCC) +
      sizeof (struct _rifflist) +           // hdrl
      sizeof (struct _avimainheader) +
      // sizeof (LIST strl)
      sizeof (struct _rifflist) +
      sizeof (struct _avistreamheader) +    // strh
      sizeof (struct _riffchunk) +          // strf
      sizeof (struct tagBITMAPINFOHEADER) + // strf
      sizeof (struct _riffchunk) +          // JUNK
      1820 +                                // pad bytes
      sizeof (struct _rifflist) +           // movi
      sizeof (struct _riffchunk) +          // 00db
      message_inout->length ();             // (part of) frame
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
    RIFF_list.fccListType = FCC ('AVI ');
    result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_list),
                                    sizeof (struct _rifflist));

    // hdrl
    RIFF_list.fcc = FCC ('LIST');
    // sizeof (fccListType) [4] + sizeof (LIST data)
    RIFF_list.cb = sizeof (FOURCC) +
      sizeof (struct _avimainheader) +
      // sizeof (LIST strl)
      sizeof (struct _rifflist) +
      sizeof (struct _avistreamheader) + // strh
      sizeof (struct _riffchunk) + // strf
      sizeof (struct tagBITMAPINFOHEADER); // strf
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
    RIFF_list.fccListType = FCC ('hdrl');
    result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_list),
                                    sizeof (struct _rifflist));

    // *NOTE*: "...the 'hdrl' list begins with the main AVI header, which is
    //         contained in an 'avih' chunk. ..."
    struct _avimainheader AVI_header_avih;
    ACE_OS::memset (&AVI_header_avih, 0, sizeof (struct _avimainheader));
    AVI_header_avih.fcc = ckidMAINAVIHEADER;
    AVI_header_avih.cb = sizeof (struct _avimainheader) - 8;
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      AVI_header_avih.cb = ACE_SWAP_LONG (AVI_header_avih.cb);
    AVI_header_avih.dwMicroSecPerFrame =
      ((mediaType_->formattype == FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
                                                    : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame));
    AVI_header_avih.dwMaxBytesPerSec =
      ((mediaType_->formattype == FORMAT_VideoInfo) ? video_info_header_p->dwBitRate
                                                    : video_info_header2_p->dwBitRate) / 8;
    AVI_header_avih.dwPaddingGranularity = STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN;
    AVI_header_avih.dwFlags = AVIF_WASCAPTUREFILE;
    //AVI_header_avih.dwTotalFrames = 0; // unreliable
    //AVI_header_avih.dwInitialFrames = 0;
    AVI_header_avih.dwStreams = 1;
    //AVI_header_avih.dwSuggestedBufferSize = 0; // unreliable
    AVI_header_avih.dwWidth =
      ((mediaType_->formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biWidth
                                                    : video_info_header2_p->bmiHeader.biWidth);
    AVI_header_avih.dwHeight =
      ((mediaType_->formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biHeight
                                                    : video_info_header2_p->bmiHeader.biHeight);
    //AVI_header_avih.dwReserved = {0, 0, 0, 0};
    result = message_block_p->copy (reinterpret_cast<char*> (&AVI_header_avih),
                                    sizeof (struct _avimainheader));

    // *NOTE*: "One or more 'strl' lists follow the main header. A 'strl' list
    //         is required for each data stream. Each 'strl' list contains
    //         information about one stream in the file, and must contain a
    //         stream header chunk ('strh') and a stream format chunk ('strf').
    //         ..."
    // strl
    RIFF_list.fcc = FCC ('LIST');
    // sizeof (fccListType) [4] + sizeof (LIST data)
    RIFF_list.cb = sizeof (FOURCC) +
      sizeof (struct _avistreamheader) + // strh
      sizeof (struct _riffchunk) + // strf
      sizeof (struct tagBITMAPINFOHEADER); // strf
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
    RIFF_list.fccListType = ckidSTREAMLIST;
    result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_list),
                                    sizeof (struct _rifflist));

    // strl --> strh
    struct _avistreamheader AVI_header_strh;
    ACE_OS::memset (&AVI_header_strh, 0, sizeof (struct _avistreamheader));
    AVI_header_strh.fcc = ckidSTREAMHEADER;
    AVI_header_strh.cb = sizeof (struct _avistreamheader) - 8;
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      AVI_header_strh.cb = ACE_SWAP_LONG (AVI_header_strh.cb);
    AVI_header_strh.fccType = streamtypeVIDEO;
    FOURCCMap fourcc_map (&mediaType_->subtype);
    AVI_header_strh.fccHandler = fourcc_map.GetFOURCC ();
    //AVI_header_strh.fccHandler = 0;
    //AVI_header_strh.dwFlags = 0;
    //AVI_header_strh.wPriority = 0;
    //AVI_header_strh.wLanguage = 0;
    //AVI_header_strh.dwInitialFrames = 0;
    // *NOTE*: dwRate / dwScale == fps
    AVI_header_strh.dwScale = 10000; // 100th nanoseconds --> seconds ???
    AVI_header_strh.dwRate =
      ((mediaType_->formattype == FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
                                                    : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame));
    //AVI_header_strh.dwStart = 0;
    //AVI_header_strh.dwLength = 0;
    //AVI_header_strh.dwSuggestedBufferSize = 0;
    AVI_header_strh.dwQuality = -1; // default
                                    //AVI_header_strh.dwSampleSize = 0;
                                    //AVI_header_strh.rcFrame = {0, 0, 0, 0};
    result = message_block_p->copy (reinterpret_cast<char*> (&AVI_header_strh),
                                    sizeof (struct _avistreamheader));

    // strl --> strf
    // *NOTE*: there is no definition for AVI stream format chunks, as their
    //         contents differ, depending on the stream type
    struct _riffchunk RIFF_chunk;
    ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
    RIFF_chunk.fcc = ckidSTREAMFORMAT;
    RIFF_chunk.cb = sizeof (struct tagBITMAPINFOHEADER);
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                    sizeof (struct _riffchunk));
    struct tagBITMAPINFOHEADER AVI_header_strf;
    ACE_OS::memset (&AVI_header_strf, 0, sizeof (struct tagBITMAPINFOHEADER));
    AVI_header_strf =
      ((mediaType_->formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader
                                                    : video_info_header2_p->bmiHeader);
    result = message_block_p->copy (reinterpret_cast<char*> (&AVI_header_strf),
                                    sizeof (struct tagBITMAPINFOHEADER));

    // strl --> strd
    // strl --> strn

    // --> END strl

    // insert JUNK chunk to align the 'movi' chunk at 2048 bytes
    // --> should speed up CD-ROM access
    unsigned int pad_bytes =
      AVI_header_avih.dwPaddingGranularity - message_block_p->length () - 8 - 12;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("inserting JUNK chunk (%d pad byte(s))...\n"),
                pad_bytes));
    RIFF_chunk.fcc = FCC ('JUNK');
    RIFF_chunk.cb = pad_bytes;
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                    sizeof (struct _riffchunk));
    ACE_OS::memset (message_block_p->wr_ptr (), 0, pad_bytes);
    message_block_p->wr_ptr (RIFF_chunk.cb);

    // movi
    RIFF_list.fcc = FCC ('LIST');
    // *NOTE*: see above
    // sizeof (fccListType) [4] + sizeof (LIST data)
    RIFF_list.cb = sizeof (FOURCC) +
      sizeof (struct _riffchunk) + // 00db
      message_inout->length ();    // (part of) frame
    if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
      RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    RIFF_list.fccListType = FCC ('movi');
    result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_list),
                                    sizeof (struct _rifflist));

    //RIFF_chunk.fcc = FCC ('00db');
    //RIFF_chunk.cb = message_inout->length ();
    //if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    //  RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
    //result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
    //                                sizeof (struct _riffchunk));

    //struct _avisuperindex AVI_header_indx;
    //ACE_OS::memset (&AVI_header_indx, 0, sizeof (struct _avisuperindex));
    //AVI_header_indx.fcc = 0;
    //AVI_header_indx.cb = 0;
    //AVI_header_indx.wLongsPerEntry = 0;
    //AVI_header_indx.bIndexSubType = 0;
    //AVI_header_indx.bIndexType = 0;
    //AVI_header_indx.nEntriesInUse = 0;
    //AVI_header_indx.dwChunkId = 0;
    //AVI_header_indx.dwReserved = 0;
    ////AVI_header_indx.aIndex = 0;
    //struct _avisuperindex_entry AVI_header_indx_entry_0;
    //ACE_OS::memset (&AVI_header_indx_entry_0, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_1;
    //ACE_OS::memset (&AVI_header_indx_entry_1, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_2;
    //ACE_OS::memset (&AVI_header_indx_entry_2, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_3;
    //ACE_OS::memset (&AVI_header_indx_entry_3, 0, sizeof (struct _avisuperindex_entry));
    //result =
    //  message_block_p->copy (reinterpret_cast<char*> (&AVI_header_indx),
    //                   sizeof (struct _avisuperindex));
#endif

    result = inherited::put_next (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", returning\n")));

      // clean up
      message_block_p->release ();

      return;
    } // end IF
    message_block_p = NULL;

    isFirst_ = false;
  } // end IF

  // *TODO*: remove type inference
  message_block_p =
    allocateMessage (configuration_->streamConfiguration->bufferSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("allocateMessage(%d) failed: \"%m\", returning\n"),
                configuration_->streamConfiguration->bufferSize));
    return;
  } // end IF
  ACE_ASSERT (message_block_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // db (--> Uncompressed video frame)
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = FCC ('00db');
  RIFF_chunk.cb = message_inout->length ();
  result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                  sizeof (struct _riffchunk));
#endif

  message_block_p->cont (message_inout);
  result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", returning\n")));

    // clean up
    message_block_p->release ();

    return;
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);

      sessionData_ =
        &const_cast<SessionDataContainerType&> (message_inout->get ());
      sessionData_->increase ();
      const SessionDataType& session_data_r = sessionData_->get ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // sanity check(s)
      ACE_ASSERT (mediaType_);
      // *TODO*: remove type inference
      ACE_ASSERT (session_data_r.sampleGrabber);
      HRESULT result =
        session_data_r.sampleGrabber->GetConnectedMediaType (mediaType_);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ISampleGrabber::GetConnectedMediaType(): \"%m\", returning\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        return;
      } // end IF
#endif

      break;
    }
    case STREAM_SESSION_END:
    {
      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
MessageType*
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::allocateMessage"));

  // initialize return value(s)
  MessageType* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);

  if (configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      message_block_p =
        static_cast<MessageType*> (configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p && !configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      MessageType (requestedSize_in));
  if (!message_block_p)
  {
    if (configuration_->streamConfiguration->messageAllocator)
    {
      if (configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate MessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate MessageType: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}
