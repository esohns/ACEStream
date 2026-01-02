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
#include "stdafx.h"

#include "test_u_camerascreen_module_onnx.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "test_u_camerascreen_defines.h"
#include "test_u_camerascreen_message.h"
#include "test_u_camerascreen_session_message.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_ONNX::Test_U_CameraScreen_ONNX (ISTREAM_T* stream_in)
#else
Test_U_CameraScreen_ONNX::Test_U_CameraScreen_ONNX (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_ONNX::Test_U_CameraScreen_ONNX"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_ONNX::handleDataMessage (Stream_CameraScreen_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraScreen_ONNX::handleDataMessage (Stream_CameraScreen_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_ONNX::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::APIHandle_);
  ACE_ASSERT (inherited::memory_info_);
  ACE_ASSERT (inherited::session_);

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

  inherited::hwc_to_chw (reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                         resolution_.cy, resolution_.cx,
#else
                         resolution_.height, resolution_.width,
#endif // ACE_WIN32 || ACE_WIN64
                         model_input_p);
  ACE_ASSERT (model_input_p);

  OrtStatus* status_p =
    inherited::APIHandle_->CreateTensorWithDataAsOrtValue (memory_info_,
                                                           model_input_p, model_input_len,
                                                           input_shape_a, input_shape_len, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
                                                           &input_tensor_p);
  if (unlikely (status_p || !input_tensor_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::CreateTensorWithDataAsOrtValue(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                inherited::APIHandle_->GetErrorMessage (status_p)));
    inherited::APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF
  status_p = inherited::APIHandle_->IsTensor (input_tensor_p,
                                              &is_tensor_i);
  if (unlikely (status_p || !is_tensor_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::IsTensor(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                inherited::APIHandle_->GetErrorMessage (status_p)));
    inherited::APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF

  status_p = inherited::APIHandle_->Run (inherited::session_,
                                         NULL,
                                         input_names_a, (const OrtValue* const*)&input_tensor_p, 1,
                                         output_names_a, 1, &output_tensor_p);
  if (unlikely (status_p || !output_tensor_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::Run(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                inherited::APIHandle_->GetErrorMessage (status_p)));
    inherited::APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF
  free (model_input_p); model_input_p = NULL;
  inherited::APIHandle_->ReleaseValue (input_tensor_p); input_tensor_p = NULL;

  is_tensor_i = 0;
  status_p = inherited::APIHandle_->IsTensor (output_tensor_p,
                                              &is_tensor_i);
  if (unlikely (status_p || !is_tensor_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::IsTensor(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                inherited::APIHandle_->GetErrorMessage (status_p)));
    inherited::APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF

  status_p =
    inherited::APIHandle_->GetTensorMutableData (output_tensor_p,
                                                 (void**)&output_tensor_data_p);
  if (unlikely (status_p || !output_tensor_data_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to OrtApi::GetTensorMutableData(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                inherited::APIHandle_->GetErrorMessage (status_p)));
    inherited::APIHandle_->ReleaseStatus (status_p);
    goto error;
  } // end IF

  message_inout->reset ();
  inherited::chw_to_hwc (output_tensor_data_p,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                         inherited::resolution_.cy, inherited::resolution_.cx,
#else
                         inherited::resolution_.height, inherited::resolution_.width,
#endif // ACE_WIN32 || ACE_WIN64
                         reinterpret_cast<uint8_t*> (message_inout->wr_ptr ()));
  message_inout->wr_ptr (model_input_len);

  inherited::APIHandle_->ReleaseValue (output_tensor_p); output_tensor_p = NULL;

  //// step3b: draw fps
  //std::ostringstream converter;
  //converter << fps;

  return;

error:
  if (input_tensor_p)
    inherited::APIHandle_->ReleaseValue (input_tensor_p);
  if (model_input_p)
    free (model_input_p);
  if (output_tensor_p)
    inherited::APIHandle_->ReleaseValue (output_tensor_p);

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_ONNX::handleSessionMessage (Stream_CameraScreen_DirectShow_SessionMessage_t*& message_inout,
#else
Test_U_CameraScreen_ONNX::handleSessionMessage (Stream_CameraScreen_SessionMessage_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_ONNX::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      const Stream_CameraScreen_DirectShow_SessionData& session_data_r =
        inherited::sessionData_->getR ();
#else
      const Stream_CameraScreen_V4L_SessionData& session_data_r =
        inherited::sessionData_->getR ();
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_s);
      if (!InlineIsEqualGUID (media_type_s.subtype, MEDIASUBTYPE_RGB24))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid format (was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_type_s.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
        goto error;
      } // end IF
      inherited::resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      if (inherited::resolution_.cx != TEST_U_ONNX_MODEL_RESOLUTION_DEFAULT_XY ||
          inherited::resolution_.cy != TEST_U_ONNX_MODEL_RESOLUTION_DEFAULT_XY)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid resolution (was: %dx%d), aborting\n"),
                    inherited::mod_->name (),
                    inherited::resolution_.cx, inherited::resolution_.cy));
        goto error;
      } // end IF
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_V4L_MediaType media_type_s;
      inherited::getMediaType (session_data_r.formats.back (),
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
      inherited::resolution_.width = media_type_s.format.width;
      inherited::resolution_.height = media_type_s.format.height;
      if (inherited::resolution_.width  != TEST_U_ONNX_MODEL_RESOLUTION_DEFAULT_XY ||
          inherited::resolution_.height != TEST_U_ONNX_MODEL_RESOLUTION_DEFAULT_XY)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid resolution (was: %ux%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::resolution_.width, inherited::resolution_.height));
        goto error;
      } // end IF
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
