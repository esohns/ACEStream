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

#ifndef TEST_I_HTTP_GET_STREAM_COMMON_H
#define TEST_I_HTTP_GET_STREAM_COMMON_H

#include <list>
#include <map>
#include <string>

#include "libxml/tree.h"

#include "ace/config-macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "stream_module_db_common.h"

#include "stream_dec_common.h"
#include "stream_dec_defines.h"

#include "stream_module_htmlparser.h"

#include "http_common.h"
#include "http_defines.h"

#include "test_i_configuration.h"
#include "test_i_defines.h"

#include "test_i_http_get_network.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

struct Test_I_HTTPGet_MessageData
 : HTTP_Record
{
  Test_I_HTTPGet_MessageData ()
   : HTTP_Record ()
   , HTMLDocument (NULL)
  {}

  virtual ~Test_I_HTTPGet_MessageData ()
  {
    if (HTMLDocument)
      xmlFreeDoc (HTMLDocument);
  }
 inline void operator+= (struct Test_I_HTTPGet_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  xmlDocPtr HTMLDocument;
};
//typedef Stream_DataBase_T<struct Test_I_MessageData> Test_I_MessageData_t;

struct Test_I_DataItem
{
 Test_I_DataItem ()
  : description ()
  , URI ()
 {}

 inline bool operator== (struct Test_I_DataItem rhs_in) { return URI == rhs_in.URI; }

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
  Test_I_DataSet ()
   : pageData ()
  {}

  struct Test_I_DataSet& operator+= (const struct Test_I_DataSet& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    pageData.insert (rhs_in.pageData.begin (), rhs_in.pageData.end ());

    return *this;
  }

  Test_I_PageData_t pageData;
};
typedef std::list<struct Test_I_DataSet> Test_I_DataSets_t;
typedef Test_I_DataSets_t::const_iterator Test_I_DataSetsIterator_t;

enum Test_I_SAXParserState : int
{
  SAXPARSER_STATE_INVALID = -1,
  ////////////////////////////////////////
  SAXPARSER_STATE_IN_HEAD = 0,
  SAXPARSER_STATE_IN_HTML,
  SAXPARSER_STATE_IN_BODY,
  ////////////////////////////////////////
  SAXPARSER_STATE_READ_DATE_1,
  SAXPARSER_STATE_READ_DATE_2,
  ////////////////////////////////////////
  SAXPARSER_STATE_READ_ITEMS_1,
  SAXPARSER_STATE_READ_ITEMS_2,
  SAXPARSER_STATE_READ_ITEMS_3
};
struct Test_I_Stream_SessionData;
struct Test_I_SAXParserContext
 : Stream_Module_HTMLParser_SAXParserContextBase
{
  Test_I_SAXParserContext ()
   : Stream_Module_HTMLParser_SAXParserContextBase ()
   , sessionData (NULL)
   , dataItem ()
   , state (SAXPARSER_STATE_INVALID)
   , timeStamp ()
  {}

  struct Test_I_Stream_SessionData* sessionData;

  struct Test_I_DataItem            dataItem;
  enum Test_I_SAXParserState        state;
  ACE_Time_Value                    timeStamp;
};

struct Test_I_HTTPGet_ConnectionState;
struct Test_I_Stream_SessionData
 : Stream_SessionData
{
  Test_I_Stream_SessionData ()
   : Stream_SessionData ()
   , connection (NULL)
   , connectionStates ()
   , data ()
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , parserContext (NULL)
   , targetFileName ()
  {}

  struct Test_I_Stream_SessionData& operator+= (const struct Test_I_Stream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);
    connectionStates.insert (rhs_in.connectionStates.begin (),
                             rhs_in.connectionStates.end ());
    data += rhs_in.data;
    //format =
    parserContext = (parserContext ? parserContext : rhs_in.parserContext);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Net_IINETConnection_t*                    connection;
  Stream_Net_ConnectionStates_t             connectionStates;
  struct Test_I_DataSet                     data; // html handler module
  enum Stream_Decoder_CompressionFormatType format; // decompressor module
  struct Test_I_SAXParserContext*           parserContext; // html parser/handler module
  std::string                               targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Test_I_Stream_SessionData> Test_I_Stream_SessionData_t;

// forward declarations
struct Test_I_HTTPGet_Configuration;
struct Test_I_SocketHandlerConfiguration;
extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_HTTPGet_StreamConfiguration,
                               struct Test_I_HTTPGet_ModuleHandlerConfiguration> Test_I_StreamConfiguration_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_HTTPGet_StreamState,
                      struct Test_I_HTTPGet_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                      Test_I_SessionManager_t,
                      Stream_ControlMessage_t,
                      Test_I_Stream_Message,
                      Test_I_Stream_SessionMessage,
                      struct Stream_UserData> Test_I_StreamBase_t;
struct Test_I_HTTPGet_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_HTTPGet_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , closeAfterReception (HTTP_DEFAULT_CLOSE_AFTER_RECEPTION)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , dataBaseOptionsFileName ()
   , dataBaseTable ()
   , HTTPForm ()
   , HTTPHeaders ()
   , inbound (true)
   , loginOptions ()
   , mode (STREAM_MODULE_HTMLPARSER_MODE_SAX)
   , peerAddress ()
   , parserConfiguration (NULL)
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , streamConfiguration (NULL)
   , targetFileName ()
   , URL ()
   , waitForConnect (true)
  {
    //crunchMessages = HTTP_DEFAULT_CRUNCH_MESSAGES; // HTTP parser module
    passive = false;
  }

  bool                                             closeAfterReception; // HTTP get module
  Net_IINETConnection_t*                           connection; // TCP target/IO module
  Net_ConnectionConfigurations_t*                  connectionConfigurations;
  Test_I_Stream_InetConnectionManager_t*           connectionManager; // TCP IO module
  std::string                                      dataBaseOptionsFileName; // db writer module
  std::string                                      dataBaseTable; // db writer module
  HTTP_Form_t                                      HTTPForm; // HTTP get module
  HTTP_Headers_t                                   HTTPHeaders; // HTTP get module
  bool                                             inbound; // net io module
  struct Stream_Module_DataBase_LoginOptions_MySQL loginOptions; // db writer module
  enum Stream_Module_HTMLParser_Mode               mode; // html parser module
  ACE_INET_Addr                                    peerAddress; // db writer module
  struct HTTP_ParserConfiguration*                 parserConfiguration;      // parser module(s)
  bool                                             printProgressDot; // file writer module
  bool                                             pushStatisticMessages;
  Test_I_StreamConfiguration_t*                    streamConfiguration; // net source module
  std::string                                      targetFileName; // file writer module
  std::string                                      URL; // HTTP get module
  bool                                             waitForConnect;
};

struct Test_I_HTTPGet_StreamConfiguration
 : Stream_Configuration
{
  Test_I_HTTPGet_StreamConfiguration ()
   : Stream_Configuration ()
  {}
};

struct Test_I_HTTPGet_StreamState
 : Stream_State
{
  Test_I_HTTPGet_StreamState ()
   : Stream_State ()
  {}
};

//typedef Stream_IModuleHandler_T<Test_I_Stream_ModuleHandlerConfiguration> Test_I_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Stream_Message,
                                          Test_I_Stream_SessionMessage> Test_I_MessageAllocator_t;

//typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_IStreamNotify_t;

#endif
