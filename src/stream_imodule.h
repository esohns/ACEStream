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

#ifndef STREAM_IMODULE_H
#define STREAM_IMODULE_H

#include "ace/Global_Macros.h"

#include "common_iclone.h"
#include "common_iget.h"
#include "common_iinitialize.h"

#include "stream_ilink.h"
#include "stream_isessionnotify.h"

// forward declarations
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Task;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Stream;
class Stream_IAllocator;

template <typename SessionDataType,
          typename SessionEventType,
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename HandlerConfigurationType>
class Stream_IModule_T
 : public Stream_ISessionNotify_T<SessionDataType,
                                  SessionEventType>
 , public Stream_IModuleLinkCB
 , public Common_IClone_T<ACE_Module<ACE_SYNCH_USE,
                                     TimePolicyType> >
 , public Common_IGetR_T<ACE_Stream<ACE_SYNCH_USE,
                                    TimePolicyType> >
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IGetR_2_T<ConfigurationType>
 , public Common_IGetR_3_T<HandlerConfigurationType>
{
 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef Common_IClone_T<TASK_T> ICLONE_TASK_T;
  typedef Common_IClone_T<MODULE_T> ICLONE_T;
  typedef Stream_ISessionNotify_T<SessionDataType,
                                  SessionEventType> INOTIFY_T;

  // *NOTE*: streams call this to reset writer/reader tasks and re-use
  //         existing modules. This is e.g. required after ACE_Module::close(),
  //         which, currently not being 'virtual', cannot be overloaded at this
  //         time
  // *WARNING*: do not call this from module_closed(), it leads to infinite
  //            recursion (--> stack overflow)
  virtual void reset () = 0;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType>
class Stream_IModuleHandler_T
// : public Common_IInitialize_T<ConfigurationType>
{
 public:
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL) = 0;

  // *NOTE*: called on tasks after a parent module has been clone()d
  //         --> use for (re-)initialization, as needed
  virtual bool postClone (ACE_Module<ACE_SYNCH_USE,
                                     TimePolicyType>*, // handle to 'original'
                          bool = false) = 0;           // initialize from 'original' ?
};

#endif
