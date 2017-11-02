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

#ifndef STREAM_NET_COMMON_H
#define STREAM_NET_COMMON_H

#include <map>
#include <string>

#include "stream_configuration.h"

#include "net_configuration.h"

typedef std::map<std::string, // module name
                 struct Net_ConnectionConfiguration> Stream_Net_StreamConnectionConfigurations_t;
typedef Stream_Net_StreamConnectionConfigurations_t::iterator Stream_Net_StreamConnectionConfigurationIterator_t;
typedef std::map<std::string, // stream name
                 Stream_Net_StreamConnectionConfigurations_t> Stream_Net_ConnectionConfigurations_t;
typedef Stream_Net_ConnectionConfigurations_t::iterator Stream_Net_ConnectionConfigurationIterator_t;

struct Stream_Net_StreamConfiguration
 : Stream_Configuration
{
  Stream_Net_StreamConfiguration ()
   : Stream_Configuration ()
  {
    finishOnDisconnect = true;
  }
};

#endif
