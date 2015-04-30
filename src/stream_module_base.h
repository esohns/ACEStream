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

#ifndef STREAM_MODULE_BASE_H
#define STREAM_MODULE_BASE_H

#include "stream_imodule.h"

#include "ace/Global_Macros.h"
#include "ace/Module.h"

#include <string>

// forward declaration(s)
class Common_IRefCount;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ReaderTaskType,
          typename WriterTaskType>
class Stream_Module_Base_T
 : public ACE_Module<TaskSynchType,
                     TimePolicyType>,
   public Stream_IModule<TaskSynchType,
                         TimePolicyType,
                         ConfigurationType>
{
 public:
  // define convenient types
  //  typedef ReaderTaskType READER_TASK_TYPE;
  //  typedef WriterTaskType WRITER_TASK_TYPE;
  typedef Stream_IModule<TaskSynchType,
                         TimePolicyType,
                         ConfigurationType> IMODULE_TYPE;

  virtual ~Stream_Module_Base_T ();

  // implement (part of) Stream_IModule
  // *TODO*: this clearly is bad design...
  virtual void get (ConfigurationType*&) const;
  virtual bool initialize (const ConfigurationType&);
  virtual void reset ();

 protected:
  Stream_Module_Base_T (const std::string&, // name
                        WriterTaskType*,    // handle to writer task
                        ReaderTaskType*,    // handle to reader task
                        Common_IRefCount*); // object counter

  const ConfigurationType* configuration_;

 private:
  typedef ACE_Module<TaskSynchType,
                     TimePolicyType> inherited;

  // implement (part of) Stream_IModule
  virtual ACE_Module<TaskSynchType,
                     TimePolicyType>* clone ();

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Base_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Base_T (const Stream_Module_Base_T&));
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Base_T& operator= (const Stream_Module_Base_T&));

  ReaderTaskType*          reader_;
  WriterTaskType*          writer_;
};

#include "stream_module_base.inl"

#endif
