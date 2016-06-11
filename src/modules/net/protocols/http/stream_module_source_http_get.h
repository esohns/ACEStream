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

#include "common_time_common.h"

#include "stream_task_base_synch.h"

#include "http_common.h"

// forward declarations
struct HTTP_Record;
class Stream_IAllocator;

template <typename ConfigurationType,
          ///////////////////////////////
          typename SessionMessageType,
          typename ProtocolMessageType>
class Stream_Module_Net_Source_HTTP_Get_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 ProtocolMessageType>
{
 public:
  Stream_Module_Net_Source_HTTP_Get_T ();
  virtual ~Stream_Module_Net_Source_HTTP_Get_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // helper methods
  ProtocolMessageType* allocateMessage (unsigned int); // (requested) size
  ProtocolMessageType* makeRequest (const std::string&,     // URI
                                    const HTTP_Headers_t&); // headers
  bool sendRequest (const std::string&,     // URI
                    const HTTP_Headers_t&); // headers
  // *NOTE*: (if possible,) this advances the read pointer to skip over the HTTP
  //         entity head
  HTTP_Record* parseResponse (ProtocolMessageType&);

  ConfigurationType*                   configuration_;
  bool                                 responseParsed_;
  bool                                 responseReceived_;
  typename SessionMessageType::DATA_T* sessionData_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 ProtocolMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T (const Stream_Module_Net_Source_HTTP_Get_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T& operator= (const Stream_Module_Net_Source_HTTP_Get_T&))

  bool                                 initialized_;
};

#include "stream_module_source_http_get.inl"

#endif
