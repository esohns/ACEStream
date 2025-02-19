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

#ifndef STREAM_DEC_AVI_ENCODER_H
#define STREAM_DEC_AVI_ENCODER_H

#include <utility>
#include <vector>

#include "ace/Basic_Types.h"
#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dec_common.h"

// forward declaration(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
struct AVCodecContext;
struct AVFormatContext;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;

extern const char libacestream_default_dec_avi_encoder_module_name_string[];

enum Stream_Decoder_AVIIndexType : int
{
  STREAM_AVI_INDEX_V1 = 0, // original standard
  STREAM_AVI_INDEX_V2,     // odml 1.02
  ////////////////////////////////////////
  STREAM_AVI_INDEX_MAX,
  STREAM_AVI_INDEX_INVALID
};

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
class Stream_Decoder_AVIEncoder_WriterTask_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
int
stream_decoder_aviencoder_libav_write_cb (void*,    // act
                                          uint8_t*, // buffer address
                                          int);     // buffer size
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          typename SessionDataType,
          ////////////////////////////////
          typename MediaType,
          typename UserDataType>
class Stream_Decoder_AVIEncoder_ReaderTask_T
 : public ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType>
{
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> inherited;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;
  typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                                 TimePolicyType,
                                                 ConfigurationType,
                                                 ControlMessageType,
                                                 DataMessageType,
                                                 SessionMessageType,
                                                 SessionDataContainerType,
                                                 SessionDataType,
                                                 MediaType,
                                                 UserDataType> WRITER_TASK_T;

  Stream_Decoder_AVIEncoder_ReaderTask_T (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Decoder_AVIEncoder_ReaderTask_T () {}

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_ReaderTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_ReaderTask_T (const Stream_Decoder_AVIEncoder_ReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_ReaderTask_T& operator= (const Stream_Decoder_AVIEncoder_ReaderTask_T&))

  // helper function(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool postProcessHeader (const std::string&); // file name
#endif // ACE_WIN32 || ACE_WIN64
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          typename SessionDataType,
          ////////////////////////////////
          typename MediaType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Decoder_AVIEncoder_WriterTask_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  friend class Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                                      TimePolicyType,
                                                      ConfigurationType,
                                                      ControlMessageType,
                                                      DataMessageType,
                                                      SessionMessageType,
                                                      SessionDataContainerType,
                                                      SessionDataType,
                                                      MediaType,
                                                      UserDataType>;

  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  // convenient types
  typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                                 TimePolicyType,
                                                 ConfigurationType,
                                                 ControlMessageType,
                                                 DataMessageType,
                                                 SessionMessageType,
                                                 SessionDataContainerType,
                                                 SessionDataType,
                                                 MediaType,
                                                 UserDataType> READER_T;

  Stream_Decoder_AVIEncoder_WriterTask_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_AVIEncoder_WriterTask_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // *NOTE*: the RIFF-AVI (storage) format specifies a header that contains size
  //         fields with information about the length of the consecutive,
  //         linearly structured bulk data.
  //         Note how in a (streaming) scenario continuously generating data,
  //         this information often is not available during initial processing
  //         and may therefore have to be filled in in a post-processing step
  //         after the stream ends, potentially requiring reparsing of (written)
  //         data. This means that, unless configuration data (duration[,
  //         format]) is supplied externally - either through session
  //         data/and or module configuration, the encoding process must be
  //         split into two separate phases (or distinct processing modules -
  //         more adequate for pipelined processing), in order to generate
  //         standard-compliant files. This implementation fills in the size
  //         information upon reception of completion event messages sent
  //         upstream by trailing modules of the processing stream (i.e. reader-
  //         side processing)
  bool                              isFirst_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
  struct AVCodecContext*            audioCodecContext_;
  struct AVCodecContext*            videoCodecContext_;
  struct AVFormatContext*           formatContext_;
  ACE_INT64                         videoSamples_;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INT64                         audioSamples_;
  ACE_UINT32                        audioFrames_;
  ACE_UINT32                        audioFrameSize_;
  ACE_UINT32                        videoFrameSize_;
  // *NOTE*: global offset from start of first RIFF chunk
  ACE_UINT64                        currentOffset_;
  // *NOTE*: offset from start of current RIFF chunk (AVI[ X])
  ACE_UINT32                        currentRIFFOffset_;
  bool                              headerWritten_;

  typedef std::vector<std::pair<ACE_UINT64, ACE_UINT32> > RIFF_OFFSETS_AND_SIZES_T;
  typedef RIFF_OFFSETS_AND_SIZES_T::iterator RIFF_OFFSETS_AND_SIZES_ITERATOR_T;
  RIFF_OFFSETS_AND_SIZES_T          RIFFOffsetsAndSizes_;

  typedef std::vector<ACE_UINT64> FRAME_OFFSETS_T;
  typedef FRAME_OFFSETS_T::const_iterator FRAMEOFFSETSITERATOR_T;
  FRAME_OFFSETS_T                   frameOffsets_; // video-
  // *NOTE*: when this reaches >1Gb, add another AVIX header to the file
  ACE_UINT64                        currentFrameOffset_;
  ACE_UINT32                        lastIndex1FrameOffsetIndex_;
  ACE_UINT32                        lastIndex1AudioFrames_; // needed for 'movi' list length calculation
  ACE_UINT64                        lastIndex1AudioSamples_; // needed for 'movi' list length calculation

  enum Stream_Decoder_AVIIndexType  indexType_; // AVI 1.0 "idx1" at end of file / "inx1" + super-index

  // helper methods
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool generateHeader (ACE_Message_Block*); // message buffer handle
  bool generateAVIXHeader (ACE_Message_Block*); // message buffer handle
#else
  bool generateHeader ();
#endif // ACE_WIN32 || ACE_WIN64
  bool generateIndex (enum Stream_Decoder_AVIIndexType, // index version
                      ACE_Message_Block*);              // message buffer handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))
};

// include template definition
#include "stream_dec_avi_encoder.inl"

#endif
