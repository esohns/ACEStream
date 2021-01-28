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

#ifndef STREAM_MODULE_NET_SOURCE_HTTP_GET_H
#define STREAM_MODULE_NET_SOURCE_HTTP_GET_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_ilock.h"
#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "http_common.h"

//extern STREAM_NET_HTTP_Export const char libacestream_default_net_http_get_module_name_string[];
extern const char libacestream_default_net_http_get_module_name_string[];

// forward declarations
struct HTTP_Record;
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Net_Source_HTTP_Get_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Net_Source_HTTP_Get_T (ISTREAM_T*); // stream handle
#else
  Stream_Module_Net_Source_HTTP_Get_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Stream_Module_Net_Source_HTTP_Get_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // helper methods
  bool send (const std::string&,    // URI
             const HTTP_Headers_t&, // headers
             const HTTP_Form_t&);   // form
  // *NOTE*: (if possible,) this advances the read pointer to skip over the HTTP
  //         entity head
  HTTP_Record* parse (DataMessageType&);

  unsigned int receivedBytes_;
  bool         resentRequest_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T (const Stream_Module_Net_Source_HTTP_Get_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T& operator= (const Stream_Module_Net_Source_HTTP_Get_T&))

  // helper methods
  DataMessageType* makeRequest (const std::string&,    // URI
                                const HTTP_Headers_t&, // headers
                                const HTTP_Form_t&);   // form
};

// include template definition
#include "stream_module_source_http_get.inl"

#endif
