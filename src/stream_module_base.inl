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

#include "stream_common.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::Stream_Module_Base_T (const std::string& name_in,
                                                            WriterTaskType* writerTask_in,
                                                            ReaderTaskType* readerTask_in,
                                                            Common_IRefCount* refCount_in,
                                                            bool isFinal_in)
 : inherited (ACE_TEXT_CHAR_TO_TCHAR (name_in.c_str ()), // name
              writerTask_in,                             // initialize writer side task
              readerTask_in,                             // initialize reader side task
              refCount_in,                               // argument passed to task open()
              inherited::M_DELETE_NONE)                  // don't "delete" ANYTHING during close()
 , configuration_ (NULL)
 , notify_ (NULL)
 /////////////////////////////////////////
 , isFinal_ (isFinal_in)
 , reader_ (readerTask_in)
 , writer_ (writerTask_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::Stream_Module_Base_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::~Stream_Module_Base_T () throw ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::~Stream_Module_Base_T"));

  // *WARNING*: the ACE_Module dtor calls module_closed() on each task. Note how
  //            the (member) task instances have been freed by the time that
  //            happens
  //            --> close() module in advance so it doesn't happen here
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
void
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::start (SessionIdType sessionID_in,
                                             const SessionDataType& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::start"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionData_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
void
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::notify (SessionIdType sessionId_in,
                                              const SessionEventType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (notify_);

  // *IMPORTANT NOTE*: note how the session event type is translated to the
  //                   stream notification type
  // *TODO*: these should be distinct types with a (partial) mapping
  // *TODO*: notifications simply generating session messages should not be
  //         forwarded to linked streams to avoid duplicates. As the
  //         implementation of this interface may be third-party, this might
  //         actually not be enforcable
  try {
    notify_->notify (sessionEvent_in,
                     true); // forward upstream ?
  }
  catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_INotify_T::notify(%d), continuing\n"),
                inherited::name (),
                sessionEvent_in));
  }
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
void
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::end (SessionIdType sessionID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::end"));

  ACE_UNUSED_ARG (sessionID_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
const ConfigurationType&
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
bool
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  // *TODO*: remove type inference
  notify_ = configuration_->notify;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
const HandlerConfigurationType&
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::getHandlerConfiguration () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::get"));

  TASK_T* task_p = writer_;
  ACE_ASSERT (task_p);
  IGET_T* iget_p = dynamic_cast<IGET_T*> (task_p);
  if (!iget_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Common_IGet_T*>(%@) failed, aborting\n"),
                inherited::name (),
                task_p));
    return HandlerConfigurationType ();
  } // end IF

  return iget_p->get ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
bool
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::isFinal () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::isFinal"));

  return isFinal_;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
void
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::reset"));

  // (re-)set reader and writer tasks after ACE_Module::close ()
  // *NOTE*: ACE_Module::close() is invoked implicitly by ACE_Stream::remove()
  inherited::writer (writer_,
                     inherited::M_DELETE_NONE);

  inherited::reader (reader_,
                     inherited::M_DELETE_NONE);

  inherited::next (NULL);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
ACE_Module<ACE_SYNCH_USE,
           TimePolicyType>*
Stream_Module_Base_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     ConfigurationType,
                     HandlerConfigurationType,
                     NotificationType,
                     ReaderTaskType,
                     WriterTaskType>::clone ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Base_T::clone"));

  // initialize return value(s)
  MODULE_T* module_p = NULL;

  typename IMODULE_T::ICLONE_T* iclone_p =
      dynamic_cast<typename IMODULE_T::ICLONE_T*> (writer_);
  if (!iclone_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Common_IClone_T> failed, aborting\n"),
                inherited::name ()));
    return NULL;
  } // end IF

  try {
    module_p = iclone_p->clone ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IClone_T::clone(), continuing\n"),
                inherited::name ()));
  }
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IClone_T::clone(), aborting\n"),
                inherited::name ()));
    return NULL;
  } // end IF

  return module_p;
}
