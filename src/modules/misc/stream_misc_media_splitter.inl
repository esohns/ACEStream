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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_lib_common.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Miscellaneous_MediaSplitter_T<ACE_SYNCH_USE,
                                     ConfigurationType,
                                     ControlMessageType,
                                     MessageType,
                                     SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                     SessionDataType>::Stream_Miscellaneous_MediaSplitter_T (ISTREAM_T* stream_in)
#else
                                     SessionDataType>::Stream_Miscellaneous_MediaSplitter_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_MediaSplitter_T::Stream_Miscellaneous_MediaSplitter_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Miscellaneous_MediaSplitter_T<ACE_SYNCH_USE,
                                     ConfigurationType,
                                     ControlMessageType,
                                     MessageType,
                                     SessionMessageType,
                                     SessionDataType>::forward (MessageType* message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Miscellaneous_MediaSplitter_T::forward"));

  // sanity check(s)
  ACE_ASSERT (message_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  std::string branch_name_string;

  // map message type to branch name
  switch (message_in->getMediaType ())
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      branch_name_string =
        ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_PLAYBACK_NAME);
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      branch_name_string = ACE_TEXT_ALWAYS_CHAR (STREAM_SUBSTREAM_DISPLAY_NAME);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown media type (was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_in->getMediaType ()));
      return;
    }
  } // end SWITCH

  ACE_ASSERT (!message_block_p);
  message_block_p = message_in->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    typename inherited::BRANCH_TO_HEAD_CONST_ITERATOR_T iterator =
      inherited::heads_.find (branch_name_string);
    ACE_ASSERT (iterator != inherited::heads_.end ());
    ACE_ASSERT ((*iterator).second);
    typename inherited::QUEUE_TO_MODULE_CONST_ITERATOR_T iterator_2 =
      std::find_if (inherited::modules_.begin (), inherited::modules_.end (),
                    std::bind2nd (typename inherited::QUEUE_TO_MODULE_MAP_FIND_S (),
                                  (*iterator).second));
    ACE_ASSERT (iterator_2 != inherited::modules_.end ());
    ACE_ASSERT ((*iterator_2).first);

    result = (*iterator_2).first->enqueue_tail (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Base::enqueue_tail(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      return;
    } // end IF
  } // end lock scope
}
