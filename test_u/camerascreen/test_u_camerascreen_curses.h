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

#ifndef TEST_U_CAMERASCREEN_CURSES_H
#define TEST_U_CAMERASCREEN_CURSES_H

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_ui_curses_common.h"
#include "common_ui_curses_manager.h"

// forward declarations
class Stream_IStreamControlBase;

struct Test_U_CursesState
 : Common_UI_Curses_State
{
  Test_U_CursesState ()
   : Common_UI_Curses_State ()
   , stream (NULL)
  {}

  Stream_IStreamControlBase* stream;
};

typedef Common_UI_Curses_Manager_T<ACE_MT_SYNCH,
                                   struct Common_UI_Curses_Configuration,
                                   struct Test_U_CursesState> Test_U_Curses_Manager_t;
typedef ACE_Singleton<Test_U_Curses_Manager_t,
                      ACE_MT_SYNCH::MUTEX> TEST_U_CURSES_MANAGER_SINGLETON;

//////////////////////////////////////////

// event hooks
bool curses_init (struct Common_UI_Curses_State*); // state
bool curses_fini (struct Common_UI_Curses_State*); // state

bool curses_input (struct Common_UI_Curses_State*, // state
                   int);                           // character
bool curses_main (struct Common_UI_Curses_State*); // state

//////////////////////////////////////////

#endif
