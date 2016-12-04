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

#ifndef TEST_I_CONNECTION_COMMON_H
#define TEST_I_CONNECTION_COMMON_H

#include <ace/INET_Addr.h>

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnector.h"

#include "test_i_common.h"

struct Test_I_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Test_I_UserData* userData;
};

struct Test_I_StreamConfiguration;
struct Test_I_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline Test_I_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , socketHandlerConfiguration (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct Test_I_StreamConfiguration*        streamConfiguration;

  struct Test_I_UserData*                   userData;
};

struct Test_I_ConnectionState
 : Net_ConnectionState
{
  inline Test_I_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , currentStatistic ()
   , userData (NULL)
  {};

  struct Test_I_ConnectionConfiguration* configuration;

  Test_I_RuntimeStatistic_t              currentStatistic;

  struct Test_I_UserData*                userData;
};

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_I_SocketHandlerConfiguration> Test_I_IInetConnector_t;

/////////////////////////////////////////

#endif
