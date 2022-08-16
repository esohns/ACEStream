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

#ifndef TEST_I_HTTP_GET_COMMON_H
#define TEST_I_HTTP_GET_COMMON_H

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_configuration.h"

#include "test_i_configuration.h"
#include "test_i_connection_common.h"

#include "test_i_http_get_stream_common.h"
#include "test_i_http_get_network.h"

struct Test_I_HTTPGet_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_I_HTTPGet_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   //messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

  //Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Test_I_HTTPGet_Configuration
 : Test_I_Configuration
{
  Test_I_HTTPGet_Configuration ()
   : Test_I_Configuration ()
   , parserConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfiguration ()
  {}

  // **************************** parser data **********************************
  struct HTTP_ParserConfiguration                  parserConfiguration;
  // **************************** signal data **********************************
  struct Test_I_HTTPGet_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                   connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_StreamConfiguration_t                     streamConfiguration;
};

#endif
