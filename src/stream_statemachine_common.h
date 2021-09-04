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

#ifndef STREAM_STATEMACHINE_COMMON_H
#define STREAM_STATEMACHINE_COMMON_H

//enum Stream_StateMachine_ControlState : int
enum Stream_StateMachine_ControlState
{
  STREAM_STATE_INVALID = -1,
  // *NOTE*: currently, this simply means that the 'head' module has been
  //         initialize()d (which doesn't say very much).
  // *TODO*: implementations should support logic to auto-initialize() all
  //         push()ed modules when switching to this state
  STREAM_STATE_INITIALIZED = 0,
  STREAM_STATE_RUNNING,
  STREAM_STATE_PAUSED,
  STREAM_STATE_STOPPED,
  STREAM_STATE_FINISHED,
  /////////////////////////////////////
  STREAM_STATE_MAX
};

#endif
