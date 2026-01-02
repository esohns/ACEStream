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

#include "test_u_camerascreen_module_curses.h"

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

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::window_);

  int result;
  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  float r_f, g_f, b_f;
  ACE_UINT8 fg, bg;
  chtype char_i;
  attr_t char_attr_i;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  cchar_t char_2;
  ACE_OS::memset (&char_2, 0, sizeof (cchar_t));
  wchar_t char_a[2] = { L'\0', L'\0' };
#endif // ACE_WIN32 || ACE_WIN64

  result = wmove (inherited::window_,
                  0, 0);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));
  for (int y = 0;
       y < getmaxy (inherited::window_);
       ++y)
    for (int x = 0;
         x < getmaxx (inherited::window_);
         ++x)
    { // memory layout is BGR24
      b_f = *data_p / 255.0F;
      g_f = *(data_p + 1) / 255.0F;
      r_f = *(data_p + 2) / 255.0F;

      fg = 0;
      bg = 0;
      char_i = L' ';
      //classifyPixelGrey (r_f, g_f, b_f,
      //                   char_i, fg, bg);
      //classifyPixelHSV (r_f, g_f, b_f,
      //                  char_i, fg, bg);
      classifyPixelOLC (r_f, g_f, b_f,
                        char_i, fg, bg);

      char_attr_i =
        (Common_UI_Curses_Tools::is_bold (fg)/* || Common_UI_Curses_Tools::is_bold (bg)*/ ? WA_BOLD : WA_NORMAL);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      char_attr_i |= COLOR_PAIR (Common_UI_Curses_Tools::colornum (fg, bg));
      char_i |= char_attr_i;
      result = wadd_wch (inherited::window_,
                         &char_i);
#else
      char_a[0] = char_i; // L'\u2665'
      result = setcchar (&char_2,
                         char_a,
                         char_attr_i,
                         Common_UI_Curses_Tools::colornum (fg, bg),
                         NULL);
      ACE_ASSERT (result == 0/*OK*/);
      result = wadd_wch (inherited::window_,
                         &char_2);
#endif // ACE_WIN32 || ACE_WIN64
      // if (unlikely (result == ERR)) // most likely reason: scrollok() NOT enabled and in last position
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to wadd_wch(), continuing\n")));

      data_p += 3;
    } // end FOR

  result = wrefresh (inherited::window_);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wrefresh(), continuing\n")));
}

void
Test_U_CameraScreen_Curses_Window::classifyPixelGrey (float red_in,
                                                      float green_in,
                                                      float blue_in,
                                                      chtype& symbol_out,
                                                      ACE_UINT8& foreGroundColor_out,
                                                      ACE_UINT8& backGroundColor_out)
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

    case 13: backGroundColor_out = COMMON_UI_CURSES_WHITE;     foreGroundColor_out = COMMON_UI_CURSES_WHITE;     symbol_out = COMMON_UI_CURSES_BLOCK_SOLID; break;
  } // end SWITCH
}

void
Test_U_CameraScreen_Curses_Window::classifyPixelHSV (float red_in,
                                                     float green_in,
                                                     float blue_in,
                                                     chtype& symbol_out,
                                                     ACE_UINT8& foreGroundColor_out,
                                                     ACE_UINT8& backGroundColor_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_CameraScreen_Curses_Window::classifyPixelHSV"));

  float hue_f, saturation_f, value_f;
  Common_Image_Tools::RGBToHSV (red_in, green_in, blue_in,
                                hue_f, saturation_f, value_f);

  static const struct { chtype c; ACE_UINT8 fg; ACE_UINT8 bg; } hues_a[] =
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
  int index = static_cast<int> ((hue_f / 360.0f) * 23.0f);

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
                                                     ACE_UINT8& foreGroundColor_out,
                                                     ACE_UINT8& backGroundColor_out)
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

  float fShading = 0.5f;

  auto compare = [&](float fV1, float fV2, float fC1, float fC2,
                     ACE_UINT8 FG_LIGHT, ACE_UINT8 FG_DARK, ACE_UINT8 BG_LIGHT, ACE_UINT8 BG_DARK)
  {
    if (fV1 >= fV2)
    {
      // Primary Is Dominant, Use in foreground
      foreGroundColor_out = fC1 > 0.5f ? FG_LIGHT : FG_DARK;

      if (fV2 < 0.0001f)
      {
        // Secondary is not variant, Use greyscale in background
        if (fC2 >= 0.00f && fC2 < 0.25f)
          backGroundColor_out = COMMON_UI_CURSES_BLACK;
        else if (fC2 >= 0.25f && fC2 < 0.50f)
          backGroundColor_out = COMMON_UI_CURSES_DARK_GREY;
        else if (fC2 >= 0.50f && fC2 < 0.75f)
          backGroundColor_out = COMMON_UI_CURSES_GREY;
        else if (fC2 >= 0.75f && fC2 <= 1.00f)
          backGroundColor_out = COMMON_UI_CURSES_WHITE;
      } // end IF
      else
      {
        // Secondary is variant, Use in background
        backGroundColor_out = fC2 > 0.5f ? BG_LIGHT : BG_DARK;
      } // end ELSE

      // Shade dominant over background (100% -> 0%)
      fShading = ((fC1 - fC2) / 2.0f) + 0.5f;
    } // end IF

    if (fShading >= 0.0f && fShading < 0.2f)
      symbol_out = L' ';
    else if (fShading >= 0.2f && fShading < 0.4f)
      symbol_out = COMMON_UI_CURSES_BLOCK_QUARTER;
    else if (fShading >= 0.4f && fShading < 0.6f)
      symbol_out = COMMON_UI_CURSES_BLOCK_HALF;
    else if (fShading >= 0.6f && fShading < 0.8f)
      symbol_out = COMMON_UI_CURSES_BLOCK_THREE_QUARTERS;
    else if (fShading >= 0.8f)
      symbol_out = COMMON_UI_CURSES_BLOCK_SOLID;
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
