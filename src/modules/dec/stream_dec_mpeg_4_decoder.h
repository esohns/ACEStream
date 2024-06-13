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

#ifndef STREAM_DEC_MPEG_4_DECODER_T_H
#define STREAM_DEC_MPEG_4_DECODER_T_H

#include <utility>
#include <vector>

#include "ace/Global_Macros.h"

#include "stream_task_base_synch.h"

extern const char libacestream_default_dec_mpeg_4_module_name_string[];

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

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
class Stream_Decoder_MPEG_4_Decoder_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
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
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Stream_Decoder_MPEG_4_Decoder_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_MPEG_4_Decoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_MPEG_4_Decoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_MPEG_4_Decoder_T (const Stream_Decoder_MPEG_4_Decoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_MPEG_4_Decoder_T& operator= (const Stream_Decoder_MPEG_4_Decoder_T&))

  // helper method(s)
  ACE_UINT64 processBox (const struct Stream_Decoder_MPEG_4_BoxHeader&, // header
                         bool&);                                        // return value: need more data ?
  ACE_UINT64 processFrame (bool&); // return value: need more data ?
  ACE_UINT32 frameToChunk (ACE_UINT32,   // frame#
                           ACE_UINT32&); // return value: first frame num in same chunk
  void dispatchQueuedSessionMessages ();

  typedef std::vector<std::pair<ACE_UINT32, ACE_UINT64> > BOXES_T;
  typedef BOXES_T::const_iterator BOXESITERATOR_T;
  typedef std::vector<std::pair<ACE_UINT32, ACE_UINT32> > FRAMETOCHUNK_T; // [first, last]
  typedef FRAMETOCHUNK_T::const_iterator FRAMETOCHUNKITERATOR_T;
  typedef std::vector<ACE_UINT64> CHUNKOFFSETS_T;
  typedef CHUNKOFFSETS_T::const_iterator CHUNKOFFSETSITERATOR_T;
  typedef std::vector<ACE_UINT32> FRAMESIZES_T;
  typedef FRAMESIZES_T::const_iterator FRAMESIZESITERATOR_T;

  BOXES_T                  boxes_;
  ACE_UINT64               boxSize_; // current-
  DataMessageType*         buffer_;
  CHUNKOFFSETS_T           chunkOffsets_; // stco/co64
  FRAMESIZES_T             frameSizes_; // stsz
  FRAMETOCHUNK_T           frameToChunk_; // stsc
  ACE_UINT64               missingBytes_;
  ACE_UINT64               offset_;
  bool                     processingFrames_;
  bool                     queueSessionMessages_;
  bool                     trackIsVideo_; // current-
  ACE_UINT32               videoFrame_; // current-
  ACE_UINT32               videoFrames_;
};

// include template definition
#include "stream_dec_mpeg_4_decoder.inl"

#endif
