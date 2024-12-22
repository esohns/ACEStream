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

#ifndef TEST_I_MODULE_PGE_H
#define TEST_I_MODULE_PGE_H

#include <vector>

#include "olcPixelGameEngine.h"

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_tools.h"

#include "common_image_tools.h"

#include "common_timer_common.h"

#include "stream_common.h"

#include "stream_lib_mediatype_converter.h"

#include "test_i_camera_msa_defines.h"
#include "test_i_msafluidsolver2d.h"

// extern Stream_Dec_Export const char
// libacestream_default_pge_module_name_string[];
extern const char libacestream_default_pge_module_name_string[];

template <typename TaskType, // Stream_TaskBaseSynch_T || Stream_TaskBaseAsynch_T
          typename MediaType>
class Test_I_Module_PGE_T
 : public TaskType
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public olc::PixelGameEngine
{
  typedef TaskType inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
  typedef olc::PixelGameEngine inherited3;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Module_PGE_T (typename TaskType::ISTREAM_T*); // stream handle
#else
  Test_I_Module_PGE_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_Module_PGE_T () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (typename inherited::DATA_MESSAGE_T*&, // data message handle
                                  bool&);                               // return value: pass message downstream ?
  virtual void handleSessionMessage (typename inherited::SESSION_MESSAGE_T*&, // session message handle
                                     bool&);                                  // return value: pass message downstream ?

  virtual bool OnUserCreate ();
  virtual bool OnUserUpdate (float); // elapsed time
  virtual bool OnUserDestroy ();

  MSAFluidSolver2D& getSolver () { return solver_; }

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_PGE_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_PGE_T (const Test_I_Module_PGE_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_PGE_T& operator= (const Test_I_Module_PGE_T&))

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  bool processNextMessage (); // return value: stop PGE ?

 public:
  class flow_zone
  {
   public:
    flow_zone (int x, int y, float u, float v)
     : x_ (x)
     , y_ (y)
     , u_ (u)
     , v_ (v)
    {}

    bool UVMeanAboveLimit (float UVCutoff_in)
    {
      return ((std::abs (u_) + std::abs (v_)) / 2.0f) > UVCutoff_in;
    }

    //void draw (olc::PixelGameEngine* engine_in)
    //{
    //  engine_in->DrawLine (x_, y_, x_ + static_cast<int32_t> (u_ * 3.0f), y_ + static_cast<int32_t> (v_ * 3.0f),
    //                       olc::WHITE, 0xFFFFFFFF);
    //}

    int   x_;
    int   y_;
    float u_;
    float v_;
  };

 private:
  std::vector<flow_zone> calculateFlow (char*, char*, int, int);
  void addForce (float, float, float, float, int);

  char*            previousImage_;
  char*            currentImage_;
  olc::Pixel*      fluidImage_;
  MSAFluidSolver2D solver_;
  float            aspectRatio2_;
};

// include template definition
#include "test_i_module_pge.inl"

#endif // TEST_I_MODULE_PGE_H
