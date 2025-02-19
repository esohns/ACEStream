#ifndef STREAM_DECODER_COMMON_H
#define STREAM_DECODER_COMMON_H

#include <string>
#if defined (DEEPSPEECH_SUPPORT)
#include <utility>
#endif // DEEPSPEECH_SUPPORT
#include <vector>

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

typedef std::vector <std::string> Stream_Decoder_STT_Result_t;
typedef Stream_Decoder_STT_Result_t::iterator Stream_Decoder_STT_ResultIterator_t;
typedef std::iterator_traits<Stream_Decoder_STT_ResultIterator_t>::difference_type Stream_Decoder_STT_ResultDifference_t;
typedef Stream_Decoder_STT_Result_t::const_iterator Stream_Decoder_STT_ResultConstIterator_t;

#if defined (DEEPSPEECH_SUPPORT)
typedef std::vector <std::pair<std::string, float> > Stream_Decoder_DeepSpeech_HotWords_t;
typedef Stream_Decoder_DeepSpeech_HotWords_t::const_iterator Stream_Decoder_DeepSpeech_HotWordsConstIterator_t;
#endif // DEEPSPEECH_SUPPORT

#endif
