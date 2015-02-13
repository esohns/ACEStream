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

#ifndef STREAM_SESSION_MESSAGE_BASE_H
#define STREAM_SESSION_MESSAGE_BASE_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "common_idumpstate.h"

#include "stream_message_base.h"

enum Stream_SessionMessageType_t
{
  // *NOTE*: see <stream_message_base.h> for details...
  STREAM_SESSION_MAP = MESSAGE_SESSION_MAP,
  // *** STREAM CONTROL ***
  SESSION_BEGIN,
  SESSION_STEP,
  SESSION_END,
  SESSION_STATISTICS
  // *** STREAM CONTROL - END ***
};

// forward declarations
class ACE_Allocator;

template <typename ConfigurationType>
class Stream_SessionMessageBase_T
 : public ACE_Message_Block
 , public Common_IDumpState
{
 public:
  // *NOTE*: assumes lifetime responsibility for the third argument !
  Stream_SessionMessageBase_T (unsigned int,                // session ID
                               Stream_SessionMessageType_t, // session message type
                               ConfigurationType*&);        // handle
  virtual ~Stream_SessionMessageBase_T ();

  // initialization-after-construction
  // *NOTE*: assumes lifetime responsibility for the third argument !
  void init (unsigned int,                // session ID
             Stream_SessionMessageType_t, // session message type
             ConfigurationType*&);        // handle

  // info
  unsigned int getID () const;
  Stream_SessionMessageType_t getType () const;
  // *TODO*: clean this up !
  const ConfigurationType* const getConfiguration () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

  // debug tools
  static void SessionMessageType2String (Stream_SessionMessageType_t, // message type
                                         std::string&);               // corresp. string

 protected:
  // copy ctor to be used by duplicate()
  Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<ConfigurationType>&);

  // *NOTE*: these may be used by message allocators...
  // *WARNING*: these ctors are NOT threadsafe...
  Stream_SessionMessageBase_T (ACE_Allocator*); // message allocator
  Stream_SessionMessageBase_T (ACE_Data_Block*, // data block
                               ACE_Allocator*); // message allocator

  ConfigurationType*          configuration_;
  unsigned int                sessionID_;
  Stream_SessionMessageType_t messageType_;
  bool                        isInitialized_;

 private:
  typedef ACE_Message_Block inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T<ConfigurationType>& operator= (const Stream_SessionMessageBase_T<ConfigurationType>&));

  // overloaded from ACE_Message_Block
  // *WARNING*: any children need to override this too !
  virtual ACE_Message_Block* duplicate (void) const;
};

// include template implementation
#include "stream_session_message_base.inl"

#endif
