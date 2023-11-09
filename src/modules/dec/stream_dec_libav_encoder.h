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

#ifndef STREAM_DEC_LIBAV_ENCODER_T_H
#define STREAM_DEC_LIBAV_ENCODER_T_H

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Global_Macros.h"

#include "stream_task_base_asynch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_misc_aggregator.h"

// forward declaration(s)
struct AVFrame;
struct SwsContext;
class ACE_Message_Block;
class Stream_IAllocator;

extern const char libacestream_default_dec_libav_encoder_module_name_string[];

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
          ////////////////////////////////
          typename MediaType> // audio/video !
class Stream_Decoder_LibAVEncoder_T
 : public Stream_Module_Aggregator_WriterTask_2<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_Module_Aggregator_WriterTask_2<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Decoder_LibAVEncoder_T (ISTREAM_T*); // stream handle
#else
  Stream_Decoder_LibAVEncoder_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Decoder_LibAVEncoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  struct AVCodecContext*  audioCodecContext_;
  struct AVFrame*         audioFrame_;
  unsigned int            audioFrameSize_;
  unsigned int            audioSamples_;
  struct AVStream*        audioStream_;
  struct AVFormatContext* formatContext_;
  bool                    headerWritten_;
  struct AVCodecContext*  videoCodecContext_;
  struct AVFrame*         videoFrame_;
  unsigned int            videoFrameSize_;
  unsigned int            videoSamples_;
  struct AVStream*        videoStream_;

  //ACE_Thread_Condition<ACE_Thread_Mutex> condition_;
  bool                                   isFirst_; // the first thread allocates the format context
  int                                    isLast_;  // the last thread deallocates the format context
  unsigned int                           numberOfStreamsInitialized_;

 private:
  // convenient types
  typedef Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
                                        MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVEncoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVEncoder_T (const Stream_Decoder_LibAVEncoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVEncoder_T& operator= (const Stream_Decoder_LibAVEncoder_T&))
};

// include template definition
#include "stream_dec_libav_encoder.inl"

#endif
