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
#include "ace/Module.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_StreamModule_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      SessionDataType,
                      SessionEventType,
                      ConfigurationType,
                      HandlerConfigurationType,
                      ModuleName,
                      NotificationType,
                      ReaderTaskType,
                      WriterTaskType>::Stream_StreamModule_T (ISTREAM_T* stream_in,
                                                              const std::string& name_in)
 : inherited (name_in,  // name
              &writer_, // initialize writer side task
              &reader_, // initialize reader side task
              false)    // do not close the module in the base class dtor
 , reader_ (stream_in)
 , writer_ (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModule_T::Stream_StreamModule_T"));

  // set task links to the module
  // *NOTE*: essential for dereferencing (name-lookups, controlled shutdown,
  //         etc)
  writer_.mod_ = this;
  reader_.mod_ = this;
  //reader_.flags_ |= ACE_Task_Flags::ACE_READER;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_StreamModule_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      SessionDataType,
                      SessionEventType,
                      ConfigurationType,
                      HandlerConfigurationType,
                      ModuleName,
                      NotificationType,
                      ReaderTaskType,
                      WriterTaskType>::~Stream_StreamModule_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModule_T::~Stream_StreamModule_T"));

  // *IMPORTANT NOTE*: the ACE_Module dtor calls module_closed() on each task.
  //                   However, the (member) tasks have been destroyed by the
  //                   time that happens
  //                   --> close() module early

  int result = -1;
  try {
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                inherited::name ()));
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%s\", continuing\n"),
                inherited::name ()));
}

// -----------------------------------------------------------------------------

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename SessionDataType,
//          typename SessionEventType,
//          typename ConfigurationType,
//          typename HandlerConfigurationType,
//          typename NotificationType,
//          typename TaskType>
//Stream_StreamModule_T<ACE_SYNCH_USE,
//                      TimePolicyType,
//                      SessionDataType,
//                      SessionEventType,
//                      ConfigurationType,
//                      HandlerConfigurationType,
//                      NotificationType,
//                      ACE_Thru_Task<ACE_SYNCH_USE,
//                                    TimePolicyType>,
//                      TaskType>::Stream_StreamModule_T (ISTREAM_T* stream_in,
//                                                        const std::string& name_in,
//                                                        Common_IRefCount* refCount_in,
//                                                        bool finalModule_in)
// : inherited (name_in,
//              &writer_,       // initialize writer side task
//              &reader_,       // initialize reader side task
//              refCount_in,    // argument passed to task open()
//              false,          // do not close the module in the base class dtor
//              finalModule_in) // final module ?
// , reader_ ()
// , writer_ (stream_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_StreamModule_T::Stream_StreamModule_T"));
//
//  // set task links to the module
//  // *NOTE*: essential for dereferencing (name-lookups, controlled shutdown,
//  //         etc)
//  writer_.mod_ = this;
//  reader_.mod_ = this;
//  //reader_.flags_ |= ACE_Task_Flags::ACE_READER;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename TaskType>
Stream_StreamModuleInputOnly_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               ModuleName,
                               NotificationType,
                               TaskType>::Stream_StreamModuleInputOnly_T (ISTREAM_T* stream_in,
                                                                          const std::string& name_in)
 : inherited (name_in,  // name
              &writer_, // initialize writer side task
              &reader_, // initialize reader side task
              false)    // do not close the module in the base class dtor
 , reader_ ()
 , writer_ (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleInputOnly_T::Stream_StreamModuleInputOnly_T"));

  // set task links to the module
  // *NOTE*: essential for dereferencing (name-lookups, controlled shutdown,
  //         etc)
  writer_.mod_ = this;
  reader_.mod_ = this;
  //reader_.flags_ |= ACE_Task_Flags::ACE_READER;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename TaskType>
Stream_StreamModuleInputOnly_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               SessionDataType,
                               SessionEventType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               ModuleName,
                               NotificationType,
                               TaskType>::~Stream_StreamModuleInputOnly_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleInputOnly_T::~Stream_StreamModuleInputOnly_T"));

  // *IMPORTANT NOTE*: the ACE_Module dtor calls module_closed() on each task.
  //                   However, the (member) tasks have been destroyed by the
  //                   time that happens
  //                   --> close() module early

  int result = -1;
  try {
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                inherited::name ()));
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%s\", continuing\n"),
                inherited::name ()));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename TaskType>
Stream_StreamModuleOutputOnly_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                ModuleName,
                                NotificationType,
                                TaskType>::Stream_StreamModuleOutputOnly_T (ISTREAM_T* stream_in,
                                                                            const std::string& name_in)
 : inherited (name_in,  // name
              &writer_, // initialize writer side task
              &reader_, // initialize reader side task
              false)    // do not close the module in the base class dtor
 , reader_ (stream_in)
 , writer_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleOutputOnly_T::Stream_StreamModuleOutputOnly_T"));

  // set task links to the module
  // *NOTE*: essential for dereferencing (name-lookups, controlled shutdown,
  //         etc)
  writer_.mod_ = this;
  reader_.mod_ = this;
  //reader_.flags_ |= ACE_Task_Flags::ACE_READER;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename TaskType>
Stream_StreamModuleOutputOnly_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                ModuleName,
                                NotificationType,
                                TaskType>::~Stream_StreamModuleOutputOnly_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleOutputOnly_T::~Stream_StreamModuleOutputOnly_T"));

  // *IMPORTANT NOTE*: the ACE_Module dtor calls module_closed() on each task.
  //                   However, the (member) tasks have been destroyed by the
  //                   time that happens
  //                   --> close() module early

  int result = -1;
  try {
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                inherited::name ()));
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%s\", continuing\n"),
                inherited::name ()));
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_StreamModuleA_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       SessionDataType,
                       SessionEventType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       ModuleName,
                       NotificationType,
                       ReaderTaskType,
                       WriterTaskType>::Stream_StreamModuleA_T (ISTREAM_T* stream_in,
                                                                const std::string& name_in)
 : inherited (name_in,  // name
              &writer_, // initialize writer side task
              &reader_, // initialize reader side task
              false)    // do not close the module in the base class dtor
 , reader_ (stream_in)
 , writer_ (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleA_T::Stream_StreamModuleA_T"));

  // set task links to the module
  // *NOTE*: essential for dereferencing (name-lookups, controlled shutdown,
  //         etc)
  writer_.mod_ = this;
  reader_.mod_ = this;
  //reader_.flags_ |= ACE_Task_Flags::ACE_READER;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_StreamModuleA_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       SessionDataType,
                       SessionEventType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       ModuleName,
                       NotificationType,
                       ReaderTaskType,
                       WriterTaskType>::~Stream_StreamModuleA_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleA_T::~Stream_StreamModuleA_T"));

  // *IMPORTANT NOTE*: the ACE_Module dtor calls module_closed() on each task.
  //                   However, the (member) tasks have been destroyed by the
  //                   time that happens
  //                   --> close() module early

  int result = -1;
  try {
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                inherited::name ()));
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%s\", continuing\n"),
                inherited::name ()));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename TaskType>
Stream_StreamModuleInputOnlyA_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                ModuleName,
                                NotificationType,
                                TaskType>::Stream_StreamModuleInputOnlyA_T (ISTREAM_T* stream_in,
                                                                            const std::string& name_in)
 : inherited (name_in,  // name
              &writer_, // initialize writer side task
              &reader_, // initialize reader side task
              false)    // do not close the module in the base class dtor
 , reader_ ()
 , writer_ (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleInputOnlyA_T::Stream_StreamModuleInputOnlyA_T"));

  // set task links to the module
  // *NOTE*: essential for dereferencing (name-lookups, controlled shutdown,
  //         etc)
  writer_.mod_ = this;
  reader_.mod_ = this;
  //reader_.flags_ |= ACE_Task_Flags::ACE_READER;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataType,
          typename SessionEventType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          const char* ModuleName,
          typename NotificationType,
          typename TaskType>
Stream_StreamModuleInputOnlyA_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionDataType,
                                SessionEventType,
                                ConfigurationType,
                                HandlerConfigurationType,
                                ModuleName,
                                NotificationType,
                                TaskType>::~Stream_StreamModuleInputOnlyA_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleInputOnlyA_T::~Stream_StreamModuleInputOnlyA_T"));

  // *IMPORTANT NOTE*: the ACE_Module dtor calls module_closed() on each task.
  //                   However, the (member) tasks have been destroyed by the
  //                   time that happens
  //                   --> close() module early

  int result = -1;
  try {
    result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                inherited::name ()));
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%s\", continuing\n"),
                inherited::name ()));
}
