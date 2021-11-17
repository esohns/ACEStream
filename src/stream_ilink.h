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

#ifndef STREAM_ILINK_H
#define STREAM_ILINK_H

#include <string>

#include "ace/Module.h"

#include "common_iinitialize.h"

#include "stream_common.h"

class Stream_IDistributorModule
 : public Common_IInitialize_T<Stream_Branches_t>
{
 public:
  virtual bool push (Stream_Module_t*) = 0; // module handle
  virtual bool pop (Stream_Module_t*) = 0; // module handle

  virtual Stream_Module_t* head (const std::string&) const = 0; // branch name
  virtual std::string branch (Stream_Module_t*) const = 0; // 'head' module handle

  // *NOTE*: the return index value is correct as long as:
  //         - the module has been initialize()d
  //         - the corresponding head module has not been push()ed yet
  virtual bool has (const std::string&,       // branch name
                    unsigned int&) const = 0; // return value: index (zero-based; see above)
  virtual Stream_ModuleList_t next () const = 0; // return value: branch 'head's
};

//////////////////////////////////////////

class Stream_ILinkCB
{
 public:
  virtual void onLink () = 0;
  virtual void onUnlink () = 0;
};

class Stream_IModuleLinkCB
{
 public:
  // *NOTE*: invoked after (!) the module has been (re-)linked
  virtual void onLink (ACE_Module_Base*) = 0; // 'downstream' ? upstream predecessor handle : downstream successor handle
  // *NOTE*: invoked just before (!) the module is unlinked
  virtual void onUnlink (ACE_Module_Base*) = 0; // 'downstream' ? upstream predecessor handle : downstream successor handle
};

#endif
