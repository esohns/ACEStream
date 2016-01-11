#ifndef STREAM_DECODER_COMMON_H
#define STREAM_DECODER_COMMON_H

#include <set>

#include "stream_dec_avi_parser.h"

struct less_RIFF_chunk_header
{
  bool operator () (const RIFF_chunk_header& lhs_in, const RIFF_chunk_header& rhs_in) const
  {
    return (lhs_in.offset < rhs_in.offset);
  }
};
typedef std::set<RIFF_chunk_header, less_RIFF_chunk_header> Stream_Decoder_RIFFChunks_t;
typedef Stream_Decoder_RIFFChunks_t::const_iterator Stream_Decoder_RIFFChunksIterator_t;

#endif
