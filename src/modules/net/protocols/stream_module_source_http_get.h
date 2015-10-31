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

// forward declarations
class Stream_IAllocator;

// definitions
#define HTTP_COMMAND_GET_STRING                       "GET"
#define HTTP_STATUS_BADREQUEST                        400
#define HTTP_VERSION_STRING                           "HTTP/1.1"
#define HTML_DEFAULT_SUFFIX                           ".html"
#define STREAM_MODULE_NET_SOURCE_HTTP_GET_DEFAULT_URL "index.html"

template <typename SessionMessageType,
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
  virtual bool initialize (Stream_IAllocator*,  // message buffer allocator
                           const std::string&); // URL

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 ProtocolMessageType> inherited;

//  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T (const Stream_Module_Net_Source_HTTP_Get_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Source_HTTP_Get_T& operator= (const Stream_Module_Net_Source_HTTP_Get_T&))

  enum Stream_HTTP_Status_Code
  {
    STREAM_HTTP_STATUS_INVALID                 = -1,
    /////////////////////////////////////
    // success
    STREAM_HTTP_STATUS_OK                      = 200,
    /////////////////////////////////////
    // redirection
    STREAM_HTTP_STATUS_MULTIPLECHOICES         = 300,
    STREAM_HTTP_STATUS_MOVEDPERMANENTLY        = 301,
    STREAM_HTTP_STATUS_FOUND                   = 302,
    STREAM_HTTP_STATUS_USEPROXY                = 305,
    STREAM_HTTP_STATUS_TEMPORARYREDIRECT       = 307,
    /////////////////////////////////////
    // client error
    STREAM_HTTP_STATUS_UNAUTHORIZED            = 401,
    STREAM_HTTP_STATUS_FORBIDDEN               = 403,
    STREAM_HTTP_STATUS_NOTFOUND                = 404,
    STREAM_HTTP_STATUS_GONE                    = 410,
    /////////////////////////////////////
    // server error
    STREAM_HTTP_STATUS_INTERNALSERVERERROR     = 500,
    STREAM_HTTP_STATUS_BADGATEWAY              = 502,
    STREAM_HTTP_STATUS_SERVICEUNAVAILABLE      = 503,
    STREAM_HTTP_STATUS_GATEWAYTIMEOUT          = 504,
    STREAM_HTTP_STATUS_HTTPVERSIONNOTSUPPORTED = 505
  };

  // helper methods
  ProtocolMessageType* allocateMessage (unsigned int); // (requested) size
  ProtocolMessageType* makeRequest (const std::string&); // URI
  bool sendRequest (const std::string&); // URI

  Stream_IAllocator* allocator_;
  bool               headerReceived_;
  bool               isInitialized_;
  std::string        URI_;
};

#include "stream_module_source_http_get.inl"

#endif
