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

#ifndef TEST_U_CAMERA_FILTER_COMMON_H
#define TEST_U_CAMERA_FILTER_COMMON_H

#include <list>
#include <map>
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "BaseTyps.h"
#include "OAIdl.h"
#include "control.h"
#include "CGuid.h"
#include "Guiddef.h"
#include "d3d9.h"
#undef GetObject
#include "evr.h"
#include "mfapi.h"
#undef GetObject
#include "mfidl.h"
#include "strmif.h"
#else
#include "linux/videodev2.h"

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_tools.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_messageallocatorheap_base.h"
#else
#include "stream_messageallocatorheap_base.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_session_data.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_common.h"
#include "stream_lib_directshow_tools.h"
#else
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_u_common.h"

enum Test_U_CameraFilter_Mode
{
  TEST_U_MODE_SOBEL = 0,
  TEST_U_MODE_PERLIN_NOISE,
  TEST_U_MODE_MARCHING_SQUARES,
  TEST_U_MODE_WEIGHTED_VORONOI_STIPPLE,
  TEST_U_MODE_GLUT,
  TEST_U_MODE_GLUT_2,
  TEST_U_MODE_GLUT_3,
  TEST_U_MODE_GLUT_4,
  TEST_U_MODE_GLUT_5,
  TEST_U_MODE_GLUT_6,
  TEST_U_MODE_GLUT_7,
  TEST_U_MODE_GLUT_8,
  TEST_U_MODE_GLUT_9,
  TEST_U_MODE_GLUT_10,
  TEST_U_MODE_GLUT_11,
  TEST_U_MODE_GLUT_12,
  TEST_U_MODE_GLUT_13,
  TEST_U_MODE_GLUT_14,
  TEST_U_MODE_GLUT_15,
  TEST_U_MODE_GLUT_16,
  TEST_U_MODE_GLUT_17,
  TEST_U_MODE_GLUT_18,
  TEST_U_MODE_GLUT_19,
  TEST_U_MODE_GLUT_20,
  TEST_U_MODE_GLUT_21,
  TEST_U_MODE_GLUT_22,
  TEST_U_MODE_GLUT_23,
  TEST_U_MODE_GLUT_24,
  TEST_U_MODE_GLUT_25,
  TEST_U_MODE_GLUT_26,
  TEST_U_MODE_GLUT_27,
  TEST_U_MODE_GLUT_28,
  TEST_U_MODE_GLUT_29,
  TEST_U_MODE_GLUT_30,
  TEST_U_MODE_GLUT_31,
  TEST_U_MODE_GLUT_32,
  TEST_U_MODE_GLUT_33,
  TEST_U_MODE_GLUT_34,
  TEST_U_MODE_GLUT_35,
  TEST_U_MODE_GLUT_36,
  TEST_U_MODE_GLUT_37,
  TEST_U_MODE_GLUT_38,
  TEST_U_MODE_GLUT_39,
  TEST_U_MODE_GLUT_40,
  TEST_U_MODE_GLUT_41,
  TEST_U_MODE_GLUT_42,
  TEST_U_MODE_GLUT_43,
  TEST_U_MODE_GLUT_44,
  TEST_U_MODE_GLUT_45,
  TEST_U_MODE_GLUT_46,
  TEST_U_MODE_GLUT_47,
  TEST_U_MODE_GLUT_48,
  TEST_U_MODE_GLUT_49,
  TEST_U_MODE_GLUT_50
};

#endif
