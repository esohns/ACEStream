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

#ifndef TEST_I_CAMERA_ML_MODULE_TENSORFLOW_H
#define TEST_I_CAMERA_ML_MODULE_TENSORFLOW_H

#include <map>
#include <string>
#include <vector>

#if defined (OPENCV_SUPPORT)
#include "opencv2/core/mat.hpp"
#endif // OPENCV_SUPPORT

#if defined (TENSORFLOW_CC_SUPPORT)
#undef Status
#undef Success
#include "tensorflow/core/framework/tensor_shape.h"
#endif // TENSORFLOW_CC_SUPPORT

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_image_common.h"

#include "stream_common.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_module_tensorflow.h"

#if defined (TENSORFLOW_SUPPORT)
template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename MediaType>
class Test_I_CameraML_Module_Tensorflow_T
 : public Stream_Module_Tensorflow_T<ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_Module_Tensorflow_T<ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef typename inherited::ISTREAM_T ISTREAM_T;
  Test_I_CameraML_Module_Tensorflow_T (ISTREAM_T*); // stream handle
#else
  Test_I_CameraML_Module_Tensorflow_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_CameraML_Module_Tensorflow_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_Tensorflow_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_Tensorflow_T (const Test_I_CameraML_Module_Tensorflow_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_Tensorflow_T& operator= (const Test_I_CameraML_Module_Tensorflow_T&))

  // helper methods
  bool loadLabels (const std::string&); // label file path
  std::vector<size_t> filterBoxes (std::vector<float>&, // scores
                                   float);              // threshold (score)
  void drawBoundingBoxes (cv::Mat&,              // image
                          std::vector<float>&,   // scores
                          std::vector<float>&,   // classes
                          std::vector<int>&,     // boxes
                          std::vector<size_t>&); // indices ("good")

  struct TF_Output           input0_;
  struct TF_Output           inputs_a_[1];
  struct TF_Output           output0_;
  struct TF_Output           output1_;
  struct TF_Output           output2_;
  struct TF_Output           output3_;
  struct TF_Output           outputs_a_[4];

  std::map<int, std::string> labelMap_;
  Common_Image_Resolution_t  resolution_;
  int                        stride_;
};
#endif // TENSORFLOW_SUPPORT

///////////////////////////////////////////

#if defined (TENSORFLOW_CC_SUPPORT)
template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename MediaType>
class Test_I_CameraML_Module_Tensorflow_2
 : public Stream_Module_Tensorflow_2<ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_Module_Tensorflow_2<ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef typename inherited::ISTREAM_T ISTREAM_T;
  Test_I_CameraML_Module_Tensorflow_2 (ISTREAM_T*); // stream handle
#else
  Test_I_CameraML_Module_Tensorflow_2 (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_CameraML_Module_Tensorflow_2 () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_Tensorflow_2 ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_Tensorflow_2 (const Test_I_CameraML_Module_Tensorflow_2&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_Tensorflow_2& operator= (const Test_I_CameraML_Module_Tensorflow_2&))

  // helper methods
  bool loadLabels (const std::string&); // label file path
  std::vector<size_t> filterBoxes (tensorflow::TTypes<float>::Flat&,     // scores
                                   tensorflow::TTypes<float,3>::Tensor&, // boxes
                                   double,                               // threshold IOU
                                   double);                              // threshold (score)
  void drawBoundingBoxes (cv::Mat&,                             // image
                          tensorflow::TTypes<float>::Flat&,     // scores
                          tensorflow::TTypes<float>::Flat&,     // classes
                          tensorflow::TTypes<float,3>::Tensor&, // boxes
                          std::vector<size_t>&);                // indices ("good")

  std::map<int, std::string> labelMap_;
  Common_Image_Resolution_t  resolution_;
  tensorflow::TensorShape    shape_;
  int                        stride_;
};
#endif // TENSORFLOW_CC_SUPPORT

// include template definition
#include "test_i_camera_ml_module_tensorflow.inl"

#endif
