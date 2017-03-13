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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfobjects.h>
#include <strmif.h>
#endif

#include <ace/Global_Macros.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#ifdef __cplusplus
extern "C"
{
#include <libavformat/avformat.h>
}
//#include <sndfile.h>
#include <sox.h>
#endif /* __cplusplus */
#endif /* ACE_WIN32 || ACE_WIN64 */

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "stream_dec_common.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
int
stream_decoder_aviencoder_libav_write_cb (void*,    // act
                                          uint8_t*, // buffer address
                                          int);     // buffer size
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
class Stream_Decoder_AVIEncoder_ReaderTask_T
 : public ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType>
{
 public:
  Stream_Decoder_AVIEncoder_ReaderTask_T ();
  virtual ~Stream_Decoder_AVIEncoder_ReaderTask_T ();

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> inherited;

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
 public:
  Stream_Decoder_AVIEncoder_WriterTask_T ();
  virtual ~Stream_Decoder_AVIEncoder_WriterTask_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

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
  bool                    isFirst_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct AVFormatContext* formatContext_;
#endif

  // helper methods
  DataMessageType* allocateMessage (unsigned int); // requested size
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: return values needs to be Stream_Module_Device_DirectShow_Tools::deleteMediaType()d !
  template <typename FormatType2> AM_MEDIA_TYPE& getFormat (const FormatType2* format_in) { return getFormat_impl (format_in); };
#else
  template <typename FormatType2> struct v4l2_format& getFormat (const FormatType2* format_in) { return getFormat_impl (format_in); };
  template <typename FormatType2> struct v4l2_fract& getFrameRate (const SessionDataType& sessionData_in,
                                                                   const FormatType2* format_in) { return getFrameRate_impl (sessionData_in,
                                                                                                                             format_in); };
#endif
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_ControlType,
                                 Stream_SessionMessageType,
                                 UserDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: return values needs to be Stream_Module_Device_DirectShow_Tools::deleteMediaType()d !
  struct _AMMediaType& getFormat_impl (const struct _AMMediaType*);
  struct _AMMediaType& getFormat_impl (const IMFMediaType*);
#else
  inline struct v4l2_format& getFormat_impl (const struct Stream_Module_Device_ALSAConfiguration*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (v4l2_format ()); ACE_NOTREACHED (return v4l2_format ();) };
//  inline struct v4l2_format& getFormat_impl (const struct v4l2_format* format_in) { ACE_ASSERT (format_in); return (*format_in); }
  inline struct v4l2_fract& getFrameRate_impl (const SessionDataType&,
                                               const Stream_Module_Device_ALSAConfiguration*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (v4l2_fract ()); ACE_NOTREACHED (return v4l2_fract ();) };
//  inline struct v4l2_fract& getFrameRate_impl (const SessionDataType& sessionData_in,
//                                               const struct v4l2_format&) { return sessionData_in.v4l2FrameRate; };
#endif

  bool generateIndex (ACE_Message_Block*); // message buffer handle

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
 public:
  Stream_Decoder_AVIEncoder_WriterTask_T ();
  virtual ~Stream_Decoder_AVIEncoder_WriterTask_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

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
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_ControlType,
                                 Stream_SessionMessageType,
                                 UserDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T (const Stream_Decoder_AVIEncoder_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_AVIEncoder_WriterTask_T& operator= (const Stream_Decoder_AVIEncoder_WriterTask_T&))

  inline const struct v4l2_format& getFormat_impl (const struct v4l2_format& format_in) { return const_cast<struct v4l2_format&> (format_in); } // return value: media type handle
  inline const struct v4l2_fract& getFrameRate_impl (const SessionDataType& sessionData_in,                               // session data
                                                     const struct v4l2_format&) { return sessionData_in.v4l2FrameRate; }; // return value: frame rate handle

  bool generateIndex (ACE_Message_Block*); // message buffer handle
};
#endif

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
 public:
  Stream_Decoder_WAVEncoder_T ();
  virtual ~Stream_Decoder_WAVEncoder_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
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

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T (const Stream_Decoder_WAVEncoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_WAVEncoder_T& operator= (const Stream_Decoder_WAVEncoder_T&))

  // helper methods
  virtual bool generateHeader (ACE_Message_Block*); // message buffer handle

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
//  struct SF_INFO      SFInfo_;
//  struct SNDFILE_tag* SNDFile_;
  struct sox_encodinginfo_t encodingInfo_;
  struct sox_signalinfo_t   signalInfo_;

  struct sox_format_t*      outputFile_;
#endif
};

// include template definition
#include "stream_dec_avi_encoder.inl"

#endif
