/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ace/Log_Msg.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_ONNXRuntime_T<ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::Stream_Module_ONNXRuntime_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , APIHandle_ (NULL)
 , env_ (NULL)
 , memory_info_ (NULL)
 , session_ (NULL)
 , resolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ONNXRuntime_T::Stream_Module_ONNXRuntime_T"));

}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_ONNXRuntime_T<ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::~Stream_Module_ONNXRuntime_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ONNXRuntime_T::~Stream_Module_ONNXRuntime_T"));

  if (session_)
    APIHandle_->ReleaseSession (session_);
  if (memory_info_)
    APIHandle_->ReleaseMemoryInfo (memory_info_);
  if (env_)
    APIHandle_->ReleaseEnv (env_);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Module_ONNXRuntime_T<ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::initialize (const ConfigurationType& configuration_in,
                                                    Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ONNXRuntime_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (session_)
    { ACE_ASSERT (APIHandle_);
      APIHandle_->ReleaseSession (session_); session_ = NULL;
    } // end IF
    if (memory_info_)
    { ACE_ASSERT (APIHandle_);
      APIHandle_->ReleaseMemoryInfo (memory_info_); memory_info_ = NULL;
    } // end IF
    if (env_)
    { ACE_ASSERT (APIHandle_);
      APIHandle_->ReleaseEnv (env_); env_ = NULL;
    } // end IF
    APIHandle_ = NULL;
  } // end IF

  const OrtApiBase* api_base_p = OrtGetApiBase ();
  ACE_ASSERT (api_base_p);
  APIHandle_ = const_cast<OrtApi*> (api_base_p->GetApi (ORT_API_VERSION));
  if (unlikely (!APIHandle_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApiBase::GetApi(%d), aborting\n"),
                inherited::mod_->name (),
                ORT_API_VERSION));
    return false;
  } // end IF

  OrtStatus* status_p = APIHandle_->CreateEnv (ORT_LOGGING_LEVEL_WARNING,
                                               ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()),
                                               &env_);
  if (unlikely (status_p || !env_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::CreateEnv(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    return false;
  } // end IF

  status_p = APIHandle_->CreateCpuMemoryInfo (OrtArenaAllocator,
                                              OrtMemTypeDefault,
                                              &memory_info_);
  if (unlikely (status_p || !memory_info_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::CreateCpuMemoryInfo(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    return false;
  } // end IF

  OrtSessionOptions* session_options_p = NULL;
  status_p = APIHandle_->CreateSessionOptions (&session_options_p);
  if (unlikely (status_p || !session_options_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::CreateSessionOptions(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    return false;
  } // end IF

  struct OrtCUDAProviderOptions CUDA_provider_options_s;
  ACE_OS::memset (&CUDA_provider_options_s, 0, sizeof (struct OrtCUDAProviderOptions));
  CUDA_provider_options_s.gpu_mem_limit = SIZE_MAX;
  status_p =
    APIHandle_->SessionOptionsAppendExecutionProvider_CUDA (session_options_p,
                                                            &CUDA_provider_options_s);
  if (unlikely (status_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::SessionOptionsAppendExecutionProvider_CUDA(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p); status_p = NULL;
  } // end IF

  struct OrtROCMProviderOptions ROCM_provider_options_s;
  ACE_OS::memset (&ROCM_provider_options_s, 0, sizeof (struct OrtROCMProviderOptions));
  ROCM_provider_options_s.gpu_mem_limit = SIZE_MAX;
  ROCM_provider_options_s.do_copy_in_default_stream = 1;
  status_p =
    APIHandle_->SessionOptionsAppendExecutionProvider_ROCM (session_options_p,
                                                            &ROCM_provider_options_s);
  if (unlikely (status_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::SessionOptionsAppendExecutionProvider_ROCM(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p); status_p = NULL;
  } // end IF

// APIHandle_->SessionOptionsAppendExecutionProvider_DML (session_options_p, NULL);

  status_p = APIHandle_->CreateSession (env_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                        ACE_TEXT_ALWAYS_WCHAR (configuration_in.model.c_str ()),
#else
                                        configuration_in.model.c_str (),
#endif // ACE_WIN32 || ACE_WIN64
                                        session_options_p,
                                        &session_);
  if (unlikely (status_p || !session_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::CreateSession(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.model.c_str ()),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    APIHandle_->ReleaseSessionOptions (session_options_p);
    return false;
  } // end IF
  APIHandle_->ReleaseSessionOptions (session_options_p); session_options_p = NULL;

  size_t count_i = 0;
  status_p = APIHandle_->SessionGetInputCount (session_, &count_i);
  if (unlikely (status_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::SessionGetInputCount(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    APIHandle_->ReleaseSession (session_); session_ = NULL;
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: model \"%s\" has %Q input(s)\n"),
              inherited::mod_->name (),
              ACE_TEXT (configuration_in.model.c_str ()),
              count_i));

  status_p = APIHandle_->SessionGetOutputCount (session_, &count_i);
  if (unlikely (status_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::SessionGetOutputCount(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    APIHandle_->ReleaseSession (session_); session_ = NULL;
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: model \"%s\" has %Q output(s)\n"),
              inherited::mod_->name (),
              ACE_TEXT (configuration_in.model.c_str ()),
              count_i));

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_ONNXRuntime_T<ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ONNXRuntime_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (APIHandle_);
  ACE_ASSERT (memory_info_);
  ACE_ASSERT (session_);

  //static int nFrames = 30; // i.e. recompute fps roughly every second of footage
  //static int iFrame = 0;
  //static float fps = 0.0f;
  //static time_t start = time (NULL);
  //static time_t end;

  //if (nFrames % (iFrame + 1) == 0)
  //{
  //  time (&end);
  //  fps = nFrames / (float)difftime (end, start);
  //  time (&start);
  //} // end IF
  //++iFrame;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  const int64_t input_shape_a[] = {1, 3, resolution_.cx, resolution_.cy};
#else
  const int64_t input_shape_a[] = {1, 3, resolution_.width, resolution_.height};
#endif // ACE_WIN32 || ACE_WIN64
  const size_t input_shape_len =
    sizeof (input_shape_a) / sizeof (input_shape_a[0]);
  const size_t model_input_ele_count =
    input_shape_a[0] * input_shape_a[1] * input_shape_a[2] * input_shape_a[3];
  const size_t model_input_len = model_input_ele_count * sizeof (float);
  float* model_input_p = NULL;

  OrtValue* input_tensor_p = NULL;
  int is_tensor_i = 0;

  const char* input_names_a[] = { ACE_TEXT_ALWAYS_CHAR ("inputImage") };
  const char* output_names_a[] = { ACE_TEXT_ALWAYS_CHAR ("outputImage") };

  OrtValue* output_tensor_p = NULL;
  float* output_tensor_data_p = NULL;

  hwc_to_chw (reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              resolution_.cy, resolution_.cx,
#else
              resolution_.height, resolution_.width,
#endif // ACE_WIN32 || ACE_WIN64
              model_input_p);
  ACE_ASSERT (model_input_p);

  OrtStatus* status_p =
    APIHandle_->CreateTensorWithDataAsOrtValue (memory_info_,
                                                model_input_p, model_input_len,
                                                input_shape_a, input_shape_len, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
                                                &input_tensor_p);
  if (unlikely (status_p || !input_tensor_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::CreateTensorWithDataAsOrtValue(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF
  status_p = APIHandle_->IsTensor (input_tensor_p, &is_tensor_i);
  if (unlikely (status_p || !is_tensor_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::IsTensor(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF

  status_p = APIHandle_->Run (session_,
                              NULL,
                              input_names_a, (const OrtValue* const*)&input_tensor_p, 1,
                              output_names_a, 1, &output_tensor_p);
  if (unlikely (status_p || !output_tensor_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::Run(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF
  free (model_input_p); model_input_p = NULL;
  APIHandle_->ReleaseValue (input_tensor_p); input_tensor_p = NULL;

  is_tensor_i = 0;
  status_p = APIHandle_->IsTensor (output_tensor_p, &is_tensor_i);
  if (unlikely (status_p || !is_tensor_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::IsTensor(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF

  status_p = APIHandle_->GetTensorMutableData (output_tensor_p,
                                               (void**)&output_tensor_data_p);
  if (unlikely (status_p || !output_tensor_data_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::GetTensorMutableData(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                APIHandle_->GetErrorMessage (status_p)));
    APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF

  message_inout->reset ();
  chw_to_hwc (output_tensor_data_p,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              resolution_.cy, resolution_.cx,
#else
              resolution_.height, resolution_.width,
#endif // ACE_WIN32 || ACE_WIN64
              reinterpret_cast<uint8_t*> (message_inout->wr_ptr ()));
  message_inout->wr_ptr (model_input_len);

  APIHandle_->ReleaseValue (output_tensor_p); output_tensor_p = NULL;

  //// step3b: draw fps
  //std::ostringstream converter;
  //converter << fps;

  return;

error:
  if (input_tensor_p)
    APIHandle_->ReleaseValue (input_tensor_p);
  if (model_input_p)
    free (model_input_p);
  if (output_tensor_p)
    APIHandle_->ReleaseValue (output_tensor_p);

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_ONNXRuntime_T<ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ONNXRuntime_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      // *TODO*: remove this test; it's not generic
      if (!InlineIsEqualGUID (media_type_s.subtype, MEDIASUBTYPE_RGB24))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid format (was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_type_s.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
        goto error;
      } // end IF
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_V4L_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      if (media_type_s.format.pixelformat != V4L2_PIX_FMT_BGR24)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid format (was: %d), aborting\n"),
                    inherited::mod_->name (),
                    media_type_s.format.pixelformat));
        goto error;
      } // end IF
      resolution_.width = media_type_s.format.width;
      resolution_.height = media_type_s.format.height;
#endif // ACE_WIN32 || ACE_WIN64

      break;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
//    case STREAM_SESSION_MESSAGE_RESIZE:
//    {
//      break;
//    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_ONNXRuntime_T<ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::hwc_to_chw (const uint8_t* data_in,
                                                    unsigned int height_in,
                                                    unsigned int width_in,
                                                    float*& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ONNXRuntime_T::hwc_to_chw"));

  // sanity check(s)
  ACE_ASSERT (!data_out);

  unsigned int stride_i = height_in * width_in;
  data_out = (float*)malloc (stride_i * 3 * sizeof (float));
  ACE_ASSERT (data_out);

  for (size_t i = 0; i != stride_i; ++i)
    for (size_t c = 0; c != 3; ++c)
      data_out[c * stride_i + i] = static_cast<float> (data_in[i * 3 + c]);
}

template <typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_ONNXRuntime_T<ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::chw_to_hwc (const float* data_in,
                                                    unsigned int height_in,
                                                    unsigned int width_in,
                                                    uint8_t* data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ONNXRuntime_T::chw_to_hwc"));

  // sanity check(s)
  ACE_ASSERT (data_out);

  unsigned int stride_i = height_in * width_in;
  for (size_t c = 0; c != 3; ++c)
  {
    size_t t = c * stride_i;
    for (size_t i = 0; i != stride_i; ++i)
    {
      float f = data_in[t + i];
      if (f < 0.0f)
        f = 0.0f;
      else if (f > 255.0f)
        f = 255.0f;
      data_out[i * 3 + c] = (uint8_t)f;
    } // end IF
  } // end IF
}
