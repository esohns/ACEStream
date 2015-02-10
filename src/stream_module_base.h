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

#ifndef RPG_STREAM_MODULE_BASE_H
#define RPG_STREAM_MODULE_BASE_H

#include "rpg_stream_imodule.h"

#include <ace/Global_Macros.h>
#include <ace/Module.h>

#include <string>

// forward declaration(s)
class RPG_Stream_IRefCount;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ReaderTaskType,
          typename WriterTaskType>
class RPG_Stream_Module_Base_t
 : public ACE_Module<TaskSynchType,
                     TimePolicyType>,
   public RPG_Stream_IModule<TaskSynchType,
                             TimePolicyType>
{
 public:
  // define convenient types
//  typedef ReaderTaskType READER_TASK_TYPE;
//  typedef WriterTaskType WRITER_TASK_TYPE;
  typedef RPG_Stream_IModule<TaskSynchType,
                             TimePolicyType> IMODULE_TYPE;

  virtual ~RPG_Stream_Module_Base_t();

  // implement (part of) RPG_Stream_IModule
  virtual void reset();

 protected:
  RPG_Stream_Module_Base_t(const std::string&,     // name
                           WriterTaskType*,        // handle to writer task
                           ReaderTaskType*,        // handle to reader task
                           RPG_Stream_IRefCount*); // object counter

 private:
  typedef ACE_Module<TaskSynchType,
                     TimePolicyType> inherited;

  // implement (part of) RPG_Stream_IModule
  virtual ACE_Module<TaskSynchType,
                     TimePolicyType>* clone();

  ACE_UNIMPLEMENTED_FUNC(RPG_Stream_Module_Base_t());
  ACE_UNIMPLEMENTED_FUNC(RPG_Stream_Module_Base_t(const RPG_Stream_Module_Base_t&));
  ACE_UNIMPLEMENTED_FUNC(RPG_Stream_Module_Base_t& operator=(const RPG_Stream_Module_Base_t&));

  WriterTaskType* myWriter;
  ReaderTaskType* myReader;
};

#include "rpg_stream_module_base.inl"

#endif
