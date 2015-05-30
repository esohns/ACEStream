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

#include "common_iclone.h"
#include "common_iget.h"
#include "common_iinitialize.h"

#include "ace/Module.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename ConfigurationType>
class Stream_IModule
 : public Common_IClone_T<ACE_Module<TaskSynchType,
                                     TimePolicyType> >
 , public Common_IGet_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  inline virtual ~Stream_IModule() {};

  // convenient types
  typedef Common_IClone_T<ACE_Module<TaskSynchType,
                                     TimePolicyType> > ICLONE_TYPE;

  // API
  // *NOTE*: streams may call this to reset writer/reader tasks and re-use
  // existing modules
  // --> needed after call to MODULE_TYPE::close(), which cannot be
  // overriden (not "virtual")
  // *WARNING*: DON'T call this from within module_closed()
  // --> creates endless loops (and eventually, stack overflows)...
  virtual void reset () = 0;
};

#endif
