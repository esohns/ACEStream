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

#ifndef TEST_I_MODULE_XPATHQUERY_H
#define TEST_I_MODULE_XPATHQUERY_H

#include "ace/Global_Macros.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_task_base_synch.h"

// definitions
#define STREAM_MODULE_XPATHQUERY_QUERY_STRING "/html/body/div[@id=\"container\"]/div[@id=\"container_content\"]/div[@id=\"mitte\"]/div[@id=\"mitte_links\"]/div[@id=\"archiv_woche\"]/ul/li/a"

template <typename SessionMessageType,
          typename MessageType,
          ///////////////////////////////
          typename ModuleHandlerConfigurationType,
          ///////////////////////////////
          typename SessionDataType>
class Stream_Module_XPathQuery_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ModuleHandlerConfigurationType>
{
 public:
  Stream_Module_XPathQuery_T ();
  virtual ~Stream_Module_XPathQuery_T ();

  // implement (part of) Stream_ITaskBase
//  virtual void handleDataMessage (MessageType*&, // data message handle
//                                  bool&);        // return value: pass message downstream ?
  // implement this so we can print overall statistics after session completes...
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual const ModuleHandlerConfigurationType& get () const;
  virtual bool initialize (const ModuleHandlerConfigurationType&);

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_XPathQuery_T (const Stream_Module_XPathQuery_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_XPathQuery_T& operator= (const Stream_Module_XPathQuery_T&))

  ModuleHandlerConfigurationType configuration_;
};

#include "stream_module_xpathquery.inl"

#endif
