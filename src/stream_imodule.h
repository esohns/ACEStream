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

#include <ace/Synch_Traits.h>

#include "common_iclone.h"
#include "common_iget.h"
#include "common_iinitialize.h"

#include "stream_isessionnotify.h"

// forward declarations
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Task;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;
class Stream_IAllocator;

template </* typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          ////////////////////////////////*/
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename HandlerConfigurationType>
class Stream_IModule_T
// : public Stream_ISessionNotify_T<SessionIdType,
//                                  SessionDataType,
//                                  SessionEventType>
 : public Common_IClone_T<ACE_Module<ACE_SYNCH_USE,
                                     TimePolicyType> >
 , public Common_IGet_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
// *NOTE*: this next line wouldn't compile (with MSVC)
// *EXPLANATION*: apparently, on function signatures, the standard stipulates
//                (in 14.5.5.1):
//                "The types of its parameters and, if the function is a class
//                member, the cv- qualifiers (if any) on the function itself
//                and the class in which the member function is declared. The
//                signature of a function template specialization includes the
//                types of its template arguments...."
//                Note that specifically, this does NOT include the return
//                types.
// , public Common_IGet_T<HandlerConfigurationType>
// , public Common_IInitialize_T<HandlerConfigurationType>
{
 public:
  inline virtual ~Stream_IModule_T () {};

  // convenient types
//  typedef Stream_ISessionNotify_T<SessionIdType,
//                                  SessionDataType,
//                                  SessionEventType> INOTIFY_T;
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  typedef Common_IClone_T<MODULE_T> ICLONE_T;
  typedef Common_IClone_T<ACE_Task<ACE_SYNCH_USE,
                                   TimePolicyType> > ITASKCLONE_T;

  // *TODO*: see above
  virtual const HandlerConfigurationType& getHandlerConfiguration () const = 0;

  virtual bool isFinal () const = 0;

  // *NOTE*: streams may call this to reset writer/reader tasks and re-use
  //         existing modules [needed after call to MODULE_TYPE::close(), which
  //         cannot be overloaded (currently not 'virtual')]
  // *WARNING*: do not call this from within module_closed(), it creates endless
  //            recursion (--> stack overflow)...
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
  inline virtual ~Stream_IModuleHandler_T () {};

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL) = 0;

  // *NOTE*: called on tasks after a module has been clone()d
  //         --> use for (re-)initialization, as needed
  virtual bool postClone (ACE_Module<ACE_SYNCH_USE,
                                     TimePolicyType>*) = 0; // clone handle
};

#endif
