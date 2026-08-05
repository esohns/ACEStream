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

#ifndef STREAM_DEC_VORBIS_DECODER_T_H
#define STREAM_DEC_VORBIS_DECODER_T_H

#include "vorbis/codec.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_headmoduletask_base.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

extern const char libacestream_default_dec_vorbis_decoder_module_name_string[];

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
class Stream_Decoder_VorbisDecoder_T
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
  Stream_Decoder_VorbisDecoder_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_VorbisDecoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator*);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_VorbisDecoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_VorbisDecoder_T (const Stream_Decoder_VorbisDecoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_VorbisDecoder_T& operator= (const Stream_Decoder_VorbisDecoder_T&))

  bool                              isFirstInput_; // -message
  bool                              isFirstOutput_; // -message
  typename DataMessageType::DATA_T* messageData_;
  Stream_SessionId_t                sessionId_; // current-

  // OGG bits
  ogg_sync_state                    sync_;
  ogg_page                          page_; // current-
  ogg_int64_t                       packetNumber_;
  int                               serialNumber_;
  ogg_stream_state                  stream_;
  bool                              streamInitialized_;

  // Vorbis bits
  vorbis_block                      block_;
  vorbis_comment                    comment_;
  vorbis_info                       info_;
  vorbis_dsp_state                  state_;
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
class Stream_Decoder_VorbisDecoderH_T
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
  Stream_Decoder_VorbisDecoderH_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_VorbisDecoderH_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_VorbisDecoderH_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_VorbisDecoderH_T (const Stream_Decoder_VorbisDecoderH_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_VorbisDecoderH_T& operator= (const Stream_Decoder_VorbisDecoderH_T&))

  // helper methods
  virtual int svc (void);

  // OGG bits
  ogg_sync_state   sync_;
  ogg_page         page_; // current-
  ogg_int64_t      packetNumber_;
  int              serialNumber_;
  ogg_stream_state stream_;
  bool             streamInitialized_;

  // Vorbis bits
  vorbis_block     block_;
  vorbis_comment   comment_;
  vorbis_info      info_;
  vorbis_dsp_state state_;
};

// include template definition
#include "stream_dec_vorbis_decoder.inl"

#endif
