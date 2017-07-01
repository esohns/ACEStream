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
                                                            TASK_T* writerTask_in,
                                                            TASK_T* readerTask_in,
                                                            Common_IRefCount* refCount_in,
                                                            bool delete_in,
                                                            bool isFinal_in)
 : inherited (ACE_TEXT_CHAR_TO_TCHAR (name_in.c_str ()), // name
              writerTask_in,                             // initialize writer side task
              readerTask_in,                             // initialize reader side task
              refCount_in,                               // argument passed to task open()
              ACE_Module_Base::M_DELETE_NONE)            // never (!) delete tasks in close()
 , configuration_ (NULL)
 , notify_ (NULL)
 /////////////////////////////////////////
 , delete_ (delete_in)
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

  // close the tasks first so the base-class will not have to
  int result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close(): \"%m\", continuing\n"),
                inherited::name ()));

  if (delete_)
  {
    delete reader_;
    delete writer_;
  } // end IF
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
    // *TODO*: remove type inferences
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_INotify_T::notify(%u,%d), continuing\n"),
                inherited::name (),
                sessionId_in, sessionEvent_in));
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
  // *NOTE*: make sure to always (!) set inherited::M_DELETE_NONE, otherwise
  //         the tasks are deleted (see ACE_Module::close_i())
  inherited::writer (writer_,
                     ACE_Module_Base::M_DELETE_NONE);
  inherited::reader (reader_,
                     ACE_Module_Base::M_DELETE_NONE);

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

  // sanity checks(s)
  ACE_ASSERT (writer_);
  ACE_ASSERT (reader_);

  TASK_T* task_p, *task_2 = NULL;
  typename IMODULE_T::ITASKCLONE_T* itaskclone_p =
      dynamic_cast<typename IMODULE_T::ITASKCLONE_T*> (writer_);
  if (!itaskclone_p)
  {
    ACE_NEW_NORETURN (task_p,
                      THRU_TASK_T ());
    if (!task_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                  inherited::name ()));
      return NULL;
    } // end IF

    goto continue_;
  } // end IF
  try {
    task_p = itaskclone_p->clone ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IClone_T::clone(), continuing\n"),
                inherited::name ()));
  }
  if (!task_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IClone_T::clone(), aborting\n"),
                inherited::name ()));
    return NULL;
  } // end IF

continue_:
  itaskclone_p =
      dynamic_cast<typename IMODULE_T::ITASKCLONE_T*> (reader_);
  if (!itaskclone_p)
  {
    ACE_NEW_NORETURN (task_2,
                      THRU_TASK_T ());
    if (!task_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                  inherited::name ()));

      // clean up
      delete task_p;

      return NULL;
    } // end IF

    goto continue_2;
  } // end IF
  try {
    task_2 = itaskclone_p->clone ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IClone_T::clone(), continuing\n"),
                inherited::name ()));
  }
  if (!task_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IClone_T::clone(), aborting\n"),
                inherited::name ()));

    // clean up
    delete task_p;

    return NULL;
  } // end IF

continue_2:
  ACE_NEW_NORETURN (module_p,
                    OWN_TYPE_T (inherited::name (),
                                task_p,
                                task_2,
                                NULL,
                                true,
                                isFinal_));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::name ()));

    // clean up
    delete task_p;
    delete task_2;

    return NULL;
  } // end IF

  bool result = false;
  if (task_p)
  {
    IMODULE_HANDLER_T* imodule_handler_p =
        dynamic_cast<IMODULE_HANDLER_T*> (writer_);
    if (!imodule_handler_p)
      goto continue_3;
    try {
      result = imodule_handler_p->postClone (this,
                                             false); // initialize ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IModuleHandler_T::postClone(), continuing\n")));
      result = false;
    }
    if (!result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_IModuleHandler_T::postClone(), aborting\n")));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
  } // end IF
continue_3:
  if (task_2)
  {
    IMODULE_HANDLER_T* imodule_handler_p =
        dynamic_cast<IMODULE_HANDLER_T*> (reader_);
    if (!imodule_handler_p)
      goto done;
    try {
      result = imodule_handler_p->postClone (this,
                                             false);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IModuleHandler_T::postClone(), continuing\n")));
      result = false;
    }
    if (!result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_IModuleHandler_T::postClone(), aborting\n")));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
  } // end IF

done:
  return module_p;
}
