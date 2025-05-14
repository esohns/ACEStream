#ifndef STREAM_DECODER_RIFF_COMMON_H
#define STREAM_DECODER_RIFF_COMMON_H

#include <cstdint>
#include <vector>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // _WIN32_WINNT && (_WIN32_WINNT >= 0x0602)
#endif // ACE_WIN32 || ACE_WIN64

struct RIFF_chunk_meta
{
  RIFF_chunk_meta ()
   : identifier (0)
   , size (0)
   , riff_list_identifier (0)
   , offset (0)
  {}

  bool operator== (const RIFF_chunk_meta& rhs_in) const
  {
    return ((identifier == rhs_in.identifier) ||
            (riff_list_identifier == rhs_in.riff_list_identifier));
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  DWORD        identifier;
#else
  uint32_t     identifier; // *NOTE*: libavformat type
#endif // ACE_WIN32 || ACE_WIN64

  // *NOTE*: adhering to the RIFF standard, this excludes the chunk
  //         'identifier', the 'size' field itself and any 'pad' bytes, iff odd
  ACE_UINT32   size;

  // *NOTE*: applies to RIFF and LIST chunks only
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //FOURCC      riff_list_identifier;
  DWORD        riff_list_identifier;
#else
  uint32_t     riff_list_identifier; // *NOTE*: libavformat type
#endif // ACE_WIN32 || ACE_WIN64

  ACE_UINT64   offset;
};

//struct less_RIFF_chunk_meta
//{
//  bool operator () (const RIFF_chunk_meta& lhs_in, const RIFF_chunk_meta& rhs_in) const
//  {
//    return (lhs_in.offset < rhs_in.offset);
//  }
//};
//typedef std::set<RIFF_chunk_meta, less_RIFF_chunk_meta> Stream_Decoder_RIFFChunks_t;
typedef std::vector<struct RIFF_chunk_meta> Stream_Decoder_RIFFChunks_t;
typedef Stream_Decoder_RIFFChunks_t::const_iterator Stream_Decoder_RIFFChunksIterator_t;

#endif
