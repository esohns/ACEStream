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

#ifndef TEST_I_CAMERA_AR_MODULE_CGE_H
#define TEST_I_CAMERA_AR_MODULE_CGE_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mmeapi.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "olcConsoleGameEngine.h"

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_timer_common.h"

#include "stream_common.h"

#include "stream_lib_mediatype_converter.h"

// extern Stream_Dec_Export const char
// libacestream_default_pge_module_name_string[];
extern const char libacestream_default_cge_module_name_string[];

template <typename TaskType, // Stream_TaskBaseSynch_T || Stream_TaskBaseAsynch_T
          typename MediaType>
class Test_I_CameraAR_Module_CGE_T
 : public TaskType
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public olcConsoleGameEngine
{
  typedef TaskType inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
  typedef olcConsoleGameEngine inherited3;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_CameraAR_Module_CGE_T (typename TaskType::ISTREAM_T*); // stream handle
#else
  Test_I_CameraAR_Module_CGE_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_CameraAR_Module_CGE_T () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (typename inherited::DATA_MESSAGE_T*&, // data message handle
                                  bool&);                               // return value: pass message downstream ?
  virtual void handleSessionMessage (typename inherited::SESSION_MESSAGE_T*&, // session message handle
                                     bool&);                                  // return value: pass message downstream ?

  virtual bool OnUserCreate ();
  virtual bool OnUserUpdate (float); // elapsed time
  virtual bool OnUserDestroy ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraAR_Module_CGE_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraAR_Module_CGE_T (const Test_I_CameraAR_Module_CGE_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraAR_Module_CGE_T& operator= (const Test_I_CameraAR_Module_CGE_T&))

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  void drawImage (float*);
  inline float getPixel (float* image_in, int x_in, int y_in)
  {
    if (x_in >= 0 && x_in < ScreenWidth () && y_in >= 0 && y_in < ScreenHeight ())
      return image_in[y_in * ScreenWidth () + x_in];
    else
      return 0.0F;
  }

  bool processNextMessage (); // return value: stop PGE ?

	float* previousImage;
  float* currentImage;
  float* previousFilteredImage;
  float* currentFilteredImage;
  float* previousMotionImage;
  float* currentMotionImage;
  float* flowFieldX;
  float* flowFieldY;

  float ballX; // position
  float ballY;
  float ballVelocityX; // velocity
  float ballVelocityY;
};

// include template definition
#include "test_i_camera_ar_module_cge.inl"

#endif
