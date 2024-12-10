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

#ifndef TEST_I_CAMERA_ML_MODULE_MEDIAPIPE_H
#define TEST_I_CAMERA_ML_MODULE_MEDIAPIPE_H

#include <vector>

#include "libmp.h"

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_image_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_ml_mediapipe_module_name_string[];

template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename MediaType>
class Test_I_CameraML_Module_MediaPipe_T
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef typename inherited::ISTREAM_T ISTREAM_T;
  Test_I_CameraML_Module_MediaPipe_T (ISTREAM_T*); // stream handle
#else
  Test_I_CameraML_Module_MediaPipe_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Test_I_CameraML_Module_MediaPipe_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_T (const Test_I_CameraML_Module_MediaPipe_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_T& operator= (const Test_I_CameraML_Module_MediaPipe_T&))

  // helper methods
  std::vector<std::vector<std::array<float, 3>>> getLandmarks (mediapipe::LibMP*);

  Common_Image_Resolution_t resolution_;
  int                       stride_;
  mediapipe::LibMP*         graph_;
};

// include template definition
#include "test_i_camera_ml_module_mediapipe.inl"

#endif
