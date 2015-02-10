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

#ifndef RPG_STREAM_ITASK_BASE_H
#define RPG_STREAM_ITASK_BASE_H

// forward declaration(s)
class ACE_Message_Block;
class RPG_Stream_MessageBase;

template <typename SessionMessageType,
          typename ProtocolMessageType>
class RPG_Stream_ITaskBase
{
 public:
  virtual ~RPG_Stream_ITaskBase() {}

  // *NOTE*: pipelined "stream tasks" generally need not worry about the lifecycle of the
  // messages passed to them; any filtering functionality however needs to set the second
  // parameter to false (--> default is "true" !), which makes the task assume lifecycle
  // responsibility for the first argument --> HANDLE WITH CARE !
  virtual void handleDataMessage(ProtocolMessageType*&, // data message handle
                                 bool&) = 0;            // return value: pass this message downstream ?
  virtual void handleSessionMessage(SessionMessageType*&,                 // session message handle
                                    bool&) = 0;                           // return value: pass this message downstream ?
  virtual void handleProcessingError(const ACE_Message_Block* const) = 0; // corresp. message
};

#endif
