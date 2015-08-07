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

#include "ace/Synch_Traits.h"

#include "common_iclone.h"
#include "common_iget.h"
#include "common_iinitialize.h"

// forward declarations
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;

template <typename ConfigurationType>
class Stream_IModuleHandler_T
 : public Common_IGet_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  inline virtual ~Stream_IModuleHandler_T () {};
};

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType,
          ///////////////////////////////
          typename HandlerConfigurationType>
class Stream_IModule_T
 : public Common_IClone_T<ACE_Module<TaskSynchType,
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
  typedef ACE_Module<TaskSynchType,
                     TimePolicyType> MODULE_T;
  typedef Common_IClone_T<MODULE_T> ICLONE_T;

  // *TODO*: see above
  virtual const HandlerConfigurationType& get () = 0;

  virtual bool isFinal () const = 0;

  // *NOTE*: streams may call this to reset writer/reader tasks and re-use
  //         existing modules [needed after call to MODULE_TYPE::close(), which
  //         cannot be overloaded (currently not 'virtual')]
  // *WARNING*: do not call this from within module_closed(), it creates endless
  //            recursion (--> stack overflow)...
  virtual void reset () = 0;
};

#endif
