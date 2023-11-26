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

#ifndef TEST_I_CAMERA_MSA_DEFINES_H
#define TEST_I_CAMERA_MSA_DEFINES_H

#define STREAM_CGE_DEFAULT_NAME_STRING "CGE"
#define STREAM_PGE_DEFAULT_NAME_STRING "PGE"

// fluid solver
#define FLUID_DEFAULT_FLUID_WIDTH         128  // fluid image size
#define FLUID_DEFAULT_UV_CUTOFF           4.0f // threshold for flow values
#define FLUID_DEFAULT_STEP                8    // (half of) gradient window size

#define FLUID_DEFAULT_DT                  1.0f
//#define FLUID_DEFAULT_FADESPEED         0.0f
#define FLUID_DEFAULT_FADESPEED           0.015f
#define FLUID_DEFAULT_SOLVER_ITERATIONS   10

//#define FLUID_DEFAULT_VISCOSITY         0.0001f
#define FLUID_DEFAULT_VISCOSITY           0.00008f

#define FLUID_DEFAULT_COLOR_MULTIPLIER    5.0f
#define FLUID_DEFAULT_VELOCITY_MULTIPLIER 3.0f

// GTK
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME               "dialog_main"
#define TEST_I_UI_GTK_SCALE_UVCUTOFF_NAME            "scale_uv_cutoff"
#define TEST_I_UI_GTK_SCALE_STEP_NAME                "scale_step"
#define TEST_I_UI_GTK_SCALE_DT_NAME                  "scale_dt"
#define TEST_I_UI_GTK_SCALE_FADE_NAME                "scale_fade"
#define TEST_I_UI_GTK_SCALE_ITERATIONS_NAME          "scale_iterations"
#define TEST_I_UI_GTK_SCALE_VISCOSITY_NAME           "scale_viscosity"
#define TEST_I_UI_GTK_SCALE_COLOR_MULTIPLIER_NAME    "scale_color_multiplier"
#define TEST_I_UI_GTK_SCALE_VELOCITY_MULTIPLIER_NAME "scale_velocity_multiplier"

#endif // TEST_I_CAMERA_MSA_DEFINES_H
