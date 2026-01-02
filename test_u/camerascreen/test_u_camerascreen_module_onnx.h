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

#ifndef TEST_U_CAMERASCREEN_MODULE_ONNX_H
#define TEST_U_CAMERASCREEN_MODULE_ONNX_H

#include "stream_control_message.h"
#include "stream_streammodule_base.h"

#include "stream_module_onnxruntime.h"

#include "test_u_camerascreen_common.h"

class Test_U_CameraScreen_ONNX
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 : public Stream_Module_ONNXRuntime_T<struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CameraScreen_DirectShow_Message_t,
                                      Stream_CameraScreen_DirectShow_SessionMessage_t,
                                      struct _AMMediaType>
#else
 : public Stream_Module_ONNXRuntime_T<struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_CameraScreen_Message_t,
                                      Stream_CameraScreen_SessionMessage_t,
                                      struct Stream_MediaFramework_V4L_MediaType>
#endif // ACE_WIN32 || ACE_WIN64
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 typedef Stream_Module_ONNXRuntime_T<struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CameraScreen_DirectShow_Message_t,
                                     Stream_CameraScreen_DirectShow_SessionMessage_t,
                                     struct _AMMediaType> inherited;
#else
 typedef Stream_Module_ONNXRuntime_T<struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CameraScreen_Message_t,
                                     Stream_CameraScreen_SessionMessage_t,
                                     struct Stream_MediaFramework_V4L_MediaType> inherited;
#endif // ACE_WIN32 || ACE_WIN64

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_CameraScreen_ONNX (ISTREAM_T*); // stream handle
#else
  Test_U_CameraScreen_ONNX (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_U_CameraScreen_ONNX () {}

  // implement (part of) Stream_ITaskBase_T
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void handleDataMessage (Stream_CameraScreen_DirectShow_Message_t*&, // message handle
                                  bool&);                                     // return value: pass message downstream ?
  virtual void handleSessionMessage (Stream_CameraScreen_DirectShow_SessionMessage_t*&, // session message handle
                                     bool&);                                            // return value: pass message downstream ?
#else
  virtual void handleDataMessage (Stream_CameraScreen_Message_t*&, // message handle
                                  bool&);                          // return value: pass message downstream ?
  virtual void handleSessionMessage (Stream_CameraScreen_SessionMessage_t*&, // session message handle
                                     bool&);                                 // return value: pass message downstream ?
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraScreen_ONNX ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraScreen_ONNX (const Test_U_CameraScreen_ONNX&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_CameraScreen_ONNX& operator= (const Test_U_CameraScreen_ONNX&))
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct Stream_CameraScreen_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_ml_onnxruntime_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_CameraScreen_ONNX);                                        // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraScreen_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Stream_CameraScreen_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_ml_onnxruntime_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_CameraScreen_ONNX);                                 // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
