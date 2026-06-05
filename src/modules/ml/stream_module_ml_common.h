#ifndef STREAM_MODULE_ML_COMMON_H
#define STREAM_MODULE_ML_COMMON_H

#include <string>
#include <vector>

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

typedef std::vector<std::string> Stream_MachineLearning_LLM_Result_t;
typedef Stream_MachineLearning_LLM_Result_t::iterator Stream_MachineLearning_LLM_ResultIterator_t;
typedef std::iterator_traits<Stream_MachineLearning_LLM_ResultIterator_t>::difference_type Stream_MachineLearning_LLM_ResultDifference_t;
typedef Stream_MachineLearning_LLM_Result_t::const_iterator Stream_MachineLearning_LLM_ResultConstIterator_t;

#endif
