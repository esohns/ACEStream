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

#include "stream_istreamcontrol.h"
#include "stream_task_base_synch.h"

#include "stream_dec_common.h"

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
          typename FormatType,
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
          typename FormatType,
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
                                                 FormatType,
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
          typename FormatType,
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
                                                      FormatType,
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

  enum AVPixelFormat format_;
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: return values needs to be Stream_Module_Device_DirectShow_Tools::deleteMediaType()d !
  template <typename FormatType2> AM_MEDIA_TYPE& getFormat (const FormatType2& format_in) { return getFormat_impl (format_in); }
#else
  template <typename FormatType2> enum AVPixelFormat& getFormat (const FormatType2& format_in) { return getFormat_impl (format_in); }
#endif // ACE_WIN32 || ACE_WIN64
  template <typename FormatType2> AVRational& getFrameRate (const SessionDataType& sessionData_in, const FormatType2& format_in) { return getFrameRate_impl (sessionData_in, format_in); }
  template <typename FormatType2> void getResolution (const SessionDataType& sessionData_in, const FormatType2& format_in, unsigned int& height_out, unsigned int& width_out) { getResolution_impl (sessionData_in, format_in, height_out, width_out); }
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle
  bool generateIndex (enum Stream_Decoder_AVIIndexType, // index version
                      ACE_Message_Block*);              // message buffer handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType& getFormat_impl (const struct _AMMediaType&);
  struct _AMMediaType& getFormat_impl (const IMFMediaType&);
  struct AVRational& getFrameRate_impl (const SessionDataType&,
                                        const struct _AMMediaType&);
  void getResolution_impl (const SessionDataType&,
                           const struct _AMMediaType&,
                           unsigned int&,
                           unsigned int&);
#else
  inline enum AVPixelFormat& getFormat_impl (const enum AVPixelFormat& format_in) { return const_cast<enum AVPixelFormat&> (format_in); }
  inline enum AVPixelFormat& getFormat_impl (const struct Stream_Module_Device_ALSAConfiguration&) { enum AVPixelFormat format_e = AV_PIX_FMT_NONE; ACE_ASSERT (false); ACE_NOTSUP_RETURN (format_e); ACE_NOTREACHED (return format_e;) }
  inline struct AVRational& getFrameRate_impl (const SessionDataType&, const Stream_Module_Device_ALSAConfiguration&) { struct AVRational rational_s; ACE_ASSERT (false); ACE_NOTSUP_RETURN (rational_s); ACE_NOTREACHED (return rational_s;) }
  inline void getResolution_impl (const SessionDataType& sessionData_in, const enum AVPixelFormat&, unsigned int& width_out, unsigned int& height_out) { width_out = sessionData_in.width; height_out = sessionData_in.height; }
  inline void getResolution_impl (const SessionDataType&, const Stream_Module_Device_ALSAConfiguration&, unsigned int& width_out, unsigned int& height_out) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // ACE_WIN32 || ACE_WIN64
  inline struct AVRational& getFrameRate_impl (const SessionDataType&, const enum AVPixelFormat&) { ACE_ASSERT (inherited::configuration_); return inherited::configuration_->frameRate; }
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
                                             struct v4l2_format,
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
  template <typename FormatType> const struct v4l2_format& getFormat (const FormatType& format_in) { return getFormat_impl (format_in); }
  template <typename FormatType> const struct v4l2_fract& getFrameRate (const SessionDataType& sessionData_in,
                                                                        const FormatType& format_in) { return getFrameRate_impl (sessionData_in,
                                                                                                                                 format_in); };
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))

  inline const struct v4l2_format& getFormat_impl (const struct v4l2_format& format_in) { return const_cast<struct v4l2_format&> (format_in); }
  inline const struct v4l2_fract& getFrameRate_impl (const SessionDataType& sessionData_in, const enum AVPixelFormat&) { return sessionData_in.frameRate; }

  bool generateIndex (ACE_Message_Block*); // message buffer handle
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
          typename FormatType,
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
                                                 FormatType,
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
                                                 FormatType,
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
