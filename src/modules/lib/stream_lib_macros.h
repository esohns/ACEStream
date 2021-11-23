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

#ifndef STREAM_LIB_MACROS_H
#define STREAM_LIB_MACROS_H

#include "ace/config-lite.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *WARNING*: !!! use with extreme caution !!!
#define RELEASE_COM_OBJECT(X) do { \
  ULONG count_i = X->Release ();   \
  while (count_i)                  \
    count_i = X->Release ();       \
} while (0)
#endif // ACE_WIN32 || ACE_WIN64

#endif
