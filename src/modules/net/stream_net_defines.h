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

#ifndef STREAM_NET_DEFINES_H
#define STREAM_NET_DEFINES_H

// module
#define MODULE_NET_IO_DEFAULT_NAME_STRING       "NetIO"
#define MODULE_NET_LISTENER_DEFAULT_NAME_STRING "NetListener"
// *NOTE*: marshalling modules usually have reader and (!) writer tasks that
//         'funnel' PDUs over the network, i.e. serialize the data (reader-side)
//         and parse the inbound data into data record (object-)instances
//         (writer-side). These are protocol/implementation-specific
#define MODULE_NET_MARSHAL_DEFAULT_NAME_STRING  "NetMarshal"
#define MODULE_NET_SOURCE_DEFAULT_NAME_STRING   "NetSource"
#define MODULE_NET_TARGET_DEFAULT_NAME_STRING   "NetTarget"

// stream
#define STREAM_NET_IO_DEFAULT_NAME_STRING       "NetworkIOStream"
#define STREAM_NET_SERVER_DEFAULT_NAME_STRING   "NetworkServerStream"

#endif
