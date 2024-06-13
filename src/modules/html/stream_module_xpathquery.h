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

#include "stream_common.h"
#include "stream_task_base_synch.h"

// definitions
// *TODO*: move this somewhere else
//#define STREAM_MODULE_XPATHQUERY_QUERY_STRING "/html/body/div[@id=\"container\"]/div[@id=\"container_content\"]/div[@id=\"mitte\"]/div[@id=\"mitte_links\"]/div[@id=\"archiv_woche\"]/ul/li/a"

extern const char libacestream_default_xpath_query_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          typename SessionDataType>
class Stream_Module_XPathQuery_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Stream_Module_XPathQuery_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_XPathQuery_T () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  // implement this so we can print overall statistics after session completes
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_XPathQuery_T (const Stream_Module_XPathQuery_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_XPathQuery_T& operator= (const Stream_Module_XPathQuery_T&))
};

// include template definition
#include "stream_module_xpathquery.inl"

#endif
