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

#ifndef TEST_I_MODULE_HTMLHANDLER_H
#define TEST_I_MODULE_HTMLHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

//#include <libxml/HTMLparser.h>

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_streammodule_base.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

// definitions
#define HTMLHANDLER_XPATH_QUERY_STRING "/html/body/div[@id=\"container\"]/div[@id=\"container_content\"]/div[@id=\"mitte\"]/div[@id=\"mitte_links\"]/div[@id=\"archiv_woche\"]/ul/li/a"

class Test_I_Stream_Module_HTMLHandler
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 Test_I_Stream_SessionMessage,
                                 Test_I_Stream_Message>
 , public Stream_IModuleHandler_T<Test_I_Stream_ModuleHandlerConfiguration>
{
 public:
  Test_I_Stream_Module_HTMLHandler ();
  virtual ~Test_I_Stream_Module_HTMLHandler ();

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_I_Stream_Message*&, // data message handle
                                  bool&);                  // return value: pass message downstream ?
  // implement this so we can print overall statistics after session completes...
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual const Test_I_Stream_ModuleHandlerConfiguration& get () const;
  virtual bool initialize (const Test_I_Stream_ModuleHandlerConfiguration&);

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 Test_I_Stream_SessionMessage,
                                 Test_I_Stream_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Module_HTMLHandler (const Test_I_Stream_Module_HTMLHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_Module_HTMLHandler& operator= (const Test_I_Stream_Module_HTMLHandler&))

  Test_I_Stream_ModuleHandlerConfiguration configuration_;
//  xmlDoc*                                  document_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                             // task synch type
                              Common_TimePolicy_t,                      // time policy
                              Stream_ModuleConfiguration,               // module configuration type
                              Test_I_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Stream_Module_HTMLHandler);        // writer type

#endif
