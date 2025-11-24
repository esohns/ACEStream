#ifndef STREAM_MODULE_ML_COMMON_H
#define STREAM_MODULE_ML_COMMON_H

#if defined (__llvm__)
enum Stream_MachineLearning_BackendType
#else
enum Stream_MachineLearning_BackendType : int
#endif // __llvm__
{
  STREAM_ML_BACKEND_TENSORFLOW = 0,
  STREAM_ML_BACKEND_TENSORFLOW_CC,
  STREAM_ML_BACKEND_LIBTORCH,
  STREAM_ML_BACKEND_ONNX_RT,
  ////////////////////////////////////////
  STREAM_ML_BACKEND_MAX,
  STREAM_ML_BACKEND_INVALID
};

#endif
