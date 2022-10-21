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
#include "stdafx.h"

#include "test_u_camerascreen_curses_window.h"

#include <ncurses.h>
#include <utility>

#include "ace/Log_Msg.h"

#include "common_image_tools.h"

#include "common_ui_curses_defines.h"
#include "common_ui_curses_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_Curses_Window::Test_U_CameraScreen_Curses_Window (ISTREAM_T* stream_in)
#else
Test_U_CameraScreen_Curses_Window::Test_U_CameraScreen_Curses_Window (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_Curses_Window::Test_U_CameraScreen_Curses_Window"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_CameraScreen_Curses_Window::handleDataMessage (Stream_CameraScreen_DirectShow_Message_t*& message_inout,
#else
Test_U_CameraScreen_Curses_Window::handleDataMessage (Stream_CameraScreen_Message_t*& message_inout,
#endif // ACE_WIN32 || ACE_WIN64
                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_Curses_Window::handleDataMessage"));

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//  return inherited::handleDataMessage (message_inout,
//                                       passMessageDownstream_out);
//#endif // ACE_WIN32 || ACE_WIN64

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->window_2);

  int result;
  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  float r_f, g_f, b_f;
  int fg, bg;
  chtype char_i;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  cchar_t char_2;
  ACE_OS::memset (&char_2, 0, sizeof (cchar_t));
#endif // ACE_WIN32 || ACE_WIN64

  result = wmove (inherited::configuration_->window_2,
                  0, 0);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));
  for (int y = 0;
       y < getmaxy (inherited::configuration_->window_2);
       ++y)
    for (int x = 0;
         x < getmaxx (inherited::configuration_->window_2);
         ++x)
    {
      r_f = *data_p / 255.0F;
      g_f = *(data_p + 1) / 255.0F;
      b_f = *(data_p + 2) / 255.0F;
      //classifyPixelGrey (r_f, g_f, b_f,
      //                   char_i, fg, bg);
      //classifyPixelHSV (r_f, g_f, b_f,
      //                  char_i, fg, bg);
      classifyPixelOLC (r_f, g_f, b_f,
                        char_i, fg, bg);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Common_UI_Curses_Tools::setcolor (fg, bg);
      result = wadd_wch (inherited::configuration_->window_2,
                         &char_i);
      Common_UI_Curses_Tools::unsetcolor (fg, bg);
#else
      char_2.attr =
        (Common_UI_Curses_Tools::is_bold (fg) ? WA_BOLD : WA_NORMAL);
      char_2.chars[0] = static_cast<wchar_t> (char_i & 0xffff);
      char_2.ext_color = Common_UI_Curses_Tools::colornum (fg, bg);
//      wchar_t char_a[] = { char_2.chars[0], 0 };
//      result = setcchar (&char_2, char_a, char_2.attr, char_2.ext_color, NULL);
//      ACE_ASSERT (result == OK);
      result = wadd_wch (inherited::configuration_->window_2,
                         &char_2);
#endif // ACE_WIN32 || ACE_WIN64
           //if (unlikely (result == ERR))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to wadd_wch(), continuing\n")));

      data_p += 3;
    } // end FOR

  result = wrefresh (inherited::configuration_->window_2);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wrefresh(), continuing\n")));
}

void
Test_U_CameraScreen_Curses_Window::classifyPixelGrey (float red_in,
                                                      float green_in,
                                                      float blue_in,
                                                      chtype& symbol_out,
                                                      int& foreGroundColor_out,
                                                      int& backGroundColor_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_Curses_Window::classifyPixelGrey"));

  float luminance = 0.2987f * red_in + 0.5870f * green_in + 0.1140f * blue_in;
  switch ((int)(luminance * 13.0f))
  {
    case 0:  backGroundColor_out = COMMON_UI_CURSES_BLACK;     foreGroundColor_out = COMMON_UI_CURSES_BLACK;     symbol_out = COMMON_UI_CURSES_BLOCK_SOLID; break;

    case 1:  backGroundColor_out = COMMON_UI_CURSES_BLACK;     foreGroundColor_out = COMMON_UI_CURSES_DARK_GREY; symbol_out = COMMON_UI_CURSES_BLOCK_QUARTER; break;
    case 2:  backGroundColor_out = COMMON_UI_CURSES_BLACK;     foreGroundColor_out = COMMON_UI_CURSES_DARK_GREY; symbol_out = COMMON_UI_CURSES_BLOCK_HALF; break;
    case 3:  backGroundColor_out = COMMON_UI_CURSES_BLACK;     foreGroundColor_out = COMMON_UI_CURSES_DARK_GREY; symbol_out = COMMON_UI_CURSES_BLOCK_THREE_QUARTERS; break;
    case 4:  backGroundColor_out = COMMON_UI_CURSES_BLACK;     foreGroundColor_out = COMMON_UI_CURSES_DARK_GREY; symbol_out = COMMON_UI_CURSES_BLOCK_SOLID; break;

    case 5:  backGroundColor_out = COMMON_UI_CURSES_DARK_GREY; foreGroundColor_out = COMMON_UI_CURSES_GREY;      symbol_out = COMMON_UI_CURSES_BLOCK_QUARTER; break;
    case 6:  backGroundColor_out = COMMON_UI_CURSES_DARK_GREY; foreGroundColor_out = COMMON_UI_CURSES_GREY;      symbol_out = COMMON_UI_CURSES_BLOCK_HALF; break;
    case 7:  backGroundColor_out = COMMON_UI_CURSES_DARK_GREY; foreGroundColor_out = COMMON_UI_CURSES_GREY;      symbol_out = COMMON_UI_CURSES_BLOCK_THREE_QUARTERS; break;
    case 8:  backGroundColor_out = COMMON_UI_CURSES_DARK_GREY; foreGroundColor_out = COMMON_UI_CURSES_GREY;      symbol_out = COMMON_UI_CURSES_BLOCK_SOLID; break;

    case 9:  backGroundColor_out = COMMON_UI_CURSES_GREY;      foreGroundColor_out = COMMON_UI_CURSES_WHITE;     symbol_out = COMMON_UI_CURSES_BLOCK_QUARTER; break;
    case 10: backGroundColor_out = COMMON_UI_CURSES_GREY;      foreGroundColor_out = COMMON_UI_CURSES_WHITE;     symbol_out = COMMON_UI_CURSES_BLOCK_HALF; break;
    case 11: backGroundColor_out = COMMON_UI_CURSES_GREY;      foreGroundColor_out = COMMON_UI_CURSES_WHITE;     symbol_out = COMMON_UI_CURSES_BLOCK_THREE_QUARTERS; break;
    case 12: backGroundColor_out = COMMON_UI_CURSES_GREY;      foreGroundColor_out = COMMON_UI_CURSES_WHITE;     symbol_out = COMMON_UI_CURSES_BLOCK_SOLID; break;
  } // end SWITCH
}

void
Test_U_CameraScreen_Curses_Window::classifyPixelHSV (float red_in,
                                                     float green_in,
                                                     float blue_in,
                                                     chtype& symbol_out,
                                                     int& foreGroundColor_out,
                                                     int& backGroundColor_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_Curses_Window::classifyPixelHSV"));

  float hue_f, saturation_f, value_f;
  Common_Image_Tools::RGBToHSV (red_in, green_in, blue_in,
                                hue_f, saturation_f, value_f);

  static const struct { chtype c; ACE_INT32 fg; ACE_INT32 bg; } hues_a[] =
  {
    { COMMON_UI_CURSES_BLOCK_SOLID,          COMMON_UI_CURSES_RED,     COMMON_UI_CURSES_RED},
    { COMMON_UI_CURSES_BLOCK_QUARTER,        COMMON_UI_CURSES_YELLOW,  COMMON_UI_CURSES_RED},
    { COMMON_UI_CURSES_BLOCK_HALF,           COMMON_UI_CURSES_YELLOW,  COMMON_UI_CURSES_RED},
    { COMMON_UI_CURSES_BLOCK_THREE_QUARTERS, COMMON_UI_CURSES_YELLOW,  COMMON_UI_CURSES_RED},

    { COMMON_UI_CURSES_BLOCK_SOLID,          COMMON_UI_CURSES_GREEN,   COMMON_UI_CURSES_YELLOW},
    { COMMON_UI_CURSES_BLOCK_QUARTER,        COMMON_UI_CURSES_GREEN,   COMMON_UI_CURSES_YELLOW},
    { COMMON_UI_CURSES_BLOCK_HALF,           COMMON_UI_CURSES_GREEN,   COMMON_UI_CURSES_YELLOW},
    { COMMON_UI_CURSES_BLOCK_THREE_QUARTERS, COMMON_UI_CURSES_GREEN,   COMMON_UI_CURSES_YELLOW},

    { COMMON_UI_CURSES_BLOCK_SOLID,          COMMON_UI_CURSES_CYAN,    COMMON_UI_CURSES_GREEN},
    { COMMON_UI_CURSES_BLOCK_QUARTER,        COMMON_UI_CURSES_CYAN,    COMMON_UI_CURSES_GREEN},
    { COMMON_UI_CURSES_BLOCK_HALF,           COMMON_UI_CURSES_CYAN,    COMMON_UI_CURSES_GREEN},
    { COMMON_UI_CURSES_BLOCK_THREE_QUARTERS, COMMON_UI_CURSES_CYAN,    COMMON_UI_CURSES_GREEN},

    { COMMON_UI_CURSES_BLOCK_SOLID,          COMMON_UI_CURSES_BLUE,    COMMON_UI_CURSES_CYAN},
    { COMMON_UI_CURSES_BLOCK_QUARTER,        COMMON_UI_CURSES_BLUE,    COMMON_UI_CURSES_CYAN},
    { COMMON_UI_CURSES_BLOCK_HALF,           COMMON_UI_CURSES_BLUE,    COMMON_UI_CURSES_CYAN},
    { COMMON_UI_CURSES_BLOCK_THREE_QUARTERS, COMMON_UI_CURSES_BLUE,    COMMON_UI_CURSES_CYAN},

    { COMMON_UI_CURSES_BLOCK_SOLID,          COMMON_UI_CURSES_MAGENTA, COMMON_UI_CURSES_BLUE},
    { COMMON_UI_CURSES_BLOCK_QUARTER,        COMMON_UI_CURSES_MAGENTA, COMMON_UI_CURSES_BLUE},
    { COMMON_UI_CURSES_BLOCK_HALF,           COMMON_UI_CURSES_MAGENTA, COMMON_UI_CURSES_BLUE},
    { COMMON_UI_CURSES_BLOCK_THREE_QUARTERS, COMMON_UI_CURSES_MAGENTA, COMMON_UI_CURSES_BLUE},

    { COMMON_UI_CURSES_BLOCK_SOLID,          COMMON_UI_CURSES_RED,     COMMON_UI_CURSES_MAGENTA},
    { COMMON_UI_CURSES_BLOCK_QUARTER,        COMMON_UI_CURSES_RED,     COMMON_UI_CURSES_MAGENTA},
    { COMMON_UI_CURSES_BLOCK_HALF,           COMMON_UI_CURSES_RED,     COMMON_UI_CURSES_MAGENTA},
    { COMMON_UI_CURSES_BLOCK_THREE_QUARTERS, COMMON_UI_CURSES_RED,     COMMON_UI_CURSES_MAGENTA}
  };

  int index = (int)((hue_f / 360.0f) * 24.0f);

  if (saturation_f > 0.2f)
  {
    symbol_out = hues_a[index].c;
    foreGroundColor_out = hues_a[index].fg;
    backGroundColor_out = hues_a[index].bg;
  } // end IF
  else
   classifyPixelGrey (red_in, green_in, blue_in,
                      symbol_out, foreGroundColor_out, backGroundColor_out);
}

void
Test_U_CameraScreen_Curses_Window::classifyPixelOLC (float red_in,
                                                     float green_in,
                                                     float blue_in,
                                                     chtype& symbol_out,
                                                     int& foreGroundColor_out,
                                                     int& backGroundColor_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_Curses_Window::classifyPixelOLC"));

  // Is pixel coloured (i.e. RGB values exhibit significant variance)
  float fMean = (red_in + green_in + blue_in) / 3.0f;
  float fRVar = (red_in - fMean) * (red_in - fMean);
  float fGVar = (green_in - fMean) * (green_in - fMean);
  float fBVar = (blue_in - fMean) * (blue_in - fMean);
  float fVariance = fRVar + fGVar + fBVar;

  if (fVariance < 0.0001f)
    return classifyPixelGrey (red_in, green_in, blue_in,
                              symbol_out, foreGroundColor_out, backGroundColor_out);

  // Pixel has colour so get dominant colour
  float y = std::min (red_in, green_in);// (r * g);
  float c = std::min (green_in, blue_in); // (g * b);
  float m = std::min (blue_in, red_in); // (b * r);

  float fMean2 = (y + c + m) / 3.0f;
  float fYVar = (y - fMean2) * (y - fMean2);
  float fCVar = (c - fMean2) * (c - fMean2);
  float fMVar = (m - fMean2) * (m - fMean2);

  float fMaxPrimaryVar = std::max (fRVar, fGVar);
  fMaxPrimaryVar = std::max (fMaxPrimaryVar, fBVar);

  float fMaxSecondaryVar = std::max (fCVar, fYVar);
  fMaxSecondaryVar = std::max (fMaxSecondaryVar, fMVar);

  float fShading = 0.5;

  auto compare = [&](float fV1, float fV2, float fC1, float fC2, short FG_LIGHT, short FG_DARK, short BG_LIGHT, short BG_DARK)
  {
    if (fV1 >= fV2)
    {
      // Primary Is Dominant, Use in foreground
      foreGroundColor_out = fC1 > 0.5f ? FG_LIGHT : FG_DARK;

      if (fV2 < 0.0001f)
      {
        // Secondary is not variant, Use greyscale in background
        if (fC2 >= 0.00f && fC2 < 0.25f) backGroundColor_out = COMMON_UI_CURSES_BLACK;
        if (fC2 >= 0.25f && fC2 < 0.50f) backGroundColor_out = COMMON_UI_CURSES_DARK_GREY;
        if (fC2 >= 0.50f && fC2 < 0.75f) backGroundColor_out = COMMON_UI_CURSES_GREY;
        if (fC2 >= 0.75f && fC2 <= 1.00f) backGroundColor_out = COMMON_UI_CURSES_WHITE;
      }
      else
      {
        // Secondary is varient, Use in background
        backGroundColor_out = fC2 > 0.5f ? BG_LIGHT : BG_DARK;
      }

      // Shade dominant over background (100% -> 0%)
      fShading = ((fC1 - fC2) / 2.0f) + 0.5f;
    }

    if (fShading >= 0.00f && fShading < 0.20f) symbol_out = L' ';
    if (fShading >= 0.20f && fShading < 0.40f) symbol_out = COMMON_UI_CURSES_BLOCK_QUARTER;
    if (fShading >= 0.40f && fShading < 0.60f) symbol_out = COMMON_UI_CURSES_BLOCK_HALF;
    if (fShading >= 0.60f && fShading < 0.80f) symbol_out = COMMON_UI_CURSES_BLOCK_THREE_QUARTERS;
    if (fShading >= 0.80f) symbol_out = COMMON_UI_CURSES_BLOCK_SOLID;
  };

  if (fRVar == fMaxPrimaryVar && fYVar == fMaxSecondaryVar)
    compare (fRVar, fYVar, red_in, y, COMMON_UI_CURSES_BRIGHT_RED, COMMON_UI_CURSES_RED, COMMON_UI_CURSES_BRIGHT_YELLOW, COMMON_UI_CURSES_YELLOW);

  if (fRVar == fMaxPrimaryVar && fMVar == fMaxSecondaryVar)
    compare (fRVar, fMVar, red_in, m, COMMON_UI_CURSES_BRIGHT_RED, COMMON_UI_CURSES_RED, COMMON_UI_CURSES_BRIGHT_MAGENTA, COMMON_UI_CURSES_MAGENTA);

  if (fRVar == fMaxPrimaryVar && fCVar == fMaxSecondaryVar)
    compare (fRVar, fCVar, red_in, c, COMMON_UI_CURSES_BRIGHT_RED, COMMON_UI_CURSES_RED, COMMON_UI_CURSES_BRIGHT_CYAN, COMMON_UI_CURSES_CYAN);

  if (fGVar == fMaxPrimaryVar && fYVar == fMaxSecondaryVar)
    compare (fGVar, fYVar, green_in, y, COMMON_UI_CURSES_BRIGHT_GREEN, COMMON_UI_CURSES_GREEN, COMMON_UI_CURSES_BRIGHT_YELLOW, COMMON_UI_CURSES_YELLOW);

  if (fGVar == fMaxPrimaryVar && fCVar == fMaxSecondaryVar)
    compare (fGVar, fCVar, green_in, c, COMMON_UI_CURSES_BRIGHT_GREEN, COMMON_UI_CURSES_GREEN, COMMON_UI_CURSES_BRIGHT_CYAN, COMMON_UI_CURSES_CYAN);

  if (fGVar == fMaxPrimaryVar && fMVar == fMaxSecondaryVar)
    compare (fGVar, fMVar, green_in, m, COMMON_UI_CURSES_BRIGHT_GREEN, COMMON_UI_CURSES_GREEN, COMMON_UI_CURSES_BRIGHT_MAGENTA, COMMON_UI_CURSES_MAGENTA);

  if (fBVar == fMaxPrimaryVar && fMVar == fMaxSecondaryVar)
    compare (fBVar, fMVar, blue_in, m, COMMON_UI_CURSES_BRIGHT_BLUE, COMMON_UI_CURSES_BLUE, COMMON_UI_CURSES_BRIGHT_MAGENTA, COMMON_UI_CURSES_MAGENTA);

  if (fBVar == fMaxPrimaryVar && fCVar == fMaxSecondaryVar)
    compare (fBVar, fCVar, blue_in, c, COMMON_UI_CURSES_BRIGHT_BLUE, COMMON_UI_CURSES_BLUE, COMMON_UI_CURSES_BRIGHT_CYAN, COMMON_UI_CURSES_CYAN);

  if (fBVar == fMaxPrimaryVar && fYVar == fMaxSecondaryVar)
    compare (fBVar, fYVar, blue_in, y, COMMON_UI_CURSES_BRIGHT_BLUE, COMMON_UI_CURSES_BLUE, COMMON_UI_CURSES_BRIGHT_YELLOW, COMMON_UI_CURSES_YELLOW);
}
