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

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_MPEG_TS_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::Stream_Decoder_MPEG_TS_Decoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , buffer_ (NULL)
 , missingPESBytes_ (0)
 , isParsingPSI_ (false)
 , missingPSIBytes_ (0)
 , programPMTPacketId_ (0)
 , program_ (0)
 , programs_ ()
 , streamType_ (0)
 , streams_ ()
 , streamPacketId_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_TS_Decoder_T::Stream_Decoder_MPEG_TS_Decoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_MPEG_TS_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::~Stream_Decoder_MPEG_TS_Decoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_TS_Decoder_T::~Stream_Decoder_MPEG_TS_Decoder_T"));

  if (buffer_)
    buffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_MPEG_TS_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                        Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_TS_Decoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF
    missingPESBytes_ = 0;
    isParsingPSI_ = false;
    missingPSIBytes_ = 0;
    program_ = 0;
    programs_.clear ();
    streamType_ = 0;
    streams_.clear ();
    streamPacketId_ = 0;
  } // end IF

  // *TODO*: remove type inferences
  program_ = configuration_in.program;
  streamType_ = configuration_in.streamType;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_MPEG_TS_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_TS_Decoder_T::handleDataMessage"));

  int result = -1;
  struct Stream_Decoder_MPEG_TS_PacketHeader* packet_header_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  if (!buffer_)
  {
    buffer_ = message_inout;
    message_block_p = buffer_;
  } // end IF
  else
  {
    message_block_p = buffer_;
    while (message_block_p->cont ())
      message_block_p = message_block_p->cont ();
    message_block_p->cont (message_inout);
    message_block_p = message_inout;
  } // end ELSE
  ACE_ASSERT (message_block_p);

  // message_block_p points at the trailing fragment

  if (missingPSIBytes_)
  {
    if (message_inout->length () < missingPSIBytes_)
    {
      missingPSIBytes_ -= message_inout->length ();
      return;
    } // end IF
    missingPSIBytes_ = 0;
  } // end IF

  unsigned int total_length = buffer_->total_length ();
  if (total_length < STREAM_DECODER_MPEG_TS_PACKET_SIZE)
    return; // done

  // *TODO*: this step is unnecessary --> implement a proper parser
  typename DataMessageType::IDATA_MESSAGE_T* idata_message_p =
    dynamic_cast<typename DataMessageType::IDATA_MESSAGE_T*> (buffer_);
  ACE_ASSERT (idata_message_p);
  try {
    idata_message_p->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));

    // clean up
    message_inout->release ();
    message_inout = NULL;
    buffer_->release ();

    return;
  } // end IF

  struct Stream_Decoder_MPEG_TS_AdapatationFieldControlHeader* adaptation_field_control_p =
    NULL;
  unsigned short packet_identifier = 0;
  unsigned int skipped_bytes = 0;
  struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamHeader* pes_header_p =
    NULL;
  unsigned short pes_packet_length = 0;
  struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamOptionalHeader* optional_pes_header_p =
    NULL;
  do
  { // done ?
    if (buffer_->length () < STREAM_DECODER_MPEG_TS_PACKET_SIZE)
      break;

    packet_header_p =
      reinterpret_cast<struct Stream_Decoder_MPEG_TS_PacketHeader*> (buffer_->rd_ptr ());
    buffer_->rd_ptr (sizeof (struct Stream_Decoder_MPEG_TS_PacketHeader));
    skipped_bytes = sizeof (struct Stream_Decoder_MPEG_TS_PacketHeader);

    // skip over adaptation field(s), if any
    if (packet_header_p->adaptation_field_control & 2)
    {
      adaptation_field_control_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_TS_AdapatationFieldControlHeader*> (buffer_->rd_ptr ());
      buffer_->rd_ptr (adaptation_field_control_p->adaptation_field_length + 1);
      skipped_bytes += (adaptation_field_control_p->adaptation_field_length + 1);
    } // end IF

    // sanity check(s)
    //ACE_ASSERT (packet_header_p->payload_unit_start_indicator);
    ACE_ASSERT (packet_header_p->synchronization == STREAM_DECODER_MPEG_TS_SYNCHRONIZATION_BYTE);

    packet_identifier =
      (packet_header_p->packet_identifier_hi << 8 | packet_header_p->packet_identifier_lo);
    if ((packet_identifier == STREAM_DECODER_MPEG_TS_PACKET_ID_PAT) ||
        (!streamPacketId_      &&
          programPMTPacketId_  &&
         (packet_identifier == programPMTPacketId_)))
    {
      parsePSI (buffer_);
      goto continue_;
    } // end IF
    if (!streamPacketId_ ||
        (packet_identifier != streamPacketId_))
      goto continue_;

    // extract PES packet

    if (packet_header_p->payload_unit_start_indicator == 1)
    {
      pes_header_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamHeader*> (buffer_->rd_ptr ());
      buffer_->rd_ptr (sizeof (struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamHeader));
      skipped_bytes +=
        sizeof (struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamHeader);

      // *NOTE*: MPEG TS video-stream PES packets may not have a defined packet
      //         length and are encapsulated in TS packets
      missingPESBytes_ = pes_header_p->pes_packet_length;
      if (ACE_LITTLE_ENDIAN)
        missingPESBytes_ = ACE_SWAP_WORD (missingPESBytes_);

      if ((pes_header_p->stream_id != STREAM_DECODER_MPEG_TS_STREAM_TYPE_PADDING) &&
          (pes_header_p->stream_id != STREAM_DECODER_MPEG_TS_STREAM_TYPE_PRIVATE_2_NAVIGATION_DATA))
      {
        optional_pes_header_p =
          reinterpret_cast<struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamOptionalHeader*> (buffer_->rd_ptr ());
        buffer_->rd_ptr (sizeof (struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamOptionalHeader));
        skipped_bytes +=
          sizeof (struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamOptionalHeader);
        buffer_->rd_ptr (optional_pes_header_p->pes_header_length);
        skipped_bytes += optional_pes_header_p->pes_header_length;
      } // end IF
    } // end IF
    else if (missingPESBytes_)
      missingPESBytes_ -= (STREAM_DECODER_MPEG_TS_PACKET_SIZE - skipped_bytes);

    // part of the specified program --> forward data
    message_block_p = buffer_->duplicate ();
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(), returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    message_block_p->wr_ptr (buffer_->rd_ptr () +
                             (STREAM_DECODER_MPEG_TS_PACKET_SIZE - skipped_bytes));

    message_block_2 = message_block_p->cont ();
    if (message_block_2)
    {
      message_block_2->release ();
      message_block_p->cont (NULL);
    } // end IF

    result = inherited::put_next (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));

      // clean up
      message_block_p->release ();

      return;
    } // end IF

continue_:
    buffer_->rd_ptr (STREAM_DECODER_MPEG_TS_PACKET_SIZE - skipped_bytes);
  } while (true);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_MPEG_TS_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_TS_Decoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  const SessionDataContainerType& session_data_container_r =
    message_inout->get ();
  typename SessionDataContainerType::DATA_T& session_data_r =
    const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      goto continue_;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        buffer_->release ();
        buffer_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_MPEG_TS_Decoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType>::parsePSI (ACE_Message_Block* messageBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_MPEG_TS_Decoder_T::parsePSI"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  unsigned int table_syntax_section_offset = 0;
  struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_PointerHeader* pointer_p =
    reinterpret_cast<struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_PointerHeader*> (messageBlock_in->rd_ptr ());
  table_syntax_section_offset =
    sizeof (struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_PointerHeader);
  table_syntax_section_offset += pointer_p->pointer;

  struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableHeader* table_header_p =
    reinterpret_cast<struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableHeader*> (messageBlock_in->rd_ptr () + table_syntax_section_offset);
  table_syntax_section_offset +=
    sizeof (struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableHeader);

  unsigned short section_length =
    ((table_header_p->section_length_hi << 8) | table_header_p->section_length_lo);
  if ((messageBlock_in->total_length () - table_syntax_section_offset) <
      section_length)
  {
    missingPSIBytes_ =
      (section_length -
       (messageBlock_in->total_length () - table_syntax_section_offset));
    isParsingPSI_ = true; // <-- accumulate missing PAT fragments
    return;
  } // end IF

  unsigned int offset = table_syntax_section_offset;
//  struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableSyntaxSection* table_syntax_section_p =
//    reinterpret_cast<struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableSyntaxSection*> (messageBlock_in->rd_ptr () + offset);
  offset +=
    sizeof (struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableSyntaxSection);
  if (table_header_p->table_id == STREAM_DECODER_MPEG_TS_TABLE_ID_PAT)
  {
    struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramAssociationSpecificData* PAT_data_p =
      NULL;

    unsigned short program_number = 0;
    unsigned short program_map_id = 0;
    do
    {
      PAT_data_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramAssociationSpecificData*> (messageBlock_in->rd_ptr () + offset);
      offset +=
        sizeof (struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramAssociationSpecificData);
      if ((offset - table_syntax_section_offset) >=
          section_length) break;

      // cache for re-use
      program_number = PAT_data_p->program_num;
      if (ACE_LITTLE_ENDIAN) program_number = ACE_SWAP_WORD (program_number);
      program_map_id =
        (PAT_data_p->program_map_id_hi << 8 | PAT_data_p->program_map_id_lo);
      programs_.insert (std::make_pair (program_number,
                                        program_map_id));

      if (program_number == program_)
        programPMTPacketId_ = program_map_id;
    } while (true);
  } // end IF
  else if (table_header_p->table_id == STREAM_DECODER_MPEG_TS_TABLE_ID_PMT)
  {
    struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramMapSpecificData* PMT_data_p =
      reinterpret_cast<struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramMapSpecificData*> (messageBlock_in->rd_ptr () + offset);
    offset +=
      sizeof (struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramMapSpecificData);
    unsigned short program_info_length =
      (PMT_data_p->program_info_length_hi << 8 | PMT_data_p->program_info_length_lo);
    offset += program_info_length;

    struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ElementaryStreamSpecificData* ES_data_p =
      NULL;

    unsigned short elementary_stream_pid = 0;
    do
    {
      if ((offset - table_syntax_section_offset) >=
          section_length) break;

      ES_data_p =
        reinterpret_cast<struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ElementaryStreamSpecificData*> (messageBlock_in->rd_ptr () + offset);
      offset +=
        sizeof (struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ElementaryStreamSpecificData);

      // cache for re-use
      elementary_stream_pid =
        (ES_data_p->elementary_pid_hi << 8 | ES_data_p->elementary_pid_lo);
      streams_.insert (std::make_pair (ES_data_p->stream_type,
                                       elementary_stream_pid));

      if (ES_data_p->stream_type == streamType_)
        streamPacketId_ = elementary_stream_pid;
    } while (true);
  } // end ELSE IF
  else if (table_header_p->table_id == 0xFF) {}
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown PAT table id (was: %d), returning\n"),
                inherited::mod_->name (),
                table_header_p->table_id));
    return;
  } // end ELSE
}
