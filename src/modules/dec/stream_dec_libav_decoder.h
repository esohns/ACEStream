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

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "stream_dec_exports.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;
//struct AVBuffer {
//  uint8_t*     data;
//  int          size;
//  volatile int refcount;
//  void (*free)(void* opaque, uint8_t* data);
//  void*        opaque;
//  int          flags;
//};
struct AVFrame;
struct SwsContext;

extern Stream_Dec_Export const char libacestream_default_dec_libav_decoder_module_name_string[];

Stream_Dec_Export enum AVPixelFormat stream_decoder_libav_getformat_cb (struct AVCodecContext*, const enum AVPixelFormat*);
//Stream_Dec_Export void stream_decoder_libav_nopfree_cb (void*, uint8_t*);

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType>
class Stream_Decoder_LibAVDecoder_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
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
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Decoder_LibAVDecoder_T (ISTREAM_T*); // stream handle
#else
  Stream_Decoder_LibAVDecoder_T (typename inherited::ISTREAM_T*); // stream handle
#endif
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
                                        SessionDataContainerType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVDecoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVDecoder_T (const Stream_Decoder_LibAVDecoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAVDecoder_T& operator= (const Stream_Decoder_LibAVDecoder_T&))

  // helper methods
  DataMessageType* allocateMessage (typename DataMessageType::MESSAGE_T, // message type
                                    unsigned int);                       // requested size

  DataMessageType*       buffer_;
//  struct AVBuffer        buffer_;
//  struct AVBufferRef     bufferRef_;
  struct AVCodecContext* codecContext_;
  unsigned int           codecFormatHeight_; // codec output-
  unsigned int           codecFrameSize_; // codec output-
  enum AVCodecID         codecId_;
  int                    codecProfile_;
  struct AVFrame*        currentFrame_;
  struct SwsContext*     decodeContext_;
  enum AVPixelFormat     decodeFormat_; // output-
  unsigned int           decodeFrameSize_; // output-
  enum AVPixelFormat     format_; // input-

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static char            paddingBuffer[AV_INPUT_BUFFER_PADDING_SIZE];
#else
  static char            paddingBuffer[FF_INPUT_BUFFER_PADDING_SIZE];
#endif
};

// include template definition
#include "stream_dec_libav_decoder.inl"

#endif
