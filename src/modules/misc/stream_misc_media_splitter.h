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

#ifndef STREAM_MISC_MEDIA_SPLITTER_H
#define STREAM_MISC_MEDIA_SPLITTER_H

#include "ace/Global_Macros.h"
#include "ace/Module.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_distributor.h"

extern const char libacestream_default_misc_media_splitter_module_name_string[];

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType> // reference counted-
class Stream_Miscellaneous_MediaSplitter_T
 : public Stream_Miscellaneous_Distributor_WriterTask_T<ACE_SYNCH_USE,
                                                        Common_TimePolicy_t,
                                                        ConfigurationType,
                                                        ControlMessageType,
                                                        MessageType,
                                                        SessionMessageType,
                                                        SessionDataType>
{
  typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_SYNCH_USE,
                                                        Common_TimePolicy_t,
                                                        ConfigurationType,
                                                        ControlMessageType,
                                                        MessageType,
                                                        SessionMessageType,
                                                        SessionDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Miscellaneous_MediaSplitter_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Miscellaneous_MediaSplitter_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  inline virtual ~Stream_Miscellaneous_MediaSplitter_T () {}

  inline virtual void handleDataMessage (MessageType*& message_inout, bool&) { forward (message_inout); }

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_MediaSplitter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_MediaSplitter_T (const Stream_Miscellaneous_MediaSplitter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_MediaSplitter_T& operator= (const Stream_Miscellaneous_MediaSplitter_T&))

  // helper methods
  void forward (MessageType*); // message handle
};

// include template definition
#include "stream_misc_media_splitter.inl"

#endif
