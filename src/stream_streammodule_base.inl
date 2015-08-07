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

#include "stream_common.h"
#include "stream_macros.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_StreamModule_T<TaskSynchType,
                      TimePolicyType,
                      ConfigurationType,
                      HandlerConfigurationType,
                      ReaderTaskType,
                      WriterTaskType>::Stream_StreamModule_T (const std::string& name_in,
                                                              Common_IRefCount* refCount_in,
                                                              bool finalModule_in)
 : inherited (name_in,
              &writer_,       // initialize writer side task
              &reader_,       // initialize reader side task
              refCount_in,    // argument passed to task open()
              finalModule_in) // final module ?
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModule_T::Stream_StreamModule_T"));

  // set task links to the module
  // *NOTE*: essential for dereferencing (name-lookups, controlled shutdown,
  //         etc)
  writer_.mod_ = this;
  reader_.mod_ = this;
  //reader_.flags_ |= ACE_Task_Flags::ACE_READER;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename ReaderTaskType,
          typename WriterTaskType>
Stream_StreamModule_T<TaskSynchType,
                      TimePolicyType,
                      ConfigurationType,
                      HandlerConfigurationType,
                      ReaderTaskType,
                      WriterTaskType>::~Stream_StreamModule_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModule_T::~Stream_StreamModule_T"));

  // *WARNING*: the ACE_Module dtor calls close() on the tasks, implicitly
  //            calling module_closed() and flush() on every task. However, all
  //            member tasks have been destroyed by the time that happens
  //            --> close() module in advance so it doesn't happen here !

  // sanity check: on the stream ?
  Stream_Module_t* module_p = inherited::next ();
  if (!module_p)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("manually closing module: \"%s\"\n"),
    //            ACE_TEXT (module->name ())));

    int result = -1;
    try
    {
      result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                  inherited::name ()));
    }
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%s\", continuing\n"),
                  inherited::name ()));
  } // end IF
}

// ----------------------------------------------------------------------------

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename TaskType>
Stream_StreamModuleInputOnly_T<TaskSynchType,
                               TimePolicyType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               TaskType>::Stream_StreamModuleInputOnly_T(const std::string& name_in,
                                                                         Common_IRefCount* refCount_in,
                                                                         bool finalModule_in)
 : inherited (name_in,
              refCount_in,    // arg passed to task open()
              finalModule_in) // final module ?
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleInputOnly_T::Stream_StreamModuleInputOnly_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename TaskType>
Stream_StreamModuleInputOnly_T<TaskSynchType,
                               TimePolicyType,
                               ConfigurationType,
                               HandlerConfigurationType,
                               TaskType>::~Stream_StreamModuleInputOnly_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StreamModuleInputOnly_T::~Stream_StreamModuleInputOnly_T"));

  // *WARNING*: the ACE_Module dtor calls close() on the tasks, implicitly
  //            calling module_closed() and flush() on every task. However, all
  //            member tasks have been destroyed by the time that happens
  //            --> close() module in advance so it doesn't happen here !

  // sanity check: on the stream ?
  Stream_Module_t* module_p = inherited::next ();
  if (!module_p)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("manually closing module: \"%s\"\n"),
    //            ACE_TEXT (module->name ())));

    int result = -1;
    try
    {
      result = inherited::close (ACE_Module_Base::M_DELETE_NONE);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in ACE_Module::close(M_DELETE_NONE), continuing\n"),
                  inherited::name ()));
    }
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Module::close(M_DELETE_NONE): \"%s\", continuing\n"),
                  inherited::name ()));
  } // end IF
}
