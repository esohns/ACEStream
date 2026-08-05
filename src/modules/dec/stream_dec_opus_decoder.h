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

#ifndef STREAM_DEC_OPUS_DECODER_T_H
#define STREAM_DEC_OPUS_DECODER_T_H

#include "ogg/ogg.h"
#include "opus.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_headmoduletask_base.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

extern const char libacestream_default_dec_opus_decoder_module_name_string[];

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
class Stream_Decoder_OpusDecoder_T
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
  Stream_Decoder_OpusDecoder_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_OpusDecoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_OpusDecoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_OpusDecoder_T (const Stream_Decoder_OpusDecoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_OpusDecoder_T& operator= (const Stream_Decoder_OpusDecoder_T&))

  // OGG bits
  ogg_sync_state   sync_;
  ogg_page         page_; // current-
  ogg_int64_t      packetNumber_;
  int              serialNumber_;
  ogg_stream_state stream_;
  bool             streamInitialized_;

  // Opus bits
  size_t           bufferSize_;
  int              channels_;
  OpusDecoder*     decoder_;
  bool             floatingPointOutput_; // ? : signed 16 bits
  int              frameSize_; // >= 5760 byte(s)
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType,
          ////////////////////////////////
          typename MediaType>
class Stream_Decoder_OpusDecoderH_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Decoder_OpusDecoderH_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_OpusDecoderH_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_OpusDecoderH_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_OpusDecoderH_T (const Stream_Decoder_OpusDecoderH_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_OpusDecoderH_T& operator= (const Stream_Decoder_OpusDecoderH_T&))

  // helper methods
  virtual int svc (void);

  //// OGG bits
  //ogg_sync_state   sync_;
  //ogg_page         page_; // current-
  //ogg_int64_t      packetNumber_;
  //int              serialNumber_;
  //ogg_stream_state stream_;
  //bool             streamInitialized_;

  OpusDecoder* decoder_;
};

// include template definition
#include "stream_dec_opus_decoder.inl"

#endif
