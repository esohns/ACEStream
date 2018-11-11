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

#include <string>
#include <vector>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfobjects.h>
#include <strmif.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#ifdef __cplusplus
}
#endif /* __cplusplus */
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//#include <sndfile.h>
#include "sox.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "common_time_common.h"

#include "common_ui_common.h"

#include "stream_istreamcontrol.h"
#include "stream_task_base_synch.h"

#include "stream_dec_common.h"

#include "stream_dev_tools.h"

// forward declaration(s)
struct AVFormatContext;
struct SwsContext;
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;

extern const char libacestream_default_dec_avi_encoder_module_name_string[];
extern const char libacestream_default_dec_wav_encoder_module_name_string[];

enum Stream_Decoder_AVIIndexType : int
{
  STREAM_AVI_INDEX_V1 = 0,
  STREAM_AVI_INDEX_V2,
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
  virtual bool postProcessHeader (const std::string&); // file name
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
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
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
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Decoder_AVIEncoder_WriterTask_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Decoder_AVIEncoder_WriterTask_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
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
  bool                    isActive_;
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
  bool                    isFirst_;

  enum AVPixelFormat      format_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct AVFormatContext* formatContext_;
#endif // ACE_WIN32 || ACE_WIN64
  unsigned int            frameSize_; // output-
  unsigned int            height_;
  unsigned int            width_;
  struct SwsContext*      transformContext_;

  typedef std::vector<unsigned int> FRAMEOFFSETS_T;
  typedef FRAMEOFFSETS_T::const_iterator FRAMEOFFSETSITERATOR_T;
  unsigned int            currentFrameOffset_;
  FRAMEOFFSETS_T          frameOffsets_;
  bool                    writeAVI1Index_; // AVI 1.0 "idx1" at end of file
  bool                    writeAVI2Index_; // AVI 2.0 "inx1" + super-index

  // helper methods
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle
  bool generateIndex (enum Stream_Decoder_AVIIndexType, // index version
                      ACE_Message_Block*);              // message buffer handle
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: return values need to be Stream_Module_Device_DirectShow_Tools::free'd !
  AM_MEDIA_TYPE getMediaType (const MediaType& mediaType_in) { return getMediaType_impl (mediaType_in); }
#else
  enum AVPixelFormat getMediaType (const MediaType& mediaType_in) { return getMediaType_impl (mediaType_in); }
#endif // ACE_WIN32 || ACE_WIN64
  Common_UI_Resolution_t getResolution (const SessionDataType& sessionData_in, const MediaType& mediaType_in) { return getResolution_impl (sessionData_in, mediaType_in); }
  struct AVRational getFrameRate (const SessionDataType& sessionData_in, const MediaType& mediaType_in) { return getFrameRate_impl (sessionData_in, mediaType_in); }

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  AM_MEDIA_TYPE getMediaType_impl (const struct _AMMediaType&);
  AM_MEDIA_TYPE getMediaType_impl (const IMFMediaType*&);
  Common_UI_Resolution_t getResolution_impl (const SessionDataType&,
                                             const struct _AMMediaType&);
  Common_UI_Resolution_t getResolution_impl (const SessionDataType&,
                                             const IMFMediaType*&);
  struct AVRational getFrameRate_impl (const SessionDataType&,
                                       const struct _AMMediaType&);
  struct AVRational getFrameRate_impl (const SessionDataType&,
                                       const IMFMediaType*&);
#else
  inline enum AVPixelFormat getMediaType_impl (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in) { return Stream_Device_Tools::v4l2FormatToffmpegFormat (mediaType_in.format.pixelformat); }
  inline enum AVPixelFormat getMediaType_impl (const struct Stream_MediaFramework_ALSA_MediaType&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (AV_PIX_FMT_NONE); ACE_NOTREACHED (return AV_PIX_FMT_NONE;) }
  inline Common_UI_Resolution_t getResolution_impl (const SessionDataType&, const struct Stream_MediaFramework_V4L_MediaType& mediaType_in) { Common_UI_Resolution_t return_value; return_value.width = mediaType_in.format.width; return_value.height = mediaType_in.format.height; return return_value; }
  inline Common_UI_Resolution_t getResolution_impl (const SessionDataType&, const struct Stream_MediaFramework_ALSA_MediaType&) { Common_UI_Resolution_t return_value; ACE_ASSERT (false); ACE_NOTSUP_RETURN (return_value); ACE_NOTREACHED (return return_value;) }
  inline struct AVRational getFrameRate_impl (const SessionDataType&, const struct Stream_MediaFramework_V4L_MediaType& mediaType_in) { struct AVRational return_value; return_value.num = mediaType_in.frameRate.numerator; return_value.den = mediaType_in.frameRate.denominator; return return_value; }
  inline struct AVRational getFrameRate_impl (const SessionDataType&, const struct Stream_MediaFramework_ALSA_MediaType&) { struct AVRational return_value; ACE_ASSERT (false); ACE_NOTSUP_RETURN (return_value); ACE_NOTREACHED (return return_value;) }
#endif // ACE_WIN32 || ACE_WIN64
//  inline struct AVRational getFrameRate_impl (const SessionDataType&, const enum AVPixelFormat&) { ACE_ASSERT (inherited::configuration_); ACE_ASSERT (inherited::configuration_->outputFormat); return inherited::configuration_->outputFormat->frameRate; }
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
// partial specialization (for V4L2)
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
          typename UserDataType>
class Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataContainerType,
                                             SessionDataType,
                                             struct Stream_MediaFramework_V4L_MediaType,
                                             UserDataType>
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

 public:
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
  bool             isFirst_;
  AVFormatContext* formatContext_;

  // helper methods
  DataMessageType* allocateMessage (unsigned int); // requested size
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle

  template <typename MediaType> struct Stream_MediaFramework_V4L_MediaType getMediaType (const MediaType& format_in) { return getMediaType_impl (format_in); }

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))

  bool generateIndex (ACE_Message_Block*); // message buffer handle

  inline const struct Stream_MediaFramework_V4L_MediaType getMediaType_impl (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in) { return const_cast<struct Stream_MediaFramework_V4L_MediaType&> (mediaType_in); }
};
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

// *NOTE*: the WAV format is a (non standard-compliant) RIFF container
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
class Stream_Decoder_WAVEncoder_T
 : public Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                                 TimePolicyType,
                                                 ConfigurationType,
                                                 ControlMessageType,
                                                 DataMessageType,
                                                 SessionMessageType,
                                                 SessionDataContainerType,
                                                 SessionDataType,
                                                 MediaType,
                                                 UserDataType>
{
  typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                                 TimePolicyType,
                                                 ConfigurationType,
                                                 ControlMessageType,
                                                 DataMessageType,
                                                 SessionMessageType,
                                                 SessionDataContainerType,
                                                 SessionDataType,
                                                 MediaType,
                                                 UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Decoder_WAVEncoder_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Decoder_WAVEncoder_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Decoder_WAVEncoder_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T (const Stream_Decoder_WAVEncoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T& operator= (const Stream_Decoder_WAVEncoder_T&))

  // helper methods
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle
#endif // ACE_WIN32 || ACE_WIN64

  // *NOTE*: the WAV (storage) format (like many others) foresees a
  //         header that contains size fields with information about
  //         the length of the consecutive, linearly structured bulk data.
  //         Note how in a (streaming) scenario continuously generating data,
  //         this information often is not available during initial processing
  //         and therefore has to be filled in in a post-processing step after
  //         the stream ends, potentially requiring reparsing of (written) data.
  //         This means that, unless configuration data (duration[, format]) is
  //         supplied externally - either through session data/and or module
  //         configuration, the encoding process has to be split into two
  //         separate phases (or distinct processing modules - more adequate for
  //         pipelined processing), in order to generate ('standard-')compliant
  //         files. This implementation fills in the size information upon
  //         reception of completion event messages sent upstream by trailing
  //         modules of the processing stream (i.e. reader-side processing)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct sox_encodinginfo_t encodingInfo_;
  struct sox_signalinfo_t   signalInfo_;

  struct sox_format_t*      outputFile_;
#endif // ACE_WIN32 || ACE_WIN64
};

// include template definition
#include "stream_dec_avi_encoder.inl"

#endif
