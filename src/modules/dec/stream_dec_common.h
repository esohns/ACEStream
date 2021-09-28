#ifndef STREAM_DECODER_COMMON_H
#define STREAM_DECODER_COMMON_H

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

#endif
