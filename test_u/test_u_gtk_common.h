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

#ifndef TEST_U_GTK_COMMON_H
#define TEST_U_GTK_COMMON_H

//#include <deque>

#include "common_ui_gtk_common.h"

// forward declarations
struct Test_U_Configuration;

//enum Test_U_GTK_Event
//{
//  TEST_U_GTKEVENT_INVALID = -1,
//  // -------------------------------------
//  TEST_U_GTKEVENT_START = 0,
//  TEST_U_GTKEVENT_DATA,
//  TEST_U_GTKEVENT_END,
//  TEST_U_GTKEVENT_STATISTIC,
//  // -------------------------------------
//  TEST_U_GTKEVENT_MAX
//};
//typedef std::deque<Test_U_GTK_Event> Test_U_GTK_Events_t;
//typedef Test_U_GTK_Events_t::const_iterator Test_U_GTK_EventsIterator_t;

struct Test_U_GTK_CBData
 : Common_UI_GTKState
{
  Test_U_GTK_CBData ()
   : Common_UI_GTKState ()
   , allowUserRuntimeStatistic (true)
   , configuration (NULL)
   //, contextId (0)
   //, eventStack ()
  {};

  bool                         allowUserRuntimeStatistic;
  struct Test_U_Configuration* configuration;
  //guint                 contextId; // statusbar
  //Test_U_GTK_Events_t   eventStack;
};

//#if defined (GTKGL_SUPPORT)
//struct Test_U_GTKGL_CBData
// : Common_UI_GTKGLState
//{
//  inline Test_U_GTKGL_CBData ()
//   : Common_UI_GTKGLState ()
//   , allowUserRuntimeStatistic (true)
//   , configuration (NULL)
//   , contextId (0)
//   , eventStack ()
//  {};
//
//  bool                  allowUserRuntimeStatistic;
//  Test_U_Configuration* configuration;
//  guint                 contextId; // statusbar
//  Test_U_GTK_Events_t   eventStack;
//};
//#endif

#endif
