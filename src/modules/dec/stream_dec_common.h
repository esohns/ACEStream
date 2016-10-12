#ifndef STREAM_DECODER_COMMON_H
#define STREAM_DECODER_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mmreg.h>
#include <dsound.h>
#else
#include <cstdint>
#endif

#include <set>

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include <mmiscapi.h>
//#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
union Stream_Decoder_DirectShow_AudioEffectOptions
{
  struct _DSFXChorus      chorusOptions;
  struct _DSFXCompressor  compressorOptions;
  struct _DSFXDistortion  distortionOptions;
  struct _DSFXEcho        echoOptions;
  struct _DSFXParamEq     equalizerOptions;
  struct _DSFXFlanger     flangerOptions;
  struct _DSFXGargle      gargleOptions;
  struct _DSFXI3DL2Reverb reverbOptions;
  struct _DSFXWavesReverb wavesReverbOptions;
};
#endif

enum Stream_Decoder_CompressionFormatType : int
{
  STREAM_COMPRESSION_FORMAT_NONE = -1,
  ////////////////////////////////////////
  STREAM_COMPRESSION_FORMAT_GZIP,
  STREAM_COMPRESSION_FORMAT_ZLIB,
  ////////////////////////////////////////
  STREAM_COMPRESSION_FORMAT_MAX,
  STREAM_COMPRESSION_FORMAT_INVALID
};

struct RIFF_chunk_header
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //FOURCC      fourcc;
  DWORD        fourcc;
#else
  uint32_t     fourcc; // *NOTE*: libavformat type
#endif
  unsigned int size;

  unsigned int offset;
};
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
