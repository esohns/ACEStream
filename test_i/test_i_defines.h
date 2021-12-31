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

#ifndef TEST_I_DEFINES_H
#define TEST_I_DEFINES_H

#include "ace/config-lite.h"

#define TEST_I_STREAM_THREAD_NAME                      "stream processor"

#define TEST_I_DEFAULT_BUFFER_SIZE                     65536 // bytes
#define TEST_I_MAX_MESSAGES                            0 // 0 --> no limits

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS   1
#else
// *IMPORTANT NOTE*: on Linux, specifying 1 will not work correctly for proactor
//                   scenarios with the default (rt signal) proactor. The thread
//                   blocked in sigwaitinfo (see man pages) will not awaken when
//                   the dispatch set is changed (*TODO*: to be verified)
#define TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS   2
#endif // ACE_WIN32 || ACE_WIN64

//---------------------------------------

#if defined (GUI_SUPPORT)
#define TEST_I_UI_CSS_FILE                             "test_i.css"
#if defined (GTK_USE)
#if defined (GTK3_USE)
#define TEST_I_UI_DEFINITION_FILE                      "test_i.gtk3"
#elif defined (GTK2_USE)
#define TEST_I_UI_DEFINITION_FILE                      "test_i.gtk2"
#endif // GTK3_USE || GTK2_USE
#elif defined (QT_USE)
#define TEST_I_UI_DEFINITION_FILE                      "mainwindow.ui"
#elif defined (WXWIDGETS_USE)
#define TEST_I_UI_DEFINITION_FILE                      "test_i.xrc"
#else
#define TEST_I_UI_DEFINITION_FILE                      ""
#endif // GTK_USE || QT_USE || WXWIDGETS_USE

#if defined (WXWIDGETS_SUPPORT)
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME       "dialog_main"
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME "wxDialog"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#endif
