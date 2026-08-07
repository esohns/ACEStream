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

#ifndef STREAM_DEC_LIBWEBM_DEMUXER_2_T_H
#define STREAM_DEC_LIBWEBM_DEMUXER_2_T_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#undef Status
#endif // ACE_WIN32 || ACE_WIN64
#include "webm/callback.h"
#include "webm/reader.h"
#include "webm/status.h"

#include <map>
#include <string>
#include <vector>

#include "ace/Global_Macros.h"

#include "stream_task_base_synch.h"

#include "stream_lib_common.h"
#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_dec_libwebm_2_module_name_string[];

// forward declaration(s)
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_IAllocator;

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
class Stream_Decoder_LibWebM_2_Demuxer_T
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
 , public webm::Reader
 , public webm::Callback
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
  Stream_Decoder_LibWebM_2_Demuxer_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_LibWebM_2_Demuxer_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement webm::Reader
  virtual webm::Status Read (std::size_t,     // num_to_read
                             std::uint8_t*,   // buffer
                             std::uint64_t*); // num_actually_read
  virtual webm::Status Skip (std::uint64_t,   // num_to_skip
                             std::uint64_t*); // num_actually_skipped
  virtual std::uint64_t Position () const;

  // implement webm::Callback
  virtual webm::Status OnElementBegin (const webm::ElementMetadata&, // metadata
                                       webm::Action*);               // return value: action
  //virtual webm::Status OnEbml (const webm::ElementMetadata&, // metadata
  //                             webm::Reader*);               // reader
  virtual webm::Status OnTrackEntry (const webm::ElementMetadata&, // metadata
                                     const webm::TrackEntry&);     // track
  virtual webm::Status OnSimpleBlockBegin (const webm::ElementMetadata&, // metadata
                                           const webm::SimpleBlock&,     // block
                                           webm::Action*);               // return value: action
  virtual webm::Status OnFrame (const webm::FrameMetadata&, // metadata
                                webm::Reader*,              // reader
                                std::uint64_t*);            // bytes_remaining
  virtual webm::Status OnClusterEnd (const webm::ElementMetadata&, // metadata
                                     const webm::Cluster&);        // cluster
  virtual webm::Status OnSegmentEnd (const webm::ElementMetadata&); // metadata

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibWebM_2_Demuxer_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibWebM_2_Demuxer_T (const Stream_Decoder_LibWebM_2_Demuxer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_LibWebM_2_Demuxer_T& operator= (const Stream_Decoder_LibWebM_2_Demuxer_T&))

  // helper methods
  void purge ();
  size_t parseVariableLengthInteger (const std::uint8_t*, // buffer
                                     size_t,              // buffer size
                                     std::uint64_t*);     // return value: value
  size_t parseXiphLacingSize (const std::uint8_t*, // buffer
                              size_t,              // buffer size
                              std::uint32_t*);     // return falue: packet size

  // override some ACE_Task_T methods
  virtual int svc (void);

  std::vector<uint8_t>                                buffer_;
  ACE_Condition_Thread_Mutex                          condition_;
  bool                                                finished_;
  size_t                                              maxBufferCapacity_;
  std::uint64_t                                       readPosition_; // current-
  std::uint64_t                                       totalPosition_; // global-

  webm::Id                                            currentElementId_;
  std::uint64_t                                       currentElementSize_;
  std::uint64_t                                       lastTrackNumber_;
  enum Stream_MediaType_Type                          lastTrackType_;
  std::string                                         lastCodecId_;
  std::vector<std::uint8_t>                           lastCodecPrivateData_;

  std::map<std::uint64_t, enum Stream_MediaType_Type> trackNumberToMessageMediaType_;
  std::vector<std::uint64_t>                          trackNumbersToSkip_;

  std::uint64_t                                       audioTrackNumber_;
  std::uint64_t                                       videoTrackNumber_;
};

// include template definition
#include "stream_dec_libwebm_demuxer_2.inl"

#endif
