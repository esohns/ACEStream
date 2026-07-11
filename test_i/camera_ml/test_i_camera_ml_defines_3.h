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

#ifndef TEST_I_CAMERA_ML_DEFINES_3_H
#define TEST_I_CAMERA_ML_DEFINES_3_H

#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_NAME_STRING                 "MediaPipe_Box2D"
#define TEST_I_CAMERA_ML_MEDIAPIPE_LIQUIDFUN_DEFAULT_NAME_STRING             "MediaPipe_LiquidFun"

#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_GRAVITY               9.81f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_STEP_FPS              60
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_SUBSTEPS              10
// *NOTE*: revolute joints are more 'rigid' than distance joints (nice), but
//         objects tend to fall 'through' them; distance joints don't have this
//         problem; weld joints seem to behave like revolute joints; rope joints
//         behave like distance joints
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_USE_REVOLUTE_JOINTS
//#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_USE_DISTANCE_JOINTS
// *NOTE*: when using revolute joints, higher vel/pos step counts are in order;
//         otherwise objects fall 'through' the bridge (*TODO*: why ?)
// *NOTE*: when using distance joints instead, the counts can be much lower
//         (e.g. 16/12), without objects falling 'through' (*TODO*: again, why ?)
#if defined (TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_USE_REVOLUTE_JOINTS)
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_ITER_VEL_PER_STEP     250
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_ITER_POS_PER_STEP     100
#else
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_ITER_VEL_PER_STEP     8
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_WORLD_ITER_POS_PER_STEP     3
#endif // TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_USE_REVOLUTE_JOINTS

#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_SPAWN_PROBABILITY      0.1f // *NOTE* per frame
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_DENSITY                1.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_X_OFFSET       25.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_FRICTION               0.1f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RADIUS                 6.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_RESTITUTION            1.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_ANG_VEL        5.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BALL_MAX_ABS_LIN_VEL        15.0f

#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_NUMBER_OF_BRIDGE_LINKS      7
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_ANGULAR_DAMPING 1.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_DENSITY         10.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_RADIUS          10.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_FRICTION        0.1f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_BODY_RESTITUTION     1.0f

#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_JOINT_LENGTH         1.0f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_JOINT_DAMP_RATIO     0.5f
#define TEST_I_CAMERA_ML_MEDIAPIPE_BOX2D_DEFAULT_BRIDGE_JOINT_FREQ_HZ        2.0f

#endif
