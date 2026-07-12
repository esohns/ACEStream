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

#ifndef TEST_I_CAMERA_ML_MODULE_MEDIAPIPE_3_BOX2D_H
#define TEST_I_CAMERA_ML_MODULE_MEDIAPIPE_3_BOX2D_H

#include <vector>

#include "box2d/box2d.h"

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

#include "test_i_camera_ml_defines.h"
#include "test_i_camera_ml_defines_3.h"

extern const char libacestream_default_ml_mediapipe_3_box2d_module_name_string[];

template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename MediaType>
class Test_I_CameraML_Module_MediaPipe_3_Box2d
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
    ball (b2WorldId& world_in, float halfDimension_in)
     : body_ (b2_nullBodyId)
     , shape_ (b2_nullShapeId)
    {
      b2BodyDef body_def = b2DefaultBodyDef ();
      body_def.type = b2_dynamicBody;
      body_def.enableSleep = false;
      body_def.position =
        { Common_Tools::getRandomNumber (-TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_X_OFFSET, TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_X_OFFSET),
          -halfDimension_in - TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS };
      // *NOTE*: this makes the balls appear mid-screen; what gives ?
      //body_def.rotation =
      //  b2MakeRot (Common_Tools::getRandomNumber (-static_cast<float> (M_PI), static_cast<float> (M_PI)));
      body_def.angularVelocity =
        Common_Tools::getRandomNumber (-TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_ANG_VEL, TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_ANG_VEL) * (static_cast<float> (M_PI) / 180.0f) / static_cast<float> (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_STEP_FPS);
      body_def.linearVelocity =
        { 0.0f,
          Common_Tools::getRandomNumber ( 5.0f * TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_LIN_VEL, 8.0f * TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_LIN_VEL) };
      body_ = b2CreateBody (world_in, &body_def);
      //ACE_ASSERT (body_ != b2_nullBodyId);

      b2Circle circle;
      circle.center = body_def.position;
      circle.radius = TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS;

      b2ShapeDef shape = b2DefaultShapeDef ();
      shape.density =
        TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_DENSITY;
      shape.material.friction =
        TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_FRICTION;
      shape.material.restitution =
        TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RESTITUTION;
      shape_ = b2CreateCircleShape (body_, &shape, &circle);
      // ACE_ASSERT (shape_ != b2_nullShapeId);
    }

    ~ball ()
    {
      //b2DestroyShape (shape_, false);
      b2DestroyBody (body_);
    }

    // *NOTE*: most balls disappear mid-screen; what gives ?
    inline bool isGone (float halfDimension_in) { return b2Body_GetPosition (body_).y > halfDimension_in; }

    b2BodyId  body_;
    b2ShapeId shape_;
  };

  struct box2dCBData
  {
    float                 halfDimension; // *NOTE*: box2d coordinates span is [-halfDimension, halfDimension]
    olc::PixelGameEngine* pge;
  };

  struct bridgeElement
  {
    b2BodyId  body;
    b2ShapeId shape;
  };

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef typename inherited::ISTREAM_T ISTREAM_T;
  Test_I_CameraML_Module_MediaPipe_3_Box2d (ISTREAM_T*); // stream handle
#else
  Test_I_CameraML_Module_MediaPipe_3_Box2d (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Test_I_CameraML_Module_MediaPipe_3_Box2d ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement (part of) b2DebugDraw
  static void DrawPolygon (b2WorldTransform, const b2Vec2*, int, b2HexColor, void*);
  static void DrawSolidPolygon (b2WorldTransform, const b2Vec2*, int, float, b2HexColor, void*);
  static void DrawCircle (b2Pos, float, b2HexColor, void*);
  static void DrawSolidCircle (b2WorldTransform, b2Pos, float, b2HexColor, void*);
  static void DrawLine (b2Pos, b2Pos, b2HexColor, void*);
  static void DrawTransform (b2WorldTransform, void*);
  static void DrawPoint (b2Pos, float, b2HexColor, void*);
  static void DrawBounds (b2AABB, b2HexColor, void*);

  // implement olc::PixelGameEngine
  virtual bool OnUserCreate ();
  virtual bool OnUserUpdate (float); // elapsed time
  virtual bool OnUserDestroy ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_3_Box2d ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_3_Box2d (const Test_I_CameraML_Module_MediaPipe_3_Box2d&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_CameraML_Module_MediaPipe_3_Box2d& operator= (const Test_I_CameraML_Module_MediaPipe_3_Box2d&))

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  // helper methods
  void initializeBox2d ();
  bool processNextMessage (bool&); // return value: stop PGE ?
  std::vector<std::vector<std::array<float, 3> > > getLandmarks (mediapipe::LibMP*);

  Common_Image_Resolution_t         resolution_;
  mediapipe::LibMP*                 graph_;

  struct box2dCBData                CBData_;
  b2DebugDraw                       draw_;
  b2WorldId                         world_;
  std::vector<struct bridgeElement> bridgeBodies_;
  std::vector<b2JointId>            bridgeJoints_;
  std::vector<ball*>                balls_;
  b2Pos                             positionThumb_;
  b2Pos                             positionIndex_;

  // *NOTE*: DrawSprite just calls Draw for every pixel...
  // olc::Sprite*                      sprite_; // for rendering camera image
};

// include template definition
#include "test_i_camera_ml_module_mediapipe_3_box2d.inl"

#endif
