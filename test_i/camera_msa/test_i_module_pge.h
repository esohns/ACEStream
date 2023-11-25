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

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_PGE_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_PGE_T (const Test_I_Module_PGE_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Module_PGE_T& operator= (const Test_I_Module_PGE_T&))

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  bool processNextMessage (); // return value: stop PGE ?

  char* previousImage_;
  char* currentImage_;
  olc::Pixel* fluidImage_;

  class MSAFluidSolver2D
  {
   public:
    MSAFluidSolver2D (int NX, int NY)
     : r_ (NULL)
     , rOld_ (NULL)
     , g_ (NULL)
     , gOld_ (NULL)
     , b_ (NULL)
     , bOld_ (NULL)
     , u_ (NULL)
     , uOld_ (NULL)
     , v_ (NULL)
     , vOld_ (NULL)
     , isInitialized_ (false)
     , viscosity_ (FLUID_DEFAULT_VISCOSITY)
     , dt_ (FLUID_DEFAULT_DT)
     , fadeSpeed_ (FLUID_DEFAULT_FADESPEED)
     , solverIterations_ (FLUID_DEFAULT_SOLVER_ITERATIONS)
     , NX_ (0)
     , NY_ (0)
     , numCells_ (0)
     , isRGB_ (true)
     , invNumCells_ (0.0f)
     , avgSpeed_ (0.0f)
     , avgDensity_ (0.0f)
     , uniformity_ (0.0f)
    {
      setup (NX, NY);
    }

    void destroy ()
    {
      delete[] r_; r_ = NULL;
      delete[] rOld_; rOld_ = NULL;
      delete[] g_; g_ = NULL;
      delete[] gOld_; gOld_ = NULL;
      delete[] b_; b_ = NULL;
      delete[] bOld_; bOld_ = NULL;

      delete[] u_; u_ = NULL;
      delete[] uOld_; uOld_ = NULL;
      delete[] v_; v_ = NULL;
      delete[] vOld_; vOld_ = NULL;

      isInitialized_ = false;
    }

    void reset()
    {
      destroy ();
        
      r_    = new float[numCells_];
      rOld_ = new float[numCells_];
      g_    = new float[numCells_];
      gOld_ = new float[numCells_];
      b_    = new float[numCells_];
      bOld_ = new float[numCells_];

      u_    = new float[numCells_];
      uOld_ = new float[numCells_];
      v_    = new float[numCells_];
      vOld_ = new float[numCells_];

      ACE_OS::memset (r_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (rOld_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (g_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (gOld_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (b_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (bOld_, 0, sizeof (float) * numCells_);

      ACE_OS::memset (u_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (uOld_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (v_, 0, sizeof (float) * numCells_);
      ACE_OS::memset (vOld_, 0, sizeof (float) * numCells_);

      isInitialized_ = true;
    }

    void setup (int NX, int NY)
    {
      dt_ = FLUID_DEFAULT_DT;
      fadeSpeed_ = FLUID_DEFAULT_FADESPEED;
      solverIterations_ = FLUID_DEFAULT_SOLVER_ITERATIONS;

      NX_ = NX;
      NY_ = NY;
      numCells_ = (NX_ + 2) * (NY_ + 2);
      invNumCells_ = 1.0f / numCells_;

      reset ();
    }

    void setViscosity (float newVisc)
    {
      viscosity_ = newVisc;
    }

    void randomizeColor ()
    {
      for (int i = 0; i < getWidth (); i++)
        for (int j = 0; j < getHeight (); j++)
        {
          int index = FLUID_IX (i, j);
          r_[index] = rOld_[index] = Common_Tools::getRandomNumber (0.0f, 1.0f);
          if (isRGB_)
          {
            g_[index] = gOld_[index] = Common_Tools::getRandomNumber (0.0f, 1.0f);
            b_[index] = bOld_[index] = Common_Tools::getRandomNumber (0.0f, 1.0f);
          } // end IF
        } // end FOR
    }

   private:
    int FLUID_IX (int i, int j) { return ((i) + (NX_ + 2) * (j)); }

   public:
    int getIndexForCellPosition (int i, int j)
    {
      if (i < 1) i = 1;
      else if (i > NX_) i = NX_;
      if (j < 1) j = 1;
      else if (j > NY_) j = NY_;

      return FLUID_IX (i, j);
    }

    int getIndexForNormalizedPosition (float x, float y)
    {
      return getIndexForCellPosition ((int)std::floor (x * (NX_ + 2)),
                                      (int)std::floor (y * (NY_ + 2)));
    }

    int getWidth () { return NX_ + 2; }
    int getHeight () { return NY_ + 2; }

   private:
    void fadeR ()
    {
      //    float holdAmount = 1 - _avgDensity * _avgDensity * _fadeSpeed;  //
      //    this is how fast the density will decay depending on how full the
      //    screen currently is
      float holdAmount = 1.0f - fadeSpeed_;

      avgDensity_ = 0.0f;
      avgSpeed_ = 0.0f;

      float totalDeviations = 0;
      float currentDeviation;
      //  float uniformityMult = uniformity * 0.05f;

      for (int i = 0; i < numCells_; i++)
      {
        // clear old values
        uOld_[i] = vOld_[i] = 0.0f;
        rOld_[i] = 0.0f;
        //    gOld[i] = bOld[i] = 0;

        // calc avg speed
        avgSpeed_ += u_[i] * u_[i] + v_[i] * v_[i];

        // calc avg density
        r_[i] = std::min (1.0f, r_[i]);
        //    g[i] = Math.min(1.0f, g[i]);
        //    b[i] = Math.min(1.0f, b[i]);
        //    float density = Math.max(r[i], Math.max(g[i], b[i]));
        float density = r_[i];
        avgDensity_ += density; // add it up

        // calc deviation (for uniformity)
        currentDeviation = density - avgDensity_;
        totalDeviations += currentDeviation * currentDeviation;

        // fade out old
        r_[i] *= holdAmount;
      } // end FOR
      avgDensity_ *= invNumCells_;
      avgSpeed_ *= invNumCells_;

      uniformity_ = 1.0f / (1.0f + totalDeviations * invNumCells_); // 0: very wide distribution, 1: very uniform
    }

    void fadeRGB ()
    {
//    float holdAmount = 1 - _avgDensity * _avgDensity * _fadeSpeed;  // this is how fast the density will decay depending on how full the screen currently is
      float holdAmount = 1.0f - fadeSpeed_;

      avgDensity_ = 0.0f;
      avgSpeed_ = 0.0f;

      float totalDeviations = 0;
      float currentDeviation;
      //  float uniformityMult = uniformity * 0.05f;

      for (int i = 0; i < numCells_; i++)
      {
        // clear old values
        uOld_[i] = vOld_[i] = 0.0f;
        rOld_[i] = 0.0f;
        gOld_[i] = bOld_[i] = 0.0f;

        // calc avg speed
        avgSpeed_ += u_[i] * u_[i] + v_[i] * v_[i];

        // calc avg density
        r_[i] = std::min (1.0f, r_[i]);
        g_[i] = std::min (1.0f, g_[i]);
        b_[i] = std::min (1.0f, b_[i]);
        float density = std::max (r_[i], std::max (g_[i], b_[i]));
        //float density = r[i];
        avgDensity_ += density;  // add it up

        // calc deviation (for uniformity)
        currentDeviation = density - avgDensity_;
        totalDeviations += currentDeviation * currentDeviation;

        // fade out old
        r_[i] *= holdAmount;
        g_[i] *= holdAmount;
        b_[i] *= holdAmount;
      } // end FOR
      avgDensity_ *= invNumCells_;
      avgSpeed_ *= invNumCells_;

      uniformity_ = 1.0f / (1.0f + totalDeviations * invNumCells_);    // 0: very wide distribution, 1: very uniform
    }

    void addSourceUV ()
    {
      for (int i = 0; i < numCells_; i++)
      {
        u_[i] += dt_ * uOld_[i];
        v_[i] += dt_ * vOld_[i];
      } // end FOR
    }

    void addSourceRGB ()
    {
      for (int i = 0; i < numCells_; i++)
      {
        r_[i] += dt_ * rOld_[i];
        g_[i] += dt_ * gOld_[i];
        b_[i] += dt_ * bOld_[i];
      } // end FOR
    }

    void addSource (float x[], float x0[])
    {
      for (int i = 0; i < numCells_; i++)
        x[i] += dt_ * x0[i];
    }

    void setBoundary (int b, float x[])
    {
      // return;
      for (int i = 1; i <= NX_; i++)
      {
        if (i <= NY_)
        {
          x[FLUID_IX (0, i)] =
            b == 1 ? -x[FLUID_IX (1, i)] : x[FLUID_IX (1, i)];
          x[FLUID_IX (NX_ + 1, i)] =
            b == 1 ? -x[FLUID_IX (NX_, i)] : x[FLUID_IX (NX_, i)];
        } // end IF

        x[FLUID_IX (i, 0)] = b == 2 ? -x[FLUID_IX (i, 1)] : x[FLUID_IX (i, 1)];
        x[FLUID_IX (i, NY_ + 1)] =
          b == 2 ? -x[FLUID_IX (i, NY_)] : x[FLUID_IX (i, NY_)];
      } // end FOR

      x[FLUID_IX (0, 0)] = 0.5f * (x[FLUID_IX (1, 0)] + x[FLUID_IX (0, 1)]);
      x[FLUID_IX (0, NY_ + 1)] =
        0.5f * (x[FLUID_IX (1, NY_ + 1)] + x[FLUID_IX (0, NY_)]);
      x[FLUID_IX (NX_ + 1, 0)] =
        0.5f * (x[FLUID_IX (NX_, 0)] + x[FLUID_IX (NX_ + 1, 1)]);
      x[FLUID_IX (NX_ + 1, NY_ + 1)] =
        0.5f * (x[FLUID_IX (NX_, NY_ + 1)] + x[FLUID_IX (NX_ + 1, NY_)]);
    }

    void setBoundaryRGB (int bound)
    {
      int index1, index2;
      for (int i = 1; i <= NX_; i++)
      {
        if (i <= NY_)
        {
          index1 = FLUID_IX (0, i);
          index2 = FLUID_IX (1, i);
          r_[index1] = bound == 1 ? -r_[index2] : r_[index2];
          g_[index1] = bound == 1 ? -g_[index2] : g_[index2];
          b_[index1] = bound == 1 ? -b_[index2] : b_[index2];

          index1 = FLUID_IX (NX_ + 1, i);
          index2 = FLUID_IX (NX_, i);
          r_[index1] = bound == 1 ? -r_[index2] : r_[index2];
          g_[index1] = bound == 1 ? -g_[index2] : g_[index2];
          b_[index1] = bound == 1 ? -b_[index2] : b_[index2];
        } // end IF

        index1 = FLUID_IX (i, 0);
        index2 = FLUID_IX (i, 1);
        r_[index1] = bound == 2 ? -r_[index2] : r_[index2];
        g_[index1] = bound == 2 ? -g_[index2] : g_[index2];
        b_[index1] = bound == 2 ? -b_[index2] : b_[index2];

        index1 = FLUID_IX (i, NY_ + 1);
        index2 = FLUID_IX (i, NY_);
        r_[index1] = bound == 2 ? -r_[index2] : r_[index2];
        g_[index1] = bound == 2 ? -g_[index2] : g_[index2];
        b_[index1] = bound == 2 ? -b_[index2] : b_[index2];
      } // end FOR
    }

    void advect (int b, float _d[], float d0[], float du[], float dv[])
    {
      int i0, j0, i1, j1;
      float x, y, s0, t0, s1, t1, dt0;

      dt0 = dt_ * NX_;

      for (int i = 1; i <= NX_; i++)
        for (int j = 1; j <= NY_; j++)
        {
          x = i - dt0 * du[FLUID_IX (i, j)];
          y = j - dt0 * dv[FLUID_IX (i, j)];

          if (x > NX_ + 0.5f)
            x = NX_ + 0.5f;
          if (x < 0.5f)
            x = 0.5f;

          i0 = (int)x;
          i1 = i0 + 1;

          if (y > NY_ + 0.5f)
            y = NY_ + 0.5f;
          if (y < 0.5f)
            y = 0.5f;

          j0 = (int)y;
          j1 = j0 + 1;

          s1 = x - i0;
          s0 = 1 - s1;
          t1 = y - j0;
          t0 = 1 - t1;

          _d[FLUID_IX (i, j)] =
            s0 * (t0 * d0[FLUID_IX (i0, j0)] + t1 * d0[FLUID_IX (i0, j1)]) +
            s1 * (t0 * d0[FLUID_IX (i1, j0)] + t1 * d0[FLUID_IX (i1, j1)]);
        } // end FOR
      setBoundary (b, _d);
    }

    void advectRGB (int b, float du[], float dv[])
    {
      int i0, j0, i1, j1;
      float x, y, s0, t0, s1, t1, dt0;

      dt0 = dt_ * NX_;

      for (int i = 1; i <= NX_; i++)
        for (int j = 1; j <= NY_; j++)
        {
          x = i - dt0 * du[FLUID_IX (i, j)];
          y = j - dt0 * dv[FLUID_IX (i, j)];

          if (x > NX_ + 0.5f)
            x = NX_ + 0.5f;
          if (x < 0.5f)
            x = 0.5f;

          i0 = (int)x;
          i1 = i0 + 1;

          if (y > NY_ + 0.5f)
            y = NY_ + 0.5f;
          if (y < 0.5f)
            y = 0.5f;

          j0 = (int)y;
          j1 = j0 + 1;

          s1 = x - i0;
          s0 = 1 - s1;
          t1 = y - j0;
          t0 = 1 - t1;

          r_[FLUID_IX (i, j)] =
            s0 * (t0 * rOld_[FLUID_IX (i0, j0)] + t1 * rOld_[FLUID_IX (i0, j1)]) +
            s1 * (t0 * rOld_[FLUID_IX (i1, j0)] + t1 * rOld_[FLUID_IX (i1, j1)]);
          g_[FLUID_IX (i, j)] =
            s0 * (t0 * gOld_[FLUID_IX (i0, j0)] + t1 * gOld_[FLUID_IX (i0, j1)]) +
            s1 * (t0 * gOld_[FLUID_IX (i1, j0)] + t1 * gOld_[FLUID_IX (i1, j1)]);
          b_[FLUID_IX (i, j)] =
            s0 * (t0 * bOld_[FLUID_IX (i0, j0)] + t1 * bOld_[FLUID_IX (i0, j1)]) +
            s1 * (t0 * bOld_[FLUID_IX (i1, j0)] + t1 * bOld_[FLUID_IX (i1, j1)]);
        } // end FOR
      setBoundaryRGB (b);
    }

    void linearSolver (int b, float x[], float x0[], float a, float c)
    {
      for (int k = 0; k < solverIterations_; k++)
      {
        for (int i = 1; i <= NX_; i++)
          for (int j = 1; j <= NY_; j++)
          {
            x[FLUID_IX (i, j)] =
              (a * (x[FLUID_IX (i - 1, j)] + x[FLUID_IX (i + 1, j)] +
                    x[FLUID_IX (i, j - 1)] + x[FLUID_IX (i, j + 1)]) +
               x0[FLUID_IX (i, j)]) /
              c;
          } // end FOR
        setBoundary (b, x);
      } // end FOR
    }

    // #define LINEAR_SOLVE_EQ  (x, x0)      (a * ( x[] + x[]  +  x[] + x[])  +
    // x0[]) / c;

    void linearSolverRGB (int b, float a, float c)
    {
      int index1, index2, index3, index4, index5;
      for (int k = 0; k < solverIterations_; k++)
      {
        for (int i = 1; i <= NX_; i++)
          for (int j = 1; j <= NY_; j++)
          {
            index5 = FLUID_IX (i, j);
            index1 = index5 - 1;         // FLUID_IX(i-1, j);
            index2 = index5 + 1;         // FLUID_IX(i+1, j);
            index3 = index5 - (NX_ + 2); // FLUID_IX(i, j-1);
            index4 = index5 + (NX_ + 2); // FLUID_IX(i, j+1);

            r_[index5] = (a * (r_[index1] + r_[index2] + r_[index3] + r_[index4]) +
                         rOld_[index5]) /
                        c;
            g_[index5] = (a * (g_[index1] + g_[index2] + g_[index3] + g_[index4]) +
                         gOld_[index5]) /
                        c;
            b_[index5] = (a * (b_[index1] + b_[index2] + b_[index3] + b_[index4]) +
                         bOld_[index5]) /
                        c;
            //        x[FLUID_IX(i, j)] = (a * ( x[FLUID_IX(i-1, j)] +
            //        x[FLUID_IX(i+1, j)]  +  x[FLUID_IX(i, j-1)] +
            //        x[FLUID_IX(i, j+1)])  +  x0[FLUID_IX(i, j)]) / c;
          } // end FOR
        setBoundaryRGB (b);
      } // end FOR
    }

    void linearSolverUV (int b, float a, float c)
    {
      int index1, index2, index3, index4, index5;
      for (int k = 0; k < solverIterations_; k++)
      {
        for (int i = 1; i <= NX_; i++)
          for (int j = 1; j <= NY_; j++)
          {
            index5 = FLUID_IX (i, j);
            index1 = index5 - 1;         // FLUID_IX(i-1, j);
            index2 = index5 + 1;         // FLUID_IX(i+1, j);
            index3 = index5 - (NX_ + 2); // FLUID_IX(i, j-1);
            index4 = index5 + (NX_ + 2); // FLUID_IX(i, j+1);

            u_[index5] = (a * (u_[index1] + u_[index2] + u_[index3] + u_[index4]) +
                         uOld_[index5]) /
                        c;
            v_[index5] = (a * (v_[index1] + v_[index2] + v_[index3] + v_[index4]) +
                         vOld_[index5]) /
                        c;
            //        x[FLUID_IX(i, j)] = (a * ( x[FLUID_IX(i-1, j)] +
            //        x[FLUID_IX(i+1, j)]  +  x[FLUID_IX(i, j-1)] +
            //        x[FLUID_IX(i, j+1)])  +  x0[FLUID_IX(i, j)]) / c;
          } // end FOR
        setBoundaryRGB (b);
      } // end FOR
    }

    void diffuse (int b, float c[], float c0[], float _diff)
    {
      float a = dt_ * _diff * NX_ * NY_;
      linearSolver (b, c, c0, a, 1.0f + 4.0f * a);
    }

    void diffuseRGB (int b, float _diff)
    {
      float a = dt_ * _diff * NX_ * NY_;
      linearSolverRGB (b, a, 1.0f + 4.0f * a);
    }

    void diffuseUV (int b, float _diff)
    {
      float a = dt_ * _diff * NX_ * NY_;
      linearSolverUV (b, a, 1.0f + 4.0f * a);
    }

    void project (float x[], float y[], float p[], float div[])
    {
      for (int i = 1; i <= NX_; i++)
        for (int j = 1; j <= NY_; j++)
        {
          div[FLUID_IX (i, j)] =
            (x[FLUID_IX (i + 1, j)] - x[FLUID_IX (i - 1, j)] +
             y[FLUID_IX (i, j + 1)] - y[FLUID_IX (i, j - 1)]) *
            -0.5f / static_cast<float> (NX_);
          p[FLUID_IX (i, j)] = 0.0f;
        } // end FOR

      setBoundary (0, div);
      setBoundary (0, p);

      linearSolver (0, p, div, 1.0f, 4.0f);

      for (int i = 1; i <= NX_; i++)
        for (int j = 1; j <= NY_; j++)
        {
          x[FLUID_IX (i, j)] -=
            0.5f * NX_ * (p[FLUID_IX (i + 1, j)] - p[FLUID_IX (i - 1, j)]);
          y[FLUID_IX (i, j)] -=
            0.5f * NY_ * (p[FLUID_IX (i, j + 1)] - p[FLUID_IX (i, j - 1)]); // *TODO*: shouldn't this be NY_ ???
        } // end FOR

      setBoundary (1, x);
      setBoundary (2, y);
    }

    void swap (float*& x, float*& xOld)
    {
      float* _tmp = x;
      x = xOld;
      xOld = _tmp;
    }

    void swapRGB ()
    {
      swap (r_, rOld_);
      swap (g_, gOld_);
      swap (b_, bOld_);
    }

   public:
    void update ()
    {
      addSourceUV ();

      swap (u_, uOld_);
      swap (v_, vOld_);

      diffuseUV (0, viscosity_);

      project (u_, v_, uOld_, vOld_);

      swap (u_, uOld_);
      swap (v_, vOld_);

      advect (1, u_, uOld_, uOld_, vOld_);
      advect (2, v_, vOld_, uOld_, vOld_);

      project (u_, v_, uOld_, vOld_);

      if (isRGB_)
      {
        addSourceRGB ();
        swapRGB ();

        diffuseRGB (0, 0);
        swapRGB ();

        advectRGB (0, u_, v_);

        fadeRGB ();
      } // end IF
      else
      {
        addSource (r_, rOld_);
        swap (r_, rOld_);

        diffuse (0, r_, rOld_, 0);
        swap (r_, rOld_);

        advect (0, r_, rOld_, u_, v_);
        fadeR ();
      } // end ELSE
    }

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
  
    float invNumCells_;

    float avgSpeed_;
    float avgDensity_;
    float uniformity_;
  };

  MSAFluidSolver2D solver_;

  void addForce (float x, float y, float dx, float dy, int frameCount)
  {
    float speed =
      dx * dx + dy * dy * aspectRatio2_; // balance the x and y components of
                                         // speed with the screen aspect ratio
    if (speed > 0.0f)
    {
      if (x < 0.0f) x = 0.0f;
      else if (x > 1.0f) x = 1.0f;
      if (y < 0.0f) y = 0.0f;
      else if (y > 1.0f) y = 1.0f;
      int index = solver_.getIndexForNormalizedPosition (x, y);

      float hue = std::fmod ((x + y) * 180.0f + frameCount, 360.0f);
      float r, g, b;
      Common_Image_Tools::HSVToRGB (hue, 1.0f, 1.0f, r, g, b);

      static float colorMult = 5.0f;
      solver_.rOld_[index] += r * colorMult;
      solver_.gOld_[index] += g * colorMult;
      solver_.bOld_[index] += b * colorMult;

      static float velocityMult = 30.0f;
      solver_.uOld_[index] += dx * velocityMult;
      solver_.vOld_[index] += dy * velocityMult;
    } // end IF
  }

  class flow_zone
  {
   public:
    flow_zone (int x, int y, float u, float v)
     : x_ (x)
     , y_ (y)
     , u_ (u)
     , v_ (v)
    {}

    bool productAboveLimit ()
    {
      return ((std::abs (u_) + std::abs (v_)) / 2.0f) > FLUID_DEFAULT_UV_CUTOFF;
    }

    void draw (olc::PixelGameEngine* engine_in)
    {
      engine_in->DrawLine (x_, y_, x_ + static_cast<int32_t> (u_ * 3.0f), y_ + static_cast<int32_t> (v_ * 3.0f),
                           olc::WHITE, 0xFFFFFFFF);
    }

    int   x_;
    int   y_;
    float u_;
    float v_;
  };

  std::vector<flow_zone> calculateFlow (char* oldImage, char* newImage, int width, int height)
  {
    std::vector<flow_zone> zones;

    static int step = 8;
    static int winStep = step * 2 + 1;

    int A2, A1B2, B1, C1, C2;
    float u, v, uu, vv;
    uu = vv = 0.0f;
    int wMax = width - step - 1;
    int hMax = height - step - 1;
    int globalY, globalX, localY, localX;

    for (globalY = step + 1; globalY < hMax; globalY += winStep)
      for (globalX = step + 1; globalX < wMax; globalX += winStep)
      {
        A2 = A1B2 = B1 = C1 = C2 = 0;

        for (localY = -step; localY <= step; localY++)
        {
          for (localX = -step; localX <= step; localX++)
          {
            int address = (globalY + localY) * width + globalX + localX;

            int gradX =
              (newImage[(address - 1) * 3]) - (newImage[(address + 1) * 3]);
            int gradY = (newImage[(address - width) * 3]) -
                        (newImage[(address + width) * 3]);
            int gradT = (oldImage[address * 3]) - (newImage[address * 3]);

            A2 += gradX * gradX;
            A1B2 += gradX * gradY;
            B1 += gradY * gradY;
            C2 += gradX * gradT;
            C1 += gradY * gradT;
          } // end FOR
        } // end FOR

        int delta = (A1B2 * A1B2 - A2 * B1);

        if (delta != 0)
        {
          /* system is not singular - solving by Kramer method */
          float Idelta = step / static_cast<float> (delta);
          int deltaX = -(C1 * A1B2 - C2 * B1);
          int deltaY = -(A1B2 * C2 - A2 * C1);
          u = deltaX * Idelta;
          v = deltaY * Idelta;
        }
        else
        {
          /* singular system - find optical flow in gradient direction */
          int norm = (A1B2 + A2) * (A1B2 + A2) + (B1 + A1B2) * (B1 + A1B2);
          if (norm != 0)
          {
            float IGradNorm = step / static_cast<float> (norm);
            float temp = -(C1 + C2) * IGradNorm;
            u = (A1B2 + A2) * temp;
            v = (B1 + A1B2) * temp;
          }
          else
            u = v = 0.0f;
        } // end ELSE

        if (static_cast<float> (-winStep) < u && u < static_cast<float> (winStep) &&
            static_cast<float> (-winStep) < v && v < static_cast<float> (winStep))
        {
          uu += u;
          vv += v;
          zones.push_back (flow_zone (globalX, globalY, u, v));
        } // end IF
      } // end FOR

    return zones;
  }

  float aspectRatio2_;
};

// include template definition
#include "test_i_module_pge.inl"

#endif // TEST_I_MODULE_PGE_H
