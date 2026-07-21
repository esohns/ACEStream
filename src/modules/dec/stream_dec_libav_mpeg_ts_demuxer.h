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

#ifndef STREAM_DEC_LIBAV_MPEG_TS_DEMUXER_T_H
#define STREAM_DEC_LIBAV_MPEG_TS_DEMUXER_T_H

#include <map>

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
}
#endif /* __cplusplus */

#include "ace/Global_Macros.h"

#include "stream_task_base_synch.h"

#include "stream_lib_common.h"
#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_dec_libav_mpeg_ts_module_name_string[];

// forward declaration(s)
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_IAllocator;

//////////////////////////////////////////

struct ACEStream_LibAV_MPEG_TS_Demuxer_CBData
{
  ACEStream_LibAV_MPEG_TS_Demuxer_CBData ()
   : buffer (NULL)
   , queue (NULL)
  {}

  ACE_Message_Block*      buffer;
  ACE_Message_Queue_Base* queue;
};

int
acestream_libav_mpeg_ts_demuxer_read_cb (void*,
                                         uint8_t*,
                                         int);

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
          typename MediaType>
class Stream_Decoder_LibAV_MPEG_TS_Demuxer_T
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
  Stream_Decoder_LibAV_MPEG_TS_Demuxer_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_LibAV_MPEG_TS_Demuxer_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAV_MPEG_TS_Demuxer_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAV_MPEG_TS_Demuxer_T (const Stream_Decoder_LibAV_MPEG_TS_Demuxer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibAV_MPEG_TS_Demuxer_T& operator= (const Stream_Decoder_LibAV_MPEG_TS_Demuxer_T&))

  // helper types
  typedef Stream_MessageQueueBase_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t> MESSAGE_QUEUE_T;

  // helper methods
  void stop ();

  // override some ACE_Task_T methods
  virtual int svc (void);

  struct AVFormatContext*                       formatContext_;

  uint8_t*                                      IOBuffer_;
  struct ACEStream_LibAV_MPEG_TS_Demuxer_CBData IOCBData_;
  struct AVIOContext*                           IOContext_;

  MESSAGE_QUEUE_T                               queue_;

  std::map<int, enum Stream_MediaType_Type>     streamIndexToMessageMediaType_;

  unsigned int                                  audioStreamPacketId_;
  unsigned int                                  videoStreamPacketId_;
};

// include template definition
#include "stream_dec_libav_mpeg_ts_demuxer.inl"

#endif
