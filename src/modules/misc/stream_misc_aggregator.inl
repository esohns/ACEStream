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
#include "stream_tools.h"

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
 , sessions_ ()
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

  Stream_IMessage* imessage_p = NULL;
  SessionIdType session_id = 0;
  SESSIONID_TO_STREAM_MAP_ITERATOR_T iterator;
  TASK_T* task_p = NULL;
  int result = 0;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;

  // step1: handle message
  bool stop_processing = false;
  handleMessage (messageBlock_in,
                 stop_processing);
  if (unlikely (stop_processing))
    goto continue_;

  // step2: forward message to downstream modules
  imessage_p = dynamic_cast<Stream_IMessage*> (messageBlock_in);
  ACE_ASSERT (imessage_p);
  session_id = imessage_p->sessionId ();
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, lock_, -1);
    // find correct stream
    iterator = sessions_.find (session_id);
    if (unlikely (iterator == sessions_.end ()))
    { // *NOTE*: most probable cause: out-of-session statistic session message
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: cannot forward message (session id was: %u), returning\n"),
      //            inherited::mod_->name (),
      //            session_id));
      result = -1;
      goto continue_;
    } // end IF

    for (LINKS_ITERATOR_T iterator_2 = writerLinks_.begin ();
         iterator_2 != writerLinks_.end ();
         ++iterator_2)
    {
      if (ACE_OS::strcmp ((*iterator).second->name ().c_str (),
                          (*iterator_2).first.c_str ()))
        continue;

      ACE_ASSERT ((*iterator_2).second);
      task_p = (*iterator_2).second->writer ();
      ACE_ASSERT (task_p);

      ACE_ASSERT (!message_block_p);
      message_block_p = messageBlock_in->duplicate ();
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        result = -1;
        break;
      } // end IF

      result_2 = task_p->put (message_block_p,
                              timeValue_in);
      if (unlikely (result_2 == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put(): \"%m\", continuing\n"),
                    (*iterator_2).second->name ()));

        // clean up
        message_block_p->release ();

        result = -1;
      } // end IF
      message_block_p = NULL;
    } // end FOR
  } // end lock scope

continue_:
  // clean up
  if (!stop_processing)
    messageBlock_in->release ();

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
ACE_Task<ACE_SYNCH_USE, TimePolicyType>*
Stream_Module_Aggregator_WriterTask_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ConfigurationType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      SessionIdType,
                                      SessionDataType>::next (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::next"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);
  typename inherited::IGET_T* iget_p =
      dynamic_cast<typename inherited::IGET_T*> (inherited::mod_);
  ACE_ASSERT (iget_p);
  // *WARNING*: this retrieves the 'most upstream' (sub-)stream
  typename inherited::STREAM_T& stream_r =
      const_cast<typename inherited::STREAM_T&> (iget_p->getR ());
  ACE_ASSERT (stream_r.tail ());

  return stream_r.tail ()->writer ();
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

  if (unlikely (inherited::isInitialized_))
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

    //sessions_.clear ();

    outboundStreamName_.clear ();
  } // end IF

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
                                      SessionDataType>::handleMessage (ACE_Message_Block* messageBlock_in,
                                                                       bool& stopProcessing_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::handleMessage"));

  // sanity check
  ACE_ASSERT (messageBlock_in);

  // initialize return value(s)
  stopProcessing_out = false;

  bool forward_b = true;
  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_DATA:
    case ACE_Message_Block::MB_PROTO:
    {
      DataMessageType* message_p =
        dynamic_cast<DataMessageType*> (messageBlock_in);
      if (!message_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<DataMessageType>(0x%@) failed (type was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    messageBlock_in,
                    ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
        goto error;
      } // end IF

        //// *IMPORTANT NOTE*: in certain scenarios (e.g. asynchronous 
        ////                   configurations with a network data source), data may
        ////                   start arriving before the corresponding session has
        ////                   finished initializing (i.e. before the
        ////                   STREAM_SESSION_MESSAGE_BEGIN message has been
        ////                   processed by all modules). Due to this race
        ////                   condition, no session data is available at this
        ////                   stage, and the modules may not behave as intended
        ////                   --> prevent dispatch of data messages in this case
        //// *WARNING*: this test does not work reliably, it only mitigates the race
        ////            condition described
        //// *TODO*: find a way to prevent this from occurring (e.g. pre-buffer all
        ////         'early' messages in the head module, introduce an intermediate
        ////         state machine state 'in_session') to handle these situations
        //if (!sessionData_)
        //{ ACE_ASSERT (inherited::mod_);
        //  if (this == inherited::mod_->writer ())
        //  {
        //    //ACE_DEBUG ((LM_WARNING,
        //    //            ACE_TEXT ("%s: no session: dropping 'early' data message, continuing\n"),
        //    //            inherited::mod_->name ()));
        //    goto error;
        //  } // end IF
        //} // end IF

      try {
        this->handleDataMessage (message_p,
                                 forward_b);
      } catch (...) {
        //          ACE_DEBUG ((LM_ERROR,
        //                      ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleDataMessage() (message id was: %u), continuing\n"),
        //                      inherited::mod_->name (),
        //                      message_p->id ()));
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleDataMessage(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      }

      break;

error:
      break;
    }
    case ACE_Message_Block::MB_BREAK:
    case ACE_Message_Block::MB_FLUSH:
    case ACE_Message_Block::MB_HANGUP:
    case ACE_Message_Block::MB_NORMAL: // undifferentiated
    case STREAM_CONTROL_CONNECT:
    case STREAM_CONTROL_LINK:
    case STREAM_CONTROL_STEP:
    {
      ControlMessageType* control_message_p =
        dynamic_cast<ControlMessageType*> (messageBlock_in);
      if (!control_message_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: dynamic_cast<ControlMessageType>(0x%@) failed (type was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    messageBlock_in,
                    ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
        goto error_2;
      } // end IF

      try {
        this->handleControlMessage (*control_message_p);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleControlMessage(), aborting\n"),
                    inherited::mod_->name ()));
      }

      break;

error_2:
      break;
    }
    case ACE_Message_Block::MB_USER:
    {
      try {
        this->handleUserMessage (messageBlock_in,
                                 stopProcessing_out,
                                 forward_b);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught an exception in Stream_ITask_T::handleUserMessage() (type was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
        goto error_3;
      }

      break;

error_3:
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: received invalid/unknown message (type was: \"%s\"), continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Tools::messageTypeToString (static_cast<enum Stream_MessageType> (messageBlock_in->msg_type ())).c_str ())));
      break;
    }
  } // end SWITCH

  // pass message downstream ?
  if (unlikely (!forward_b))
    stopProcessing_out = true;
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
                                      SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // clean up
  passMessageDownstream_out = false;
  message_inout->release (); message_inout = NULL;
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

  // sanity check(s)
  ACE_ASSERT (message_inout);

  enum Stream_SessionMessageType message_type_e = message_inout->type ();
  SessionIdType session_id = message_inout->sessionId ();

  switch (message_type_e)
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
insert:
    {
      typename inherited::TASK_BASE_T::ISTREAM_T* istream_p =
          const_cast<typename inherited::TASK_BASE_T::ISTREAM_T*> (inherited::getP ());
      ACE_ASSERT (istream_p);
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
        sessions_.insert (std::make_pair (session_id,
                                          istream_p));
      } // end lock scope
    } // *WARNING*: control falls through here
    case STREAM_SESSION_MESSAGE_LINK:
    {
      SESSIONID_TO_STREAM_MAP_ITERATOR_T iterator;
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
        iterator = sessions_.find (session_id);
        // *IMPORTANT NOTE*: if a substream has been prepended, the session id
        //                   may not be known yet --> insert
        if (iterator == sessions_.end ())
          goto insert;

        ACE_ASSERT (inherited::sessionData_);
        inherited::sessionData_->increase ();
        sessionData_.insert (std::make_pair (session_id,
                                             inherited::sessionData_));
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      SESSIONID_TO_STREAM_MAP_ITERATOR_T iterator;
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
        iterator = sessions_.find (session_id);
        if (likely (iterator != sessions_.end ()))
          sessions_.erase (iterator);
      } // end lock scope
    } // *WARNING*: control falls through here
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      SESSION_DATA_ITERATOR_T iterator;
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
        iterator = sessionData_.find (session_id);
        if (likely (iterator != sessionData_.end ()))
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

  // clean up
  passMessageDownstream_out = false;
  message_inout->release (); message_inout = NULL;
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
                                      SessionDataType>::onLink (ACE_Module_Base* module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Aggregator_WriterTask_T::onLink"));

  // sanity check(s)
  ACE_ASSERT (module_in);

  const MODULE_T* module_p = dynamic_cast<MODULE_T*> (module_in);
  const MODULE_T* module_2 = NULL;
  const MODULE_T* tail_p = NULL;
  typename inherited::IGET_T* iget_p = NULL;
  typename inherited::STREAM_T* stream_p = NULL;
  typename inherited::TASK_BASE_T::ISTREAM_T* istream_p = NULL;

  // sanity check(s)
  ACE_ASSERT (module_p);
  if (!ACE_OS::strcmp (module_p->name (),
                       ACE_TEXT ("ACE_Stream_Tail")) ||
      !ACE_OS::strcmp (module_p->name (),
                       ACE_TEXT (STREAM_MODULE_TAIL_NAME)))
  {
    // *NOTE*: 'this' is being push()ed onto the stream
    //         --> nothing to do
    return;
  } // end IF
  module_2 = const_cast<MODULE_T*> (module_p)->next ();
  ACE_ASSERT (module_2);
  if (!ACE_OS::strcmp (inherited::mod_->name (),
                       module_2->name ()))
  {
    // *NOTE*: 'this' is 'downstream'
    //         --> nothing to do
    return;
  } // end IF

  // *NOTE*: 'this' is (the tail end of-) 'upstream' --> proceed

  // step1: retrieve a stream handle of the module
  iget_p =
      dynamic_cast<typename inherited::IGET_T*> (const_cast<MODULE_T*> (module_p));
  if (!iget_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Common_IGetR_T<ACE_Stream>>(0x%@) failed, returning\n"),
                module_p->name (),
                module_p));
    return;
  } // end IF
  stream_p =
      &const_cast<typename inherited::STREAM_T&> (iget_p->getR ());
  tail_p = stream_p->tail ();
  ACE_ASSERT (tail_p);
  istream_p =
      dynamic_cast<typename inherited::TASK_BASE_T::ISTREAM_T*> (stream_p);
  if (unlikely (!istream_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStream_T>(0x%@) failed, returning\n"),
                module_p->name (),
                stream_p));
    return;
  } // end IF
  stream_p = istream_p->upstream (false); // do not recurse
  ACE_ASSERT (stream_p);
  istream_p =
      dynamic_cast<typename inherited::TASK_BASE_T::ISTREAM_T*> (stream_p);
  if (unlikely (!istream_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStream_T>(0x%@) failed, returning\n"),
                module_p->name (),
                stream_p));
    return;
  } // end IF

  // step2: find upstream module (on that stream)
  for (STREAM_ITERATOR_T iterator (*stream_p);
       iterator.next (module_2);
       iterator.advance ())
  { ACE_ASSERT (const_cast<MODULE_T*> (module_2)->next ());
    if (!ACE_OS::strcmp (const_cast<MODULE_T*> (module_2)->next ()->name (),
                         inherited::mod_->name ()))
      break;
  } // end FOR
  if (unlikely (!module_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: could not find upstream module, returning\n"),
                module_p->name ()));
    return;
  } // end IF

  // step3: add map entry
  { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, lock_);
    readerLinks_.insert (std::make_pair (istream_p->name (),
                                         const_cast<MODULE_T*> (module_2)));
    writerLinks_.insert (std::make_pair (istream_p->name (),
                                         const_cast<MODULE_T*> (module_p)));
  } // end lock scope

  // step4: reset 'next' module to the stream tail
  // *IMPORTANT NOTE*: 'this' always references the tail of the modules' stream
  //                   most recently linked; that may not be accurate
  // *NOTE*: avoid ACE_Module::link(); it implicitly invokes
  //         Stream_Module_Base_T::next(), which would effectively remove the
  //         reader/writer link(s) (see above) again
  //inherited::mod_->link (tail_p);
  inherited::mod_->writer ()->next (const_cast<MODULE_T*> (tail_p)->writer ());
  const_cast<MODULE_T*> (tail_p)->reader ()->next (inherited::mod_->reader ());
  inherited::mod_->MODULE_T::next (const_cast<MODULE_T*> (tail_p));

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: linked (%s --> x --> %s)\n"),
  //            inherited::mod_->name (),
  //            module_2->name (),
  //            module_p->name ()));
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
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: unlinked (%s: x --> %s)\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT ((*iterator).first.c_str ()),
      //            (*iterator).second->name ()));
      iterator_2 = readerLinks_.find ((*iterator).first);
      ACE_ASSERT (iterator_2 != readerLinks_.end ());
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

  WRITER_TASK_T* writer_p =
    dynamic_cast<WRITER_TASK_T*> (inherited::sibling ());
  if (unlikely (!writer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Module_Aggregator_WriterTask_T>: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  typename WRITER_TASK_T::LINKS_ITERATOR_T iterator =
    writer_p->readerLinks_.find (writer_p->outboundStreamName_);
  if (unlikely (iterator == writer_p->readerLinks_.end ()))
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
