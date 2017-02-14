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

#ifndef TEST_I_COMMON_H
#define TEST_I_COMMON_H

#include <algorithm>
#include <deque>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>

#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include <libxml/tree.h>

#include "common.h"
#include "common_inotify.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"
#include "stream_statemachine_control.h"

#include "stream_module_db_common.h"

#include "stream_dec_common.h"
#include "stream_dec_defines.h"

#include "stream_module_htmlparser.h"

#include "stream_module_net_common.h"

#include "net_configuration.h"

#include "http_common.h"
#include "http_defines.h"

#include "test_i_defines.h"

#include "test_i_connection_manager_common.h"
#include "test_i_http_get_network.h"
//#include "test_i_message.h"
//#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;
struct Test_I_ConnectionState;

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

typedef Stream_Statistic Test_I_RuntimeStatistic_t;

typedef Common_IStatistic_T<Test_I_RuntimeStatistic_t> Test_I_StatisticReportingHandler_t;

struct Test_I_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_I_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    paddingBytes = STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

struct Test_I_MessageData
{
  inline Test_I_MessageData ()
   : HTTPRecord (NULL)
   , HTMLDocument (NULL)
  {};
  inline ~Test_I_MessageData ()
  {
    if (HTTPRecord)
      delete HTTPRecord;
    if (HTMLDocument)
      xmlFreeDoc (HTMLDocument);
  };
 inline void operator+= (struct Test_I_MessageData rhs_in)
 { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); };
 inline operator struct HTTP_Record&() const
 { ACE_ASSERT (HTTPRecord); return *HTTPRecord; };

  struct HTTP_Record* HTTPRecord;
  xmlDocPtr           HTMLDocument;
};
//typedef Stream_DataBase_T<struct Test_I_MessageData> Test_I_MessageData_t;

struct Test_I_DataItem
{
 inline Test_I_DataItem ()
  : description ()
  , URI ()
 {};
 inline bool operator== (struct Test_I_DataItem rhs_in)
 {
   return URI == rhs_in.URI;
 };

 std::string description;
 std::string URI;
};
typedef std::list<struct Test_I_DataItem> Test_I_DataItems_t;
typedef Test_I_DataItems_t::const_iterator Test_I_DataItemsIterator_t;
typedef std::map<ACE_Time_Value, Test_I_DataItems_t> Test_I_PageData_t;
typedef Test_I_PageData_t::const_reverse_iterator Test_I_PageDataReverseConstIterator_t;
typedef Test_I_PageData_t::iterator Test_I_PageDataIterator_t;
struct Test_I_DataSet
{
  inline Test_I_DataSet ()
   : pageData ()
   , title ()
  {};

  inline struct Test_I_DataSet& operator+= (const struct Test_I_DataSet& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    pageData.insert (rhs_in.pageData.begin (), rhs_in.pageData.end ());
    title = (title.empty () ? rhs_in.title : title);

    return *this;
  }

  Test_I_PageData_t pageData;
  std::string       title;
};
typedef std::list<struct Test_I_DataSet> Test_I_DataSets_t;
typedef Test_I_DataSets_t::const_iterator Test_I_DataSetsIterator_t;

enum Test_I_SAXParserState
{
  SAXPARSER_STATE_INVALID = -1,
  ////////////////////////////////////////
  SAXPARSER_STATE_IN_HEAD = 0,
  SAXPARSER_STATE_IN_HTML,
  SAXPARSER_STATE_IN_BODY,
  ////////////////////////////////////////
  SAXPARSER_STATE_READ_DATE,
  SAXPARSER_STATE_READ_DESCRIPTION,
  SAXPARSER_STATE_READ_TITLE,
  ////////////////////////////////////////
  SAXPARSER_STATE_READ_ITEM,
  SAXPARSER_STATE_READ_ITEMS
};
struct Test_I_SAXParserContext
 : Stream_Module_HTMLParser_SAXParserContextBase
{
  inline Test_I_SAXParserContext ()
   : Stream_Module_HTMLParser_SAXParserContextBase ()
   , sessionData (NULL)
   , dataItem ()
   , state (SAXPARSER_STATE_INVALID)
   , timeStamp ()
  {};

  struct Test_I_Stream_SessionData* sessionData;

  struct Test_I_DataItem            dataItem;
  enum Test_I_SAXParserState        state;
  ACE_Time_Value                    timeStamp;
};

struct Test_I_ConnectionConfiguration;
struct Test_I_StreamConfiguration;
struct Test_I_UserData
 : Stream_UserData
{
  inline Test_I_UserData ()
   : Stream_UserData ()
   , connectionConfiguration (NULL)
   , streamConfiguration (NULL)
  {};

  struct Test_I_ConnectionConfiguration* connectionConfiguration;
  struct Test_I_StreamConfiguration*     streamConfiguration;
};

struct Test_I_Stream_SessionData
 : Stream_SessionData
{
  inline Test_I_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , data ()
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , parserContext (NULL)
   , targetFileName ()
   , userData (NULL)
  {};

  inline Test_I_Stream_SessionData& operator+= (const struct Test_I_Stream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connectionState =
      (connectionState ? connectionState : rhs_in.connectionState);
    data += rhs_in.data;
    //format =
    parserContext = (parserContext ? parserContext : rhs_in.parserContext);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  struct Test_I_ConnectionState*            connectionState;
  struct Test_I_DataSet                     data; // html handler module
  enum Stream_Decoder_CompressionFormatType format; // decompressor module
  struct Test_I_SAXParserContext*           parserContext; // html parser/handler module
  std::string                               targetFileName; // file writer module
  struct Test_I_UserData*                   userData;
};
typedef Stream_SessionData_T<struct Test_I_Stream_SessionData> Test_I_Stream_SessionData_t;

//struct Test_I_Stream_SocketHandlerConfiguration
// : Net_SocketHandlerConfiguration
//{
//  inline Test_I_Stream_SocketHandlerConfiguration ()
//   : Net_SocketHandlerConfiguration ()
//   ///////////////////////////////////////
//   , userData (NULL)
//  {};

//  struct Test_I_UserData* userData;
//};

// forward declarations
struct Test_I_Configuration;
struct Test_I_ModuleHandlerConfiguration;
struct Test_I_SocketHandlerConfiguration;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_StreamState,
                      struct Test_I_StreamConfiguration,
                      Test_I_RuntimeStatistic_t,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_ModuleHandlerConfiguration,
                      struct Test_I_Stream_SessionData,
                      Test_I_Stream_SessionData_t,
                      Test_I_ControlMessage_t,
                      Test_I_Stream_Message,
                      Test_I_Stream_SessionMessage> Test_I_StreamBase_t;
struct Test_I_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Test_I_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , connection (NULL)
   , connectionManager (NULL)
   , dataBaseOptionsFileName ()
   , dataBaseTable ()
   , hostName ()
   , HTTPForm ()
   , HTTPHeaders ()
   , inbound (true)
   , loginOptions ()
   , mode (STREAM_MODULE_HTMLPARSER_SAX)
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , stream (NULL)
   , targetFileName ()
   , URL ()
  {
    crunchMessages = HTTP_DEFAULT_CRUNCH_MESSAGES; // HTTP parser module
    passive = false;
  };

  struct Test_I_Configuration*               configuration;
  Test_I_IConnection_t*                      connection; // TCP target/IO module
  Test_I_Stream_InetConnectionManager_t*     connectionManager; // TCP IO module
  std::string                                dataBaseOptionsFileName; // db writer module
  std::string                                dataBaseTable; // db writer module
  std::string                                hostName; // net source module
  HTTP_Form_t                                HTTPForm; // HTTP get module
  HTTP_Headers_t                             HTTPHeaders; // HTTP get module
  bool                                       inbound; // net io module
  struct Stream_Module_DataBase_LoginOptions loginOptions; // db writer module
  enum Stream_Module_HTMLParser_Mode         mode; // html parser module
  bool                                       printProgressDot; // file writer module
  bool                                       pushStatisticMessages;
  struct Net_SocketConfiguration*            socketConfiguration;
  struct Test_I_SocketHandlerConfiguration*  socketHandlerConfiguration;
  Test_I_StreamBase_t*                       stream;
  std::string                                targetFileName; // file writer module
  std::string                                URL; // HTTP get module
};

struct Stream_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Stream_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   //messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  //Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Test_I_StreamConfiguration
 : Stream_Configuration
{
  inline Test_I_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_ModuleHandlerConfiguration* moduleHandlerConfiguration;

  struct Test_I_UserData*                   userData;
};

struct Test_I_StreamState
 : Stream_State
{
  inline Test_I_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Stream_SessionData* currentSessionData;
  struct Test_I_UserData*           userData;
};

struct Test_I_Configuration
{
  inline Test_I_Configuration ()
   : signalHandlerConfiguration ()
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , connectionConfiguration ()
   , allocatorConfiguration ()
   , parserConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , useReactor (NET_EVENT_USE_REACTOR)
   , userData ()
  {};

  // **************************** signal data **********************************
  struct Stream_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Net_SocketConfiguration           socketConfiguration;
  struct Test_I_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_I_ConnectionConfiguration    connectionConfiguration;
  // **************************** stream data **********************************
  struct Stream_AllocatorConfiguration     allocatorConfiguration;
  struct Common_ParserConfiguration        parserConfiguration;
  struct Stream_ModuleConfiguration        moduleConfiguration;
  struct Test_I_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************
  bool                                     useReactor;

  struct Test_I_UserData                   userData;
};

typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_IStreamNotify_t;

#endif
