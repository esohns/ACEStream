#ifndef STREAM_DECODER_MPEG_TS_COMMON_H
#define STREAM_DECODER_MPEG_TS_COMMON_H

#include <cstdint>

#include "ace/config-lite.h"

// ---------------------------------------
// MPEG-2 Transport Stream
#if defined (ACE_WIN32) || defined (ACE_WIN64)
__pragma (pack (push, 1))
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_Decoder_MPEG_TS_PacketHeader
{
  /*00*/ uint8_t  synchronization;          /* 0x47 */
#if defined (ACE_LITTLE_ENDIAN)
  /*01*/ uint8_t  packet_identifier_hi:5,
                  tansport_priority:1,
                  payload_unit_start_indicator:1,
                  transport_error_indicator:1;
  /*02*/ uint8_t  packet_identifier_lo;
#else
  /*01*/ uint16_t transport_error_indicator:1,
                  payload_unit_start_indicator:1,
                  tansport_priority:1,
                  packet_identifier:13;
#endif // ACE_LITTLE_ENDIAN
#if defined (ACE_LITTLE_ENDIAN)
  /*03*/ uint8_t  continuity_counter:4,
                  adaptation_field_control:2,
                  transport_scrambling_control:2;
#else
  /*03*/ uint8_t  transport_scrambling_control:2,
                  adaptation_field_control:2,
                  continuity_counter:4;
#endif // ACE_LITTLE_ENDIAN
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_TS_AdaptationFieldControlHeader
{
  /*00*/ uint8_t adaptation_field_length;
#if defined (ACE_LITTLE_ENDIAN)
  /*01*/ uint8_t adaptation_field_extension_flag:1,
                 transport_private_data_flag:1,
                 splicing_point_flag:1,
                 opcr_flag:1,
                 pcr_flag:1,
                 elementary_stream_priority_indicator:1,
                 random_access_indicator:1,
                 discontinuity_indicator:1;
#else
  /*01*/ uint8_t discontinuity_indicator:1,
                 random_access_indicator:1,
                 elementary_stream_priority_indicator:1,
                 pcr_flag:1,
                 opcr_flag:1,
                 splicing_point_flag:1,
                 transport_private_data_flag:1,
                 adaptation_field_extension_flag:1;
#endif // ACE_LITTLE_ENDIAN
  ///*02*/ uint8_t pcr[6];                                  // optional
  ///*08*/ uint8_t opcr[6];                                 // optional
  ///*14*/ uint8_t splice_countdown;                        // optional
  ///*15*/ uint8_t transport_private_data_length;           // optional
  ///*16*/ uint8_t transport_private_data;                  // optional
  ///*xx*/ uint8_t adaptation_extension;                    // optional
  ///*xx*/ uint8_t stuffing_bytes;                          // optional, always 0xFF
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_TS_AdaptationExtensionFieldHeader
{
  /*00*/ uint8_t  adaptation_extension_length;
#if defined (ACE_LITTLE_ENDIAN)
  /*01*/ uint8_t  reserved:5,
                  seamless_splice_flag:1,
                  piecewise_rate_flag:1,
                  legal_time_window_flag:1;
#else
  /*01*/ uint8_t  legal_time_window_flag:1,
                  piecewise_rate_flag:1,
                  seamless_splice_flag:1,
                  reserved:5;
#endif // ACE_LITTLE_ENDIAN
  ///*02*/ uint16_t legal_time_window_valid_flag:1,         // optional
  //                legal_time_window_offset:15;            // optional
  ///*04*/ uint8_t  reserved:2,                             // optional
  //                piecewise_rate_byte1;                   // optional
  ///*05*/ uint8_t  piecewise_rate_byte2_3[2];              // optional
  ///*07*/ uint8_t  splice_type:4,                          // optional
  //                dts_next_access_unit_byte1;             // optional
  ///*08*/ uint8_t  dts_next_access_unit_byte2_5[4];        // optional
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_PointerHeader
{
  /*00*/ uint8_t pointer;
  ///*01*/ uint8_t pointer_filler_bytes; // n*8
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif
struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableHeader
{
  /*00*/ uint8_t  table_id;
#if defined (ACE_LITTLE_ENDIAN)
  /*01*/ uint8_t  section_length_hi:2,
                  section_length_unused_bits:2,
                  reserved_bits:2,
                  private_bit:1,
                  section_syntax_indicator:1;
  /*02*/ uint8_t  section_length_lo;
#else
  /*01*/ uint16_t section_syntax_indicator:1,
                  private_bit:1,
                  reserved_bits:2,
                  section_length_unused_bits:2,
                  section_length:10;
#endif // ACE_LITTLE_ENDIAN
  ///*03*/ uint8_t  syntax_section_table_data; // n*8
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_TableSyntaxSection
{
  /*00*/ uint16_t table_id_extension;
#if defined (ACE_LITTLE_ENDIAN)
  /*02*/ uint8_t  current_next_indicator:1,
                  version_number:5,
                  reserved_bits:2;
#else
  /*02*/ uint8_t  reserved_bits:2,
                  version_number:5,
                  current_next_indicator:1;
#endif // ACE_LITTLE_ENDIAN
  /*03*/ uint8_t  section_number;
  /*03*/ uint8_t  last_section_number;
  ///*04*/ uint8_t  table_data; // n*8
  ///*xx*/ uint8_t  crc32[4];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_Descriptor
{
  /*00*/ uint8_t descriptor_tag;
  /*02*/ uint8_t descriptor_length;
  /*03*/ uint8_t descriptor_data; // n*8
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramAssociationSpecificData
{
  /*00*/ uint16_t program_num;
#if defined (ACE_LITTLE_ENDIAN)
  /*02*/ uint8_t  program_map_id_hi:5,
                  reserved_bits:3;
  /*03*/ uint8_t  program_map_id_lo;
#else
  /*02*/ uint16_t reserved_bits:3,
                  program_map_id:13;
#endif // ACE_LITTLE_ENDIAN
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ProgramMapSpecificData
{
#if defined (ACE_LITTLE_ENDIAN)
  /*00*/ uint8_t pcr_pid_hi:5,
                 reserved_bits:3;
  /*01*/ uint8_t pcr_pid_lo;
#else
  /*00*/ uint16_t reserved_bits:3,
                  pcr_pid:13;
#endif // ACE_LITTLE_ENDIAN
#if defined (ACE_LITTLE_ENDIAN)
  /*02*/ uint8_t  program_info_length_hi:2,
                  program_info_length_unused_bits:2,
                  reserved_bits_2:4;
  /*03*/ uint8_t  program_info_length_lo;
#else
  /*02*/ uint16_t reserved_bits_2:4,
                  program_info_length_unused_bits:2,
                  program_info_length:10;
#endif // ACE_LITTLE_ENDIAN
  ///*04*/ uint8_t  program_descriptors; // n*8
  ///*xx*/ uint8_t  elementary_stream_info_data; // n*8
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
struct Stream_Decoder_MPEG_TS_ProgramSpecificInformation_ElementaryStreamSpecificData
{
  /*00*/ uint8_t  stream_type;
#if defined (ACE_LITTLE_ENDIAN)
  /*01*/ uint8_t  elementary_pid_hi:5,
                  reserved_bits:3;
  /*02*/ uint8_t  elementary_pid_lo;
#else
  /*01*/ uint16_t reserved_bits:3,
                  elementary_pid:13;
#endif // ACE_LITTLE_ENDIAN
#if defined (ACE_LITTLE_ENDIAN)
  /*04*/ uint8_t  es_info_length_hi:2,
                  es_info_length_unused_bits:2,
                  reserved_bits_2:4;
  /*05*/ uint8_t  es_info_length_lo;
#else
  /*04*/ uint16_t reserved_bits_2:4,
                  es_info_length_unused_bits:2,
                  es_info_length:10;
#endif // ACE_LITTLE_ENDIAN
  ///*06*/ uint8_t  elementary_stream_descriptors; // n*8
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamHeader
{
  /*00*/ uint16_t packet_start_code_prefix_hi;
  /*02*/ uint8_t  packet_start_code_prefix_lo;
  /*03*/ uint8_t  stream_id;
  /*04*/ uint16_t pes_packet_length;
  ///*06*/ uint8_t  optional_pes_header; // n*8
  ///*xx*/ uint8_t  stuffing_bytes;      // n*8
  ///*xx*/ uint8_t  data;                // n*8
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
struct Stream_Decoder_MPEG_TS_PacketizedElementaryStreamOptionalHeader
{
#if defined (ACE_LITTLE_ENDIAN)
  /*00*/ uint8_t  original_or_copy:1,
                  copyright:1,
                  data_alignment_indicator:1,
                  priority:1,
                  scrambling_control:2,
                  marker_bits:2;
  /*01*/ uint8_t  extension_flag:1,
                  crc_flag:1,
                  additional_copy_info_flag:1,
                  dsm_trick_mode_flag:1,
                  es_rate_flag:1,
                  escr_flag:1,
                  pts_dts_indicator:2;
#else
  /*00*/ uint8_t  marker_bits:2,
                  scrambling_control:2,
                  priority:1,
                  data_alignment_indicator:1,
                  copyright:1,
                  original_or_copy:1;
  /*01*/ uint8_t  pts_dts_indicator:2,
                  escr_flag:1,
                  es_rate_flag:1,
                  dsm_trick_mode_flag:1,
                  additional_copy_info_flag:1,
                  crc_flag:1,
                  extension_flag:1;
#endif // ACE_LITTLE_ENDIAN
  /*02*/ uint8_t  pes_header_length;
  ///*03*/ uint8_t  optional_fields; // n*8
  ///*xx*/ uint8_t  stuffing_bytes;  // n*8
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

#if defined (ACE_WIN32) || defined (ACE_WIN64)
__pragma (pack (pop))
#endif // ACE_WIN32 || ACE_WIN64

#endif
