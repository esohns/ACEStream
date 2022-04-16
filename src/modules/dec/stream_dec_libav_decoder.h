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

#ifndef STREAM_DEC_LIBAV_DECODER_T_H
#define STREAM_DEC_LIBAV_DECODER_T_H

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Global_Macros.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

// forward declaration(s)
struct SwsContext;
class ACE_Message_Block;
class Stream_IAllocator;

extern const char libacestream_default_dec_libav_decoder_module_name_string[];

enum AVPixelFormat stream_decoder_libav_getformat_cb (struct AVCodecContext*, const enum AVPixelFormat*);

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
          typename MediaType>
class Stream_Decoder_LibAVDecoder_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Decoder_LibAVDecoder_T (ISTREAM_T*); // stream handle
#else
  Stream_Decoder_LibAVDecoder_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Decoder_LibAVDecoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataContainerType,
                                        MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVDecoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVDecoder_T (const Stream_Decoder_LibAVDecoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVDecoder_T& operator= (const Stream_Decoder_LibAVDecoder_T&))

  // helper methods
  bool decodePacket (struct AVPacket&,   // data packet
                     DataMessageType*&); // return value: decoded frame
  void drainBuffers (Stream_SessionId_t); // session id

  enum AVCodecID               codecId_;
  struct AVCodecContext*       context_;
  enum AVPixelFormat           format_; // codec output-
  unsigned int                 formatHeight_; // codec output-
  unsigned int                 formatWidth_; // codec output-
  struct AVFrame*              frame_;
  unsigned int                 frameSize_; // codec output-
  enum AVPixelFormat           outputFormat_; // output-
  unsigned int                 outputFrameSize_; // output-
  struct AVCodecParserContext* parserContext_;
  int64_t                      parserPosition_;
  int                          profile_; // codec-
  struct SwsContext*           transformContext_;
};

// include template definition
#include "stream_dec_libav_decoder.inl"

#endif
