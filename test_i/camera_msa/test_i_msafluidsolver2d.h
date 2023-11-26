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

#ifndef TEST_I_MSAFLUIDSOLVER2D_H
#define TEST_I_MSAFLUIDSOLVER2D_H

#include "test_i_camera_msa_defines.h"

class MSAFluidSolver2D
{
 public:
  MSAFluidSolver2D (int, int);

 private:
  void reset ();

 public:
  void destroy ();
  void setup (int, int);
  void randomizeColor ();

 private:
  inline int FLUID_IX (int i, int j) { return (i + (NX_ + 2) * j); }

 public:
  inline int getIndexForCellPosition (int i, int j)
  {
    //if (i < 1) i = 1;
    //else if (i > NX_) i = NX_;
    //if (j < 1) j = 1;
    //else if (j > NY_) j = NY_;

    return FLUID_IX (i, j);
  }

  inline int getIndexForNormalizedPosition (float x, float y)
  {
    return getIndexForCellPosition (static_cast<int> (std::floor (x * (NX_ + 2))),
                                    static_cast<int> (std::floor (y * (NY_ + 2))));
  }

  inline int getWidth () { return NX_ + 2; }
  inline int getHeight () { return NY_ + 2; }

 private:
  void fadeR ();
  void fadeRGB ();

  void addSourceUV ();
  void addSourceRGB ();
  void addSource (float[], float[]);

  void setBoundary (int, float[]);
  void setBoundaryRGB (int);

  void advect (int, float[], float[], float[], float[]);
  void advectRGB (int, float[], float[]);

  void linearSolver (int, float[], float[], float, float);
  void linearSolverRGB (int, float, float);
  void linearSolverUV (int, float, float);

  void diffuse (int, float[], float[], float);
  void diffuseRGB (int, float);
  void diffuseUV (int, float);

  void project (float[], float[], float[], float[]);

  void swap (float*&, float*&);
  void swapRGB ();

 public:
  void update ();

  float* r_;
  float* rOld_;
  float* g_;
  float* gOld_;
  float* b_;
  float* bOld_;

  float* u_;
  float* uOld_;
  float* v_;
  float* vOld_;

  bool   isInitialized_;

  float  dt_;
  float  fadeSpeed_;
  int    solverIterations_;

  float  viscosity_;

  int    NX_;
  int    NY_;
  int    numCells_;
  bool   isRGB_;
  
  float  invNumCells_;

  float  avgSpeed_;
  float  avgDensity_;
  float  uniformity_;

  ////////////////////////////////////////
  float  UVCutoff_; // *TODO*: these do not really belong here
  int    step_;
  float  colorMultiplier_;
  float  velocityMultiplier_;
};

#endif // TEST_I_MSAFLUIDSOLVER2D_H
