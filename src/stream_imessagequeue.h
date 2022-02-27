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

#ifndef STREAM_IMESSAGEQUEUE_H
#define STREAM_IMESSAGEQUEUE_H

// forward declarations
class ACE_Message_Block;
class ACE_Time_Value;

class Stream_IMessageQueue
{
 public:
  // *WARNING*: caller needs to hold inherited::lock_ !
  virtual int enqueue_head_i (ACE_Message_Block*,       // message block handle
                              ACE_Time_Value* = 0) = 0; // timeout [NULL: block]
   
   // *NOTE*: returns #flushed messages
  virtual unsigned int flush (bool = false) = 0; // flush session messages ?

  // *NOTE*: resets the 'shutting down' state
  virtual void reset () = 0;

  virtual bool isShuttingDown () const = 0; // MB_STOP has been dequeued ?
  virtual void waitForIdleState () const = 0;
};

#endif
