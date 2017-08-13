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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Module_Aggregator_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionIdType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           SessionDataType>::Stream_Module_Aggregator_T (ISTREAM_T* stream_in)
#else
                           SessionDataType>::Stream_Module_Aggregator_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , lock_ ()
 , modules_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_T::Stream_Module_Aggregator_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
int
Stream_Module_Aggregator_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionIdType,
                           SessionDataType>::put (ACE_Message_Block* messageBlock_in,
                                                  ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_T::put"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  // step1: make a shallow copy of the message ?
  ACE_Message_Block* message_block_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, -1);
    if (!modules_.empty ())
    {
      message_block_p = messageBlock_in->duplicate ();
      if (!message_block_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        return -1;
      } // end IF
    } // end IF
  } // end lock scope

  // step2: handle message
  bool stop_processing = false;
  inherited::handleMessage (messageBlock_in,
                            stop_processing);
  if (stop_processing)
  {
    // clean up
    if (message_block_p)
      message_block_p->release ();

    return 0;
  } // end IF

  // step2: forward message to any downstream modules
  TASK_T* task_p = NULL;
  int result = -1;
  ACE_Message_Block* message_block_2 = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, -1);
    for (MODULES_ITERATOR_T iterator = modules_.begin ();
         iterator != modules_.end ();
         ++iterator)
    { ACE_ASSERT ((*iterator).first);
      task_p = (*iterator).first->writer ();
      ACE_ASSERT (task_p);

      ACE_ASSERT (message_block_p);
      message_block_2 = message_block_p->duplicate ();
      if (!message_block_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

        // clean up
        message_block_p->release ();

        return -1;
      } // end IF

      result = task_p->put (message_block_2,
                            timeValue_in);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put() (module was: \"%s\"): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    (*iterator).first->name ()));

        // clean up
        message_block_2->release ();
      } // end IF
    } // end FOR
  } // end lock scope

  // clean up
  if (message_block_p)
    message_block_p->release ();

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
bool
Stream_Module_Aggregator_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionIdType,
                           SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_T::initialize"));

  if (inherited::isInitialized_)
  {
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, false);
      modules_.clear ();
    } // end lock scope
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_Aggregator_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionIdType,
                           SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::mod_);
      //MODULE_T* module_p = inherited::mod_->next ();
      //ACE_ASSERT (module_p);
      //MODULES_ITERATOR_T iterator = modules_.end ();
      //{ ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
      //  iterator = modules_.find (module_p);
      //  ACE_ASSERT (iterator != modules_.end ());
      //} // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::mod_);

      //MODULE_T* module_p = inherited::mod_->next ();
      //ACE_ASSERT (module_p);
      //MODULES_ITERATOR_T iterator = modules_.end ();
      //{ ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
      //  iterator = modules_.find (module_p);
      //  ACE_ASSERT (iterator != modules_.end ());
      //  modules_.erase (iterator);
      //} // end lock scope

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_Aggregator_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionIdType,
                           SessionDataType>::onLink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_T::onLink"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::stream_);

  // step1: find upstream module
  STREAM_T* stream_p = dynamic_cast<STREAM_T*> (inherited::stream_);
  ACE_ASSERT (stream_p);
  const MODULE_T* module_p = NULL;
  for (STREAM_ITERATOR_T iterator (*stream_p);
       iterator.next (module_p);
       iterator.advance ())
  { ACE_ASSERT (const_cast<MODULE_T*> (module_p)->next ());
    if (!ACE_OS::strcmp (const_cast<MODULE_T*> (module_p)->next ()->name (),
                         inherited::mod_->name ()))
      break;
  } // end FOR
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: could not find upstream module, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // step2: add map entry
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
    modules_.insert (std::make_pair (inherited::mod_->next (),
                                     const_cast<MODULE_T*> (module_p)));
  } // end lock scope
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: linked (%s --> x --> %s)\n"),
              inherited::mod_->name (),
              module_p->name (),
              inherited::mod_->next ()->name ()));

  // step3: reset 'next' module to the tail
  module_p = stream_p->tail ();
  inherited::mod_->link (const_cast<MODULE_T*> (module_p));
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Module_Aggregator_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionIdType,
                           SessionDataType>::onUnlink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_T::onUnlink"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  // remove map entry
  MODULE_T* module_p = inherited::mod_->next ();
  ACE_ASSERT (module_p);
  MODULES_ITERATOR_T iterator = modules_.end ();
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
    iterator = modules_.find (module_p);
    ACE_ASSERT (iterator != modules_.end ());
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: unlinked (%s --> x --> %s)\n"),
                inherited::mod_->name (),
                (*iterator).second->name (),
                inherited::mod_->next ()->name ()));
    modules_.erase (iterator);
  } // end lock scope
}
