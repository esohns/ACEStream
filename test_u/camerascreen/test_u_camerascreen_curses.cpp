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

#include "test_u_camerascreen_curses.h"

#include <clocale>
#include <string>

#include "ace/Assert.h"
#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#include "common_ui_curses_tools.h"

#include "stream_macros.h"

#include "test_u_camerascreen_common.h"
#include "test_u_camerascreen_defines.h"

bool
curses_input (struct Common_UI_Curses_State* state_in,
              int inputCharacter_in)
{
  STREAM_TRACE (ACE_TEXT ("::curses_input"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct Test_U_CursesState* state_p =
    static_cast<struct Test_U_CursesState*> (state_in);
  ACE_ASSERT (state_p->stream);

  switch (inputCharacter_in)
  {
    case 27: // ESC
    {
      state_p->stream->stop (false, // wait ?
                             true,  // recurse ?
                             true); // high priority ?

      // shutdown
      TEST_U_CURSES_MANAGER_SINGLETON::instance ()->stop (false,  // wait ?
                                                          false); // N/A
      break;
    }
    default:
      break;
  } // end SWITCH

  return true;
}

bool
curses_init (struct Common_UI_Curses_State* state_in)
{
  STREAM_TRACE (ACE_TEXT ("::curses_init"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct Test_U_CursesState* state_p =
    static_cast<struct Test_U_CursesState*> (state_in);
  ACE_ASSERT (!state_p->screen);
  ACE_ASSERT (!state_p->std_window);
//  const struct Common_UI_Curses_Configuration& configuration_r =
//    TEST_U_CURSES_MANAGER_SINGLETON::instance ()->getR_2 ();

  int result = ERR;
  char* string_p = NULL;
  mmask_t mouse_mask = 0;

  setlocale (LC_ALL, "en_US.UTF-8");

  // lock state
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_p->lock, false);

  use_env (TRUE);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  use_tioctl (TRUE);
#endif // ACE_WIN32 || ACE_WIN64
//  nofilter ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: if this fails, the program exits, which is not intended behavior
  // *TODO*: --> use newterm() instead
  initscr ();
#else
  state_p->screen = newterm (NULL, NULL, NULL); // use $TERM, STD_OUT, STD_IN
  if (unlikely (!state_p->screen))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to newterm(0x%@), continuing\n"),
                NULL));
//    result = ERR;
//    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  // *NOTE*: for some (odd) reason, newterm does not work as advertised
  //         (curscr, stdscr corrupt, return value works though)
  state_p->std_window = stdscr;
  if (unlikely (!state_p->std_window))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initscr(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  state_p->screen = SP;
#endif // ACE_WIN32 || ACE_WIN64
//  ACE_ASSERT (state_p->screen && state_p->std_window);
  ACE_ASSERT (state_p->std_window);

//  result = resizeterm (configuration_r.height, // lines
//                       configuration_r.width); // columns
//  if (unlikely (result == ERR))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to resizeterm(%d,%d), aborting\n"),
//                configuration_r.height, configuration_r.width));
//    goto error;
//  } // end IF

  string_p = longname ();
  if (unlikely (!string_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to longname(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("initialized curses terminal (\"%s\")\n"),
              ACE_TEXT (string_p)));

  if (has_colors ())
  {
    result = start_color (); // intialize colors
    if (unlikely (result == ERR))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start_color(), aborting\n")));
      goto error;
    } // end IF

    Common_UI_Curses_Tools::init_colorpairs ();
  } // end IF

  result = curs_set (COMMON_UI_CURSES_CURSOR_MODE_INVISIBLE); // cursor mode
  if (unlikely (result == ERR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to curs_set(%d), aborting\n"),
                COMMON_UI_CURSES_CURSOR_MODE_INVISIBLE));
    goto error;
  } // end IF
  result = nonl ();
  if (unlikely (result == ERR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nonl(), aborting\n")));
    goto error;
  } // end IF

  // step2a: initialize input
  result = noecho (); // disable local echo
  if (unlikely (result == ERR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to noecho(), aborting\n")));
    goto error;
  } // end IF
  result = raw (); // disable line buffering, special character processing
  if (unlikely (result == ERR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to raw(), aborting\n")));
    goto error;
  } // end IF
  result = keypad (state_p->std_window, TRUE); // enable function/arrow/... keys
  if (unlikely (result == ERR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to keypad(), aborting\n")));
    goto error;
  } // end IF
  result = meta (state_p->std_window, TRUE); // 8-bit characters
  if (unlikely (result == ERR))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to meta(), aborting\n")));
    goto error;
  } // end IF
  mouse_mask = mousemask (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  if (unlikely (mouse_mask == 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to mousemask(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF

  return true;

error:
  result = endwin ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to endwin(), continuing\n")));
  result = refresh (); // restore terminal
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to refresh(), continuing\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  delscreen (state_p->screen); state_p->screen = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  return false;
}

bool
curses_fini (struct Common_UI_Curses_State* state_in)
{
  STREAM_TRACE (ACE_TEXT ("::curses_init"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct Test_U_CursesState* state_p =
    static_cast<struct Test_U_CursesState*> (state_in);

  int result = ERR;

  // lock state
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_p->lock, false);

  result = endwin ();
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to endwin(), continuing\n")));
  result = refresh (); // restore terminal
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to refresh(), continuing\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  delscreen (state_p->screen); state_p->screen = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}

bool
curses_main (struct Common_UI_Curses_State* state_in)
{
  STREAM_TRACE (ACE_TEXT ("::curses_main"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct Test_U_CursesState* state_p =
    static_cast<struct Test_U_CursesState*> (state_in);

  int result = ERR;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_p->lock,
                                                  ACE_Acquire_Method::ACE_REGULAR);

  // step3b: handle input
  while (true)
  {
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_p->lock, false);

    // step3ba: done ?
    if (state_p->finished)
    {
      result = OK;
      break; // done
    } // end IF

    // step3bc: get user input
    { // *IMPORTANT NOTE*: release lock while waiting for input
      ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock, false);
      if (!curses_input (state_in,
                         wgetch (state_p->std_window)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::curses_input(%d), aborting\n")));
        result = ERR;
        goto clean;
      } // end IF
    } // end lock scope
  } // end WHILE

clean:
  if (result == ERR)
  {
    // shutdown
    TEST_U_CURSES_MANAGER_SINGLETON::instance ()->stop (false,  // wait ?
                                                        false); // N/A
  } // end IF

  return (result == OK);
}
