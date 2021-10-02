#ifndef STREAM_DECODER_MPEG_4_COMMON_H
#define STREAM_DECODER_MPEG_4_COMMON_H

#include <cstdint>

#include "ace/Basic_Types.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
__pragma (pack (push, 1))
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_Decoder_MPEG_4_BoxHeader
{
  /*00*/ uint32_t length;
  /*04*/ uint32_t type;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_LargeBoxHeader
{
  /*00*/ uint32_t length; // 1
  /*04*/ uint32_t type;
  /*08*/ uint64_t largesize;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_FullBoxHeader
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*08*/ uint8_t version;
  /*09*/ uint8_t flags[3];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_FileTypeBox // ftyp
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*08*/ uint32_t major_brand;
  /*12*/ uint32_t minor_version;
  /*16*/ uint32_t compatible_brands[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_MovieHeaderBox0 // mvhd version == 0
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t creation_time;
  /*16*/ uint32_t modification_time;
  /*20*/ uint32_t timescale;
  /*24*/ uint32_t duration;
  /*28*/ uint32_t rate;
  /*32*/ uint16_t volume;
  /*34*/ uint16_t reserved;
  /*36*/ uint32_t reserved_2[2];
  /*44*/ uint32_t matrix[9];
  /*80*/ uint32_t pre_defined[6];
  /*104*/ uint32_t next_track_ID;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_MovieHeaderBox1 // mvhd version == 1
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint64_t creation_time;
  /*20*/ uint64_t modification_time;
  /*28*/ uint32_t timescale;
  /*32*/ uint64_t duration;
  /*40*/ uint32_t rate;
  /*44*/ uint16_t volume;
  /*46*/ uint16_t reserved;
  /*48*/ uint32_t reserved_2[2];
  /*56*/ uint32_t matrix[9];
  /*92*/ uint32_t pre_defined[6];
  /*116*/ uint32_t next_track_ID;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_BaseDescriptor
{
  /*00*/ uint8_t tag;
  /*01*/ uint8_t size_of_instance[4];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_InitialObjectDescriptor
 : Stream_Decoder_MPEG_4_BaseDescriptor
{
  /*05*/ uint16_t od_id;
  /*07*/ uint8_t od_profile_level;
  /*08*/ uint8_t scene_profile_level;
  /*09*/ uint8_t audio_profile_level;
  /*10*/ uint8_t video_profile_level;
  /*11*/ uint8_t graphics_profile_level;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_InitialObjectDescriptorBox
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ struct Stream_Decoder_MPEG_4_InitialObjectDescriptor io_descriptor;
  /*24*/ struct Stream_Decoder_MPEG_4_BaseDescriptor es_id_included_descriptor;
  /*29*/ uint32_t track_ID;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_TrackHeaderBox0 // tkhd version == 0
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t creation_time;
  /*16*/ uint32_t modification_time;
  /*20*/ uint32_t track_ID;
  /*24*/ uint32_t reserved;
  /*28*/ uint32_t duration;
  /*32*/ uint32_t reserved2[2];
  /*40*/ uint16_t layer;
  /*42*/ uint16_t alternate_group;
  /*44*/ uint16_t volume;
  /*46*/ uint16_t reserved3;
  /*48*/ uint32_t matrix[9];
  /*84*/ uint32_t width;
  /*88*/ uint32_t height;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_TrackHeaderBox1 // tkhd version == 1
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint64_t creation_time;
  /*20*/ uint64_t modification_time;
  /*28*/ uint32_t track_ID;
  /*32*/ uint32_t reserved;
  /*36*/ uint64_t duration;
  /*44*/ uint32_t reserved2[2];
  /*48*/ uint16_t layer;
  /*50*/ uint16_t alternate_group;
  /*52*/ uint16_t volume;
  /*54*/ uint16_t reserved3;
  /*56*/ uint32_t matrix[9];
  /*92*/ uint32_t width;
  /*96*/ uint32_t height;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_EditListEntry0
{
  /*12*/ uint32_t segment_duration;
  /*16*/ int32_t media_time;
  /*20*/ uint16_t media_rate_integer;
  /*22*/ uint16_t media_rate_fraction;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_EditListEntry1
{
  /*12*/ uint64_t segment_duration;
  /*20*/ uint64_t media_time;
  /*28*/ uint16_t media_rate_integer;
  /*30*/ uint16_t media_rate_fraction;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_EditListBox0 // elst version == 0
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_EditListEntry0 entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_EditListBox1 // elst version == 1
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_EditListEntry1 entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_MediaHeaderBox0 // mdhd version == 0
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t creation_time;
  /*16*/ uint32_t modification_time;
  /*20*/ uint32_t timescale;
  /*24*/ uint32_t duration;
#if defined (ACE_LITTLE_ENDIAN)
  /*28*/ uint8_t  language2_hi:2,
                  language1:5,
                  pad:1;
  /*29*/ uint8_t  language3:5,
                  language2_lo:3;
#else
  /*28*/ uint16_t pad:1,
                  language1:5,
                  language2:5,
                  language3:5;
#endif // ACE_LITTLE_ENDIAN
  /*30*/ uint16_t pre_defined;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_MediaHeaderBox1 // mdhd version == 1
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint64_t creation_time;
  /*20*/ uint64_t modification_time;
  /*28*/ uint32_t timescale;
  /*32*/ uint64_t duration;
#if defined (ACE_LITTLE_ENDIAN)
  /*40*/ uint8_t  language2_hi:2,
                  language1:5,
                  pad:1;
  /*41*/ uint8_t  language3:5,
                  language2_lo:3;
#else
  /*40*/ uint16_t pad:1,
                  language1:5,
                  language2:5,
                  language3:5;
#endif // ACE_LITTLE_ENDIAN
  /*42*/ uint16_t pre_defined;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_HandlerReferenceBox // hdlr
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t pre_defined;
  /*16*/ uint32_t handler_type;
  /*20*/ uint32_t reserved[3];
  /*32*/ uint8_t name[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_VideoMediaHeaderBox // vmhd
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint16_t graphics_mode;
  /*14*/ uint16_t opcolor[3];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SoundMediaHeaderBox // smhd
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ int16_t balance;
  /*14*/ uint16_t reserved;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_DataReferenceEntryUrlBox
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint8_t location[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_DataReferenceEntryUrnBox
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint8_t name[];
  ///*xx*/ uint8_t location[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_DataReferenceBox // dref
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_FullBoxHeader entries[]; // *WARNING*: only the first offset is correct !
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBoxBase
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*08*/ uint8_t reserved[6];
  /*14*/ uint16_t data_reference_index;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_VideoSampleDescriptionEntryBoxBase
 : Stream_Decoder_MPEG_4_SampleDescriptionEntryBoxBase
{
  /*16*/ uint16_t pre_defined;
  /*18*/ uint16_t reserved;
  /*20*/ uint32_t pre_defined2[3];
  /*32*/ uint16_t width;
  /*34*/ uint16_t height;
  /*36*/ uint32_t horizontal_resolution;
  /*40*/ uint32_t vertical_resolution;
  /*44*/ uint32_t reserved2;
  /*48*/ uint16_t frame_count;
  /*50*/ uint8_t compressor_name[32];
  /*82*/ uint16_t depth;
  /*84*/ uint16_t pre_defined3; // *NOTE*: ought to be -1, indicating that there is no (inline-)color_table
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_AudioSampleDescriptionEntryBoxBase
 : Stream_Decoder_MPEG_4_SampleDescriptionEntryBoxBase
{
  /*16*/ uint32_t reserved[2];
  /*24*/ uint16_t channel_count;
  /*26*/ uint16_t sample_size;
  /*28*/ uint16_t pre_defined;
  /*30*/ uint16_t reserved2;
  /*32*/ uint32_t sample_rate;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry
{
  /*92*/ uint16_t sequence_parameter_set_length;
  /*94*/ uint8_t sequence_parameter_set_nal_unit[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_AVCCBox_Part1 // avcC
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*86*/ uint8_t configuration_version;
  /*87*/ uint8_t avc_profile_indication;
  /*88*/ uint8_t profile_compatibility;
  /*89*/ uint8_t avc_level_indication;
#if defined (ACE_LITTLE_ENDIAN)
  /*90*/ uint8_t length_size_minus_one:2,
                 reserved:6;
#else
  /*90*/ uint8_t reserved:6,
                 length_size_minus_one:2;
#endif // ACE_LITTLE_ENDIAN
#if defined (ACE_LITTLE_ENDIAN)
  /*91*/ uint8_t num_of_sequence_parameter_sets:5,
                 reserved2:3;
#else
  /*91*/ uint8_t reserved2:3,
                 num_of_sequence_parameter_sets:5;
#endif // ACE_LITTLE_ENDIAN
  /*92*/ uint8_t entries[]; // struct Stream_Decoder_MPEG_4_AVCCBoxSequenceParameterSetEntry
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry
{
  /*92*/ uint16_t picture_parameter_set_length;
  /*94*/ uint8_t picture_parameter_set_nal_unit[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_AVCCBox_Part2 // avcC
{
  /*xx*/ uint8_t num_of_picture_parameter_sets;
  /*xy*/ uint8_t entries[]; // struct Stream_Decoder_MPEG_4_AVCCBoxPictureParameterSetEntry
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_ColorInformationBoxBase // colr
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*08*/ uint32_t colour_type;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_ColorInformationBoxNCLX // nclx
 : Stream_Decoder_MPEG_4_ColorInformationBoxBase
{
  /*12*/ uint16_t colour_primaries;
  /*16*/ uint16_t transfer_characteristics;
  /*20*/ uint16_t matrix_coefficients;
#if defined (ACE_LITTLE_ENDIAN)
  /*22*/ uint8_t  reserved:7,
                  full_range_flag:1;
#else
  /*22*/ uint8_t  full_range_flag:1,
                  reserved:7;
#endif // ACE_LITTLE_ENDIAN
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_PixelAspectRatioBox // pasp
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*08*/ uint32_t hSpacing;
  /*12*/ uint32_t vSpacing;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAVCBox // avc1
 : Stream_Decoder_MPEG_4_VideoSampleDescriptionEntryBoxBase
{
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase
 : Stream_Decoder_MPEG_4_BaseDescriptor
{
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_DecoderConfigurationDescriptor
 : Stream_Decoder_MPEG_4_BaseDescriptor
{
  /*01*/ uint8_t object_type_indication;
#if defined (ACE_LITTLE_ENDIAN)
  /*02*/ uint8_t reserved:1,
                 up_stream:1,
                 stream_type:6;
#else
  /*02*/ uint8_t stream_type:6,
                 up_stream:1,
                 reserved:1;
#endif // ACE_LITTLE_ENDIAN
  /*03*/ uint8_t buffer_size_db[3];
  /*06*/ uint32_t max_bitrate;
  /*10*/ uint32_t avg_bitrate;
  /*14*/ struct Stream_Decoder_MPEG_4_DecoderSpecificInformationDescriptorBase decoder_specific_information[]; // *WARNING*: only the first offset is correct !
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_ElementaryStreamDescriptor
 : Stream_Decoder_MPEG_4_BaseDescriptor
{
  /*01*/ uint16_t es_id;
#if defined (ACE_LITTLE_ENDIAN)
  /*03*/ uint8_t  stream_priority:5,
                  ocr_stream_flag:1,
                  url_flag:1,
                  stream_dependence_flag:1;
#else
  /*03*/ uint8_t  stream_dependence_flag:1,
                  url_flag:1,
                  ocr_stream_flag:1,
                  stream_priority:5;
#endif // ACE_LITTLE_ENDIAN
  ///*04*/ uint16_t depends_on_es_id; // ? stream_dependence_flag
  ///*06*/ uint8_t url_length; // ? url_flag
  ///*07*/ uint8_t url_string[]; // ? url_flag
  ///*xx*/ uint16_t ocr_es_id; // ? ocr_stream_flag
  /*04*/ struct Stream_Decoder_MPEG_4_DecoderConfigurationDescriptor decoder_configuration_descriptor;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_ElementarySampleDescriptionBox // esds
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ struct Stream_Decoder_MPEG_4_ElementaryStreamDescriptor es_descriptor;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleDescriptionEntryAACBox // mp4a
 : Stream_Decoder_MPEG_4_AudioSampleDescriptionEntryBoxBase
{
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBitRateBox // btrt
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*12*/ uint32_t bufferSizeDB;
  /*16*/ uint32_t maxBitrate;
  /*20*/ uint32_t avgBitrate;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleDescriptionBox // stsd
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_SampleDescriptionEntryBoxBase entries[]; // *WARNING*: only the first offset is correct !
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry
{
  /*16*/ uint32_t sample_count;
  /*20*/ uint32_t sample_delta;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_TimeToSampleBox // stts
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_TimeToSampleBoxEntry entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0
{
  /*16*/ uint32_t sample_count;
  /*20*/ uint32_t sample_offset;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_CompositionOffsetBox0 // ctts
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry0 entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1
{
  /*16*/ uint32_t sample_count;
  /*20*/ int32_t sample_offset;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_CompositionOffsetBox1 // ctts
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_CompositionOffsetBoxEntry1 entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SyncSampleBox // stss
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ uint32_t entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleDependencyType
{
#if defined (ACE_LITTLE_ENDIAN)
  /*xx*/ uint8_t sample_has_redundancy:2,
                 sample_is_depended_on:2,
                 sample_depends_on:2,
                 is_leading:2;
#else
  /*xx*/ uint8_t is_leading:2,
                 sample_depends_on:2,
                 sample_is_depended_on:2,
                 sample_has_redundancy:2;
#endif // ACE_LITTLE_ENDIAN
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleDependencyTypeBox // sdtp
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ struct Stream_Decoder_MPEG_4_SampleDependencyType entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleToChunkBoxEntry
{
  /*16*/ uint32_t first_chunk;
  /*20*/ uint32_t samples_per_chunk;
  /*24*/ uint32_t samples_description_index;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleToChunkBox // stsc
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ struct Stream_Decoder_MPEG_4_SampleToChunkBoxEntry entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleSizeBox // stsz
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t sample_size;
  /*16*/ uint32_t sample_count;
  /*20*/ uint32_t entry_size[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_ChunkOffsetBox // stco
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t entry_count;
  /*16*/ uint32_t chunk_offset[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase
{
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryRoll // roll
 : Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase
{
  /*00*/ int16_t roll_distance;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10 // version1, length0
{
  /*24*/ uint32_t description_length;
  /*28*/ struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase sample_group_entry;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox // sgpd
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t grouping_type;
  /*16*/ uint32_t default_length_or_sample_description_index;
  /*20*/ uint32_t entry_count;
  /*24*/ struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntryBase entries[]; // *WARNING*: only the first offset is correct !
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBox10 // sgpd version1, length0
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t grouping_type;
  /*16*/ uint32_t default_length_or_sample_description_index;
  /*20*/ uint32_t entry_count;
  /*24*/ struct Stream_Decoder_MPEG_4_SampleGroupDescriptionBoxEntry10 entries[]; // *WARNING*: only the first offset is correct !
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleToGroupBoxEntry
{
  /*20*/ uint32_t sample_count;
  /*24*/ uint32_t group_description_index;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleToGroupBox // sbgp
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t grouping_type;
  /*16*/ uint32_t entry_count;
  /*20*/ struct Stream_Decoder_MPEG_4_SampleToGroupBoxEntry entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_SampleToGroupBox1 // sbgp version1
 : Stream_Decoder_MPEG_4_FullBoxHeader
{
  /*12*/ uint32_t grouping_type;
  /*16*/ uint32_t grouping_type_parameter;
  /*20*/ uint32_t entry_count;
  /*24*/ struct Stream_Decoder_MPEG_4_SampleToGroupBoxEntry entries[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_FreeSpaceBox // free
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*08*/ uint8_t data[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

struct Stream_Decoder_MPEG_4_MediaDataBox // mdat
 : Stream_Decoder_MPEG_4_BoxHeader
{
  /*08*/ uint8_t data[];
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__

#if defined (ACE_WIN32) || defined (ACE_WIN64)
__pragma (pack (pop))
#endif // ACE_WIN32 || ACE_WIN64

#endif
