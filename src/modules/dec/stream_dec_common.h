#ifndef STREAM_DECODER_COMMON_H
#define STREAM_DECODER_COMMON_H

#if defined(DEEPSPEECH_SUPPORT)
#include <string>
#include <utility>
#include <vector>
#endif // DEEPSPEECH_SUPPORT

#if defined (__llvm__)
enum Stream_Decoder_CompressionFormatType
#else
enum Stream_Decoder_CompressionFormatType : int
#endif // __llvm__
{
  STREAM_COMPRESSION_FORMAT_NONE = -1,
  ////////////////////////////////////////
  STREAM_COMPRESSION_FORMAT_GZIP,
  STREAM_COMPRESSION_FORMAT_ZLIB,
  ////////////////////////////////////////
  STREAM_COMPRESSION_FORMAT_MAX,
  STREAM_COMPRESSION_FORMAT_INVALID
};

#if defined (DEEPSPEECH_SUPPORT)
typedef std::vector <std::pair<std::string, float> > Stream_Decoder_DeepSpeech_HotWords_t;
typedef Stream_Decoder_DeepSpeech_HotWords_t::const_iterator Stream_Decoder_DeepSpeech_HotWordsConstIterator_t;

typedef std::vector <std::string> Stream_Decoder_DeepSpeech_Result_t;
typedef Stream_Decoder_DeepSpeech_Result_t::iterator Stream_Decoder_DeepSpeech_ResultIterator_t;
#endif // DEEPSPEECH_SUPPORT

#endif
