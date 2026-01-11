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

#ifndef STREAM_DEC_MPEG_TS_DECODER_T_H
#define STREAM_DEC_MPEG_TS_DECODER_T_H

#include <map>

#include "ace/Global_Macros.h"

#include "stream_task_base_synch.h"

extern const char libacestream_default_dec_mpeg_ts_module_name_string[];

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
class Stream_Decoder_MPEG_TS_Decoder_T
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
  Stream_Decoder_MPEG_TS_Decoder_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Decoder_MPEG_TS_Decoder_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_MPEG_TS_Decoder_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_MPEG_TS_Decoder_T (const Stream_Decoder_MPEG_TS_Decoder_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Decoder_MPEG_TS_Decoder_T& operator= (const Stream_Decoder_MPEG_TS_Decoder_T&))

  // helper methods
  void parsePSI (ACE_Message_Block*, // data pointer
                 unsigned int&);     // inout: skipped bytes

  // helper types
  typedef std::map<unsigned short, unsigned short> PROGRAMNUM_TO_PMTPACKETID_T;
  typedef PROGRAMNUM_TO_PMTPACKETID_T::const_iterator PROGRAMNUM_TO_PMTPACKETID_ITERATOR_T;
  typedef std::map<unsigned short, unsigned short> STREAMTYPE_TO_PACKETID_T;
  typedef STREAMTYPE_TO_PACKETID_T::const_iterator STREAMTYPE_TO_PACKETID_ITERATOR_T;

  DataMessageType*            buffer_;
  unsigned int                missingPESBytes_;
  bool                        isParsingPSI_; // program-specific information
  unsigned int                missingPSIBytes_;
  unsigned int                programPMTPacketId_;
  PROGRAMNUM_TO_PMTPACKETID_T programs_;
  STREAMTYPE_TO_PACKETID_T    streams_;
  unsigned int                audioStreamPacketId_;
  unsigned int                videoStreamPacketId_;
};

// include template definition
#include "stream_dec_mpeg_ts_decoder.inl"

#endif
