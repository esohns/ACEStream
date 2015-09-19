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

#ifndef TEST_I_SOURCE_COMMON_H
#define TEST_I_SOURCE_COMMON_H

#include "test_i_common.h"

struct Test_I_Source_GTK_CBData
 : Stream_GTK_CBData
{
  inline Test_I_Source_GTK_CBData ()
   : Stream_GTK_CBData ()
   , UDPStream (NULL)
  {};

  Test_I_StreamBase_t* UDPStream;
};

struct Test_I_Source_ThreadData
{
  inline Test_I_Source_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
  {};

  Test_I_Source_GTK_CBData* CBData;
  guint                     eventSourceID;
};

#endif
