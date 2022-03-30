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

#ifndef STREAM_INOTIFY_H
#define STREAM_INOTIFY_H

#include <string>

#include "ace/Notification_Strategy.h"

template <typename NotificationType>
class Stream_IEvent_T
{
 public:
  ////////////////////////////////////////
  virtual void onEvent (NotificationType) = 0;
};

template <typename NotificationType>
class Stream_INotify_T
{
 public:
  // *NOTE*: notify (module) (status) events, may enqeue session message(s)
  virtual void notify (NotificationType,  // session message type
                       bool = false) = 0; // forward upstream ?
};

class Stream_IOutboundDataNotify
 //: public Common_IGetP_T<ACE_Notification_Strategy>
{
 public:
  virtual const ACE_Notification_Strategy* const getP (bool = false) const = 0; // recurse upstream ?
  // set up event dispatch notification for any outbound data reaching the
  // (most upstream) head modules' ('reader'-)task queue
  virtual bool initialize_2 (ACE_Notification_Strategy*,                                         // strategy handle
                             const std::string& = ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head")) = 0; // module name
};

#endif
