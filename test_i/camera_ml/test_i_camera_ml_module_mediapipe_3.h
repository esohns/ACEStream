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

#ifndef TEST_I_CAMERA_ML_MODULE_MEDIAPIPE_3_H
#define TEST_I_CAMERA_ML_MODULE_MEDIAPIPE_3_H

#include <vector>

#include "Box2D/Box2D.h"

#include "olcPixelGameEngine.h"

#include "libmp.h"

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_tools.h"

#include "common_image_common.h"

#include "stream_common.h"
#include "stream_task_base_asynch.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_ml_mediapipe_3_module_name_string[];

template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename MediaType>
class Test_I_CameraML_Module_MediaPipe_3
 : public Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public olc::PixelGameEngine
 , public b2Draw
{
  typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                  Common_TimePolicy_t,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
  typedef olc::PixelGameEngine inherited3;

 public:
  class ball
  {
   public:
    ball (b2World* world_in, float halfDimension_in)
     : body_ (NULL)
    {
      b2BodyDef body_def;
      body_def.type = b2_dynamicBody;
      body_def.position.Set (0.0f, -halfDimension_in);
      body_def.linearVelocity.Set (Common_Tools::getRandomNumber (-5.0f, 5.0f),
                                   Common_Tools::getRandomNumber (-5.0f, 5.0f));
      body_ = world_in->CreateBody (&body_def);
      b2CircleShape shape;
      shape.m_radius = TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS;
      b2FixtureDef fixture_def;
      fixture_def.shape = &shape;
      fixture_def.density = 1.0f;
      fixture_def.friction = 0.1f;
      fixture_def.restitution = 0.85f;
      body_->CreateFixture (&fixture_def);
    }

    ~ball ()
    {
      if (body_)
        body_->GetWorld ()->DestroyBody (body_);
    }

    bool isGone (float halfDimension_in)
    {
      return body_->GetPosition ().y > halfDimension_in;
    }

    b2Body* body_;
  };

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef typename inherited::ISTREAM_T ISTREAM_T;
  Test_I_CameraML_Module_MediaPipe_3 (ISTREAM_T*); // stream handle
#else
  Test_I_CameraML_Module_MediaPipe_3 (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Test_I_CameraML_Module_MediaPipe_3 ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement b2Draw
  virtual void DrawPolygon (const b2Vec2*, int32, const b2Color&);
  virtual void DrawSolidPolygon (const b2Vec2*, int32, const b2Color&);
  virtual void DrawCircle (const b2Vec2&, float32, const b2Color&);
  virtual void DrawSolidCircle (const b2Vec2&, float32, const b2Vec2&, const b2Color&);
  virtual void DrawParticles (const b2Vec2*, float32, const b2ParticleColor*, int32);
  virtual void DrawSegment (const b2Vec2&, const b2Vec2&, const b2Color&);
  virtual void DrawTransform (const b2Transform&);

  // implement olc::PixelGameEngine
  virtual bool OnUserCreate ();
  virtual bool OnUserUpdate (float); // elapsed time
  virtual bool OnUserDestroy ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_3 ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_3 (const Test_I_CameraML_Module_MediaPipe_3&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_3& operator= (const Test_I_CameraML_Module_MediaPipe_3&))

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  // helper methods
  void initializeBox2d ();
  bool processNextMessage (); // return value: stop PGE ?
  std::vector<std::vector<std::array<float, 3>>> getLandmarks (mediapipe::LibMP*);

  Common_Image_Resolution_t resolution_;
  int                       stride_;
  mediapipe::LibMP*         graph_;

  b2World*                  world_;
  float                     halfDimension_; // *NOTE*: coords go from [-halfDimension_, halfDimension_]
  std::vector<b2Body*>      bridge_;
  std::vector<ball*>        balls_;
  b2Vec2                    positionThumb_, positionIndex_;
};

// include template definition
#include "test_i_camera_ml_module_mediapipe_3.inl"

#endif
