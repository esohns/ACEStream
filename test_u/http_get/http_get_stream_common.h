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

#ifndef HTTP_GET_STREAM_COMMON_H
#define HTTP_GET_STREAM_COMMON_H

#include <list>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "stream_dec_common.h"
#include "stream_dec_defines.h"

#include "net_iconnection.h"
#include "net_configuration.h"
#include "net_connection_manager.h"

#include "http_common.h"
#include "http_defines.h"

//#include "http_get_network.h"

// forward declarations
struct HTTPGet_AllocatorConfiguration;
class HTTPGet_Message;
class HTTPGet_SessionMessage;

struct HTTPGet_MessageData
 : HTTP_Record
{
  HTTPGet_MessageData ()
   : HTTP_Record ()
  {};
  virtual ~HTTPGet_MessageData ()
  {};
  inline void operator+= (struct HTTPGet_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};
typedef Stream_DataBase_T<struct HTTPGet_MessageData> HTTPGet_MessageData_t;

// forward declarations
//extern const char stream_name_string_[];
struct HTTPGet_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_Configuration,
                               struct HTTPGet_ModuleHandlerConfiguration> HTTPGet_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<HTTPGet_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> HTTPGet_ConnectionConfiguration_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //HTTPGet_ConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> HTTPGet_IConnection_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 HTTPGet_ConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> HTTPGet_ConnectionManager_t;
struct HTTPGet_SessionData
 : Stream_SessionData
{
  HTTPGet_SessionData ()
   : Stream_SessionData ()
   , connection (NULL)
   , connectionStates ()
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , targetFileName ()
  {};

  HTTPGet_SessionData& operator+= (const struct HTTPGet_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);
    connectionStates.insert (rhs_in.connectionStates.begin (),
                             rhs_in.connectionStates.end ());
    // format =
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  Net_IINETConnection_t*                    connection;
  Stream_Net_ConnectionStates_t             connectionStates;
  enum Stream_Decoder_CompressionFormatType format; // decompressor module
  std::string                               targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct HTTPGet_SessionData> HTTPGet_SessionData_t;

typedef Stream_ISessionDataNotify_T<struct HTTPGet_SessionData,
                                    enum Stream_SessionMessageType,
                                    HTTPGet_Message,
                                    HTTPGet_SessionMessage> HTTPGet_Notification_t;
typedef std::list<HTTPGet_Notification_t*> HTTPGet_Subscribers_t;
typedef HTTPGet_Subscribers_t::iterator HTTPGet_SubscribersIterator_t;

struct HTTPGet_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  HTTPGet_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , closeAfterReception (HTTP_DEFAULT_CLOSE_AFTER_RECEPTION)
   , configuration (NULL)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , HTTPForm ()
   , HTTPHeaders ()
   , inbound (true)
   , parserConfiguration (NULL)
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
   , URL ()
   , waitForConnect (true)
  {
    closeAfterReception = true; // http get module closes the connection after data has arrived
    passive = false; // net source module maintains the connection
    stopOnUnlink = true; // (downstream) head modules stop the active session after connection closes and the stream is unlinked
  };

  bool                             closeAfterReception;      // HTTP get module
  struct HTTPGet_Configuration*    configuration;
  Net_IINETConnection_t*           connection;               // TCP target/IO module
  Net_ConnectionConfigurations_t*  connectionConfigurations;
  HTTPGet_ConnectionManager_t*     connectionManager;        // TCP IO module
  HTTP_Form_t                      HTTPForm;                 // HTTP get module
  HTTP_Headers_t                   HTTPHeaders;              // HTTP get module
  bool                             inbound;                  // net io module
  struct HTTP_ParserConfiguration* parserConfiguration;      // parser module(s)
  bool                             printProgressDot;         // file writer module
  bool                             pushStatisticMessages;
  HTTPGet_StreamConfiguration_t*   streamConfiguration;      // net source module
  HTTPGet_Notification_t*          subscriber;
  HTTPGet_Subscribers_t*           subscribers;
  std::string                      targetFileName;           // file writer module
  std::string                      URL;                      // HTTP get module
  bool                             waitForConnect;
};

struct HTTPGet_StreamState
 : Stream_State
{
  HTTPGet_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   //, userData (NULL)
  {};

  struct HTTPGet_SessionData* sessionData;

  //struct HTTPGet_UserData*    userData;
};

#endif
