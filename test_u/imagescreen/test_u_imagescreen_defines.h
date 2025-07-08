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

#ifndef TEST_U_IMAGESCREEN_DEFINES_H
#define TEST_U_IMAGESCREEN_DEFINES_H

#if defined (GTK_USE)
#if defined (GTK3_USE)
#define TEST_U_UI_DEFINITION_FILE              "imagescreen.gtk3"
#elif defined (GTK2_USE)
#define TEST_U_UI_DEFINITION_FILE              "imagescreen.gtk2"
#endif
#elif defined (QT_USE)
#define TEST_U_UI_DEFINITION_FILE              "imagescreen.ui"
#elif defined (WXWIDGETS_USE)
#define TEST_U_UI_DEFINITION_FILE              "imagescreen.xrc"
#else
#define TEST_U_UI_DEFINITION_FILE              ""
#endif
#define TEST_U_UI_CSS_FILE                     "imagescreen.css"

//---------------------------------------

#if defined (GTK_SUPPORT)
#define TEST_U_UI_GTK_BUTTON_ABOUT_NAME                "about"
#define TEST_U_UI_GTK_BUTTON_QUIT_NAME                 "quit"
#define TEST_U_UI_GTK_COMBOBOX_ADAPTER_NAME            "combobox_adapter"
#define TEST_U_UI_GTK_COMBOBOX_DISPLAY_NAME            "combobox_display"
#define TEST_U_UI_GTK_DIALOG_ABOUT_NAME                "dialog_about"
#define TEST_U_UI_GTK_DIALOG_MAIN_NAME                 "dialog_main"
#define TEST_U_UI_GTK_DRAWINGAREA_NAME                 "drawingarea"
#define TEST_U_UI_GTK_FILECHOOSERBUTTON_DIRECTORY_NAME "filechooserbutton"
#define TEST_U_UI_GTK_LISTSTORE_ADAPTER_NAME           "liststore_adapter"
#define TEST_U_UI_GTK_LISTSTORE_DISPLAY_NAME           "liststore_display"
#define TEST_U_UI_GTK_PROGRESSBAR_NAME                 "progressbar"
#define TEST_U_UI_GTK_STATUSBAR_NAME                   "statusbar"
#define TEST_U_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME     "togglebutton_fullscreen"
#define TEST_U_UI_GTK_TOGGLEBUTTON_PLAY_NAME           "togglebutton_start"
#define TEST_U_UI_GTK_WINDOW_FULLSCREEN                "window_fullscreen"
#endif // GTK_SUPPORT

#if defined (WXWIDGETS_SUPPORT)
#define TEST_U_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME       "dialog_main"
#define TEST_U_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME "wxDialog"
#endif // WXWIDGETS_SUPPORT


#endif
