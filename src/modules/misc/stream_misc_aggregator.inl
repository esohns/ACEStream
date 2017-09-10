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
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                      SessionDataType>::Stream_Module_Aggregator_WriterTask_T (ISTREAM_T* stream_in)
#else
                                      SessionDataType>::Stream_Module_Aggregator_WriterTask_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , lock_ ()
 , readerLinks_ ()
 , writerLinks_ ()
 , sessionData_ ()
 //, stream_ ()
 , outboundStreamName_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::Stream_Module_Aggregator_WriterTask_T"));

  inherited::aggregate_ = true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::~Stream_Module_Aggregator_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::~Stream_Module_Aggregator_WriterTask_T"));

  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
    for (SESSION_DATA_ITERATOR_T iterator = sessionData_.begin ();
         iterator != sessionData_.end ();
         ++iterator)
      (*iterator).second->decrease ();
  } // end lock scope
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
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::put (ACE_Message_Block* messageBlock_in,
                                                             ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::put"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  // step1: make a shallow copy of the message ?
  ACE_Message_Block* message_block_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, -1);
    if (!writerLinks_.empty ())
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

  // step2: forward message to all downstream modules
  TASK_T* task_p = NULL;
  int result = -1;
  ACE_Message_Block* message_block_2 = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, -1);
    for (LINKS_ITERATOR_T iterator = writerLinks_.begin ();
         iterator != writerLinks_.end ();
         ++iterator)
    { ACE_ASSERT ((*iterator).second);
      task_p = (*iterator).second->writer ();
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
                    ACE_TEXT ("%s: failed to ACE_Task::put(): \"%m\", continuing\n"),
                    (*iterator).second->name ()));

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
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                                    Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::initialize"));

  if (inherited::isInitialized_)
  {
    //{ ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, -1);
      //for (SESSION_DATA_ITERATOR_T iterator = sessionData_.begin ();
      //     iterator != sessionData_.end ();
      //     ++iterator)
      //  (*iterator).second->decrease ();
      //sessionData_.clear ();

    //// prevent duplicates
    //  STREAMS_ITERATOR_T iterator = stream_.begin ();
    //  for (;
    //       iterator != stream_.end ();
    //       ++iterator)
    //    if (*iterator == configuration_in.stream)
    //      break;
    //  if (iterator != stream_.end ())
    //    stream_.erase (iterator);
    //} // end lock scope

    outboundStreamName_.clear ();
  } // end IF

  //{ ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, -1);
  //  stream_.push_back (configuration_in.stream);
  //} // end lock scope

  outboundStreamName_ = configuration_in.outboundStreamName;

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
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    case STREAM_SESSION_MESSAGE_LINK:
    { ACE_ASSERT (inherited::sessionData_);
      inherited::sessionData_->increase ();

      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->get ();

      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
        sessionData_.insert (std::make_pair (session_data_r.sessionId,
                                             inherited::sessionData_));
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    case STREAM_SESSION_MESSAGE_END:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
        message_inout->get ();
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        session_data_container_r.get ();

      SESSION_DATA_ITERATOR_T iterator;
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
        iterator = sessionData_.find (session_data_r.sessionId);
        if (iterator != sessionData_.end ())
        {
          (*iterator).second->decrease ();
          sessionData_.erase (iterator);
        } // end IF
      } // end lock scope

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
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::onLink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::onLink"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::stream_);

  STREAM_T* stream_p = dynamic_cast<STREAM_T*> (inherited::stream_);
  ACE_ASSERT (stream_p);

  // step1: find upstream module
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
  MODULE_T* module_2 = inherited::mod_->next ();
  ACE_ASSERT (module_2);
  bool next_is_tail = !ACE_OS::strcmp (module_2->name (),
                                       ACE_TEXT ("ACE_Stream_Tail"));
  if ((!ACE_OS::strcmp (module_p->name (),
                        ACE_TEXT ("ACE_Stream_Head")) ||
       !ACE_OS::strcmp (module_p->name (),
                        ACE_TEXT (STREAM_MODULE_HEAD_NAME))) &&
      next_is_tail)
    return; // module is being push()ed onto the stream --> nothing to do

  // step2: add map entry ?
  if (!next_is_tail)
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
    readerLinks_.insert (std::make_pair (inherited::stream_->name (),
                                         const_cast<MODULE_T*> (module_p)));
    writerLinks_.insert (std::make_pair (inherited::stream_->name (),
                                         module_2));
  } // end lock scope
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: linked (%s --> x --> %s)\n"),
  //            inherited::mod_->name (),
  //            module_p->name (),
  //            module_2->name ()));

  // step3: reset 'next' module to the stream tail, iff necessary
  // *NOTE*: this test avoids infinite recursion, but is 'flaky' in that the
  //         module ends up not being linked to the 'correct' tail module (i.e.
  //         the instance of the 'current' stream); this may cause issues
  if (!next_is_tail)
    inherited::mod_->link (const_cast<MODULE_T*> (stream_p->tail ()));
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
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::onUnlink (ACE_Module_Base* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::onUnlink"));

  // sanity check(s)
  typename inherited::MODULE_T* module_p =
    dynamic_cast<typename inherited::MODULE_T*> (module_in);
  ACE_ASSERT (module_p);

  // remove map entry
  LINKS_ITERATOR_T iterator, iterator_2;
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
    iterator = writerLinks_.begin ();
    for (;
         iterator != writerLinks_.end ();
         ++iterator)
      if (!ACE_OS::strcmp (module_p->name (),
                           (*iterator).second->name ()))
        break;
    if (iterator != writerLinks_.end ())
    {
      iterator_2 = readerLinks_.find ((*iterator).first);
      ACE_ASSERT (iterator_2 != readerLinks_.end ());

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: unlinked (%s: x --> %s)\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT ((*iterator).first.c_str ()),
      //            (*iterator).second->name ()));
      readerLinks_.erase (iterator_2);
      writerLinks_.erase (iterator);
    } // end IF
  } // end lock scope
}

// -----------------------------------------------------------------------------

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Module_Aggregator_ReaderTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::Stream_Module_Aggregator_ReaderTask_T (ISTREAM_T* stream_in)
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_ReaderTask_T::Stream_Module_Aggregator_ReaderTask_T"));

  ACE_UNUSED_ARG (stream_in);

  inherited::flags_ |= ACE_Task_Flags::ACE_READER;
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
Stream_Module_Aggregator_ReaderTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::put (ACE_Message_Block* messageBlock_in,
                                                             ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_ReaderTask_T::put"));

  ACE_Task_Base* task_base_p = inherited::sibling ();
  ACE_ASSERT (task_base_p);
  WRITER_TASK_T* writer_p = dynamic_cast<WRITER_TASK_T*> (task_base_p);
  if (!writer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Module_Aggregator_WriterTask_T>: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  typename WRITER_TASK_T::LINKS_ITERATOR_T iterator =
    writer_p->readerLinks_.find (writer_p->outboundStreamName_);
  if (iterator == writer_p->readerLinks_.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve upstream module (stream name was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (writer_p->outboundStreamName_.c_str ())));
    return -1;
  } // end IF
  ACE_ASSERT ((*iterator).second);
  TASK_T* task_p = (*iterator).second->reader ();
  ACE_ASSERT (task_p);

  return task_p->put (messageBlock_in,
                      timeValue_in);
}
