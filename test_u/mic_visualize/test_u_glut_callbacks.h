/***************************************************************************
*   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef TEST_U_GLUT_CALLBACKS_H
#define TEST_U_GLUT_CALLBACKS_H

// GLUT routines
void test_u_glut_reshape (int, int);
void test_u_glut_key (unsigned char, int, int);
void test_u_glut_key_special (int, int, int);
void test_u_glut_menu (int);
void test_u_glut_mouse_button (int, int, int, int);
void test_u_glut_mouse_move (int, int);
void test_u_glut_timer (int);
void test_u_glut_draw (void);
void test_u_glut_idle (void);
void test_u_glut_visible (int);

#endif
