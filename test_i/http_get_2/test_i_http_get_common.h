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

#ifndef TEST_I_HTTP_GET_COMMON_H
#define TEST_I_HTTP_GET_COMMON_H

#include <list>
#include <set>
#include <string>

#include "libxml/tree.h"

#include "stream_base.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"

#include "stream_dec_common.h"

#include "stream_document_defines.h"

#include "stream_module_htmlparser.h"

#include "http_common.h"
#include "http_defines.h"

#include "test_i_common.h"
#include "test_i_configuration.h"

#include "test_i_http_get_connection_common.h"
#include "test_i_http_get_defines.h"

struct Test_I_StockItem
{
  Test_I_StockItem ()
   : /*description ()
   ,*/ ISIN ()
   , symbol ()
   , WKN ()
   , isStock (true)
  {}
  inline bool operator== (const struct Test_I_StockItem& rhs_in) { return (ISIN == rhs_in.ISIN); }

  //std::string description;
  std::string ISIN;
  std::string symbol;
  std::string WKN;
  bool        isStock;
};

struct Test_I_StockRecord
{
  Test_I_StockRecord ()
   : change (0.0)
   , item (NULL)
   , timeStamp (ACE_Time_Value::zero)
   , value (0.0)
  {}
  inline bool operator== (const struct Test_I_StockRecord& rhs_in) { ACE_ASSERT (rhs_in.item); ACE_ASSERT (item); return (*item == *rhs_in.item); }

  double                   change;
  struct Test_I_StockItem* item;
  ACE_Time_Value           timeStamp;
  double                   value;
};

struct Test_I_HTTPGet_MessageData
 : HTTP_Record
{
  Test_I_HTTPGet_MessageData ()
   : HTTP_Record ()
   , HTMLDocument (NULL)
   , stockItem ()
  {}
  virtual ~Test_I_HTTPGet_MessageData ()
  {
    if (HTMLDocument)
      xmlFreeDoc (HTMLDocument);
  }
  inline void operator+= (Test_I_HTTPGet_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  xmlDocPtr               HTMLDocument;
  struct Test_I_StockItem stockItem;
};
//typedef Stream_DataBase_T<struct Test_I_HTTPGet_MessageData> Test_I_HTTPGet_MessageData_t;

struct less_stock_item
{
  inline bool operator () (const struct Test_I_StockItem& lhs_in, const struct Test_I_StockItem& rhs_in) const { return (lhs_in.ISIN < rhs_in.ISIN); }
};
typedef std::set<struct Test_I_StockItem, less_stock_item> Test_I_StockItems_t;
typedef Test_I_StockItems_t::iterator Test_I_StockItemsIterator_t;

typedef std::list<struct Test_I_StockRecord> Test_I_StockRecords_t;
typedef Test_I_StockRecords_t::const_iterator Test_I_StockRecordsIterator_t;

struct Test_I_HTTPGet_SessionData
 : Stream_SessionData
{
  Test_I_HTTPGet_SessionData ()
   : Stream_SessionData ()
   , connection (NULL)
   , connectionStates ()
   , data ()
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   //, parserContext (NULL)
   , targetFileName ()
  {}
  struct Test_I_HTTPGet_SessionData& operator+= (const struct Test_I_HTTPGet_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);
    connectionStates.insert (rhs_in.connectionStates.begin (),
                             rhs_in.connectionStates.end ());
    data.insert (data.end (), rhs_in.data.begin (), rhs_in.data.end ());
    //parserContext = (parserContext ? parserContext : rhs_in.parserContext);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    //userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Net_IINETConnection_t*                    connection;
  Stream_Net_ConnectionStates_t             connectionStates;
  Test_I_StockRecords_t                     data; // html parser/spreadsheet writer module
  enum Stream_Decoder_CompressionFormatType format; // decompressor module
  //Test_I_SAXParserContext*                  parserContext; // html parser/handler module
  std::string                               targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Test_I_HTTPGet_SessionData> Test_I_HTTPGet_SessionData_t;

enum Test_I_SAXParserState : int
{
  SAXPARSER_STATE_INVALID = -1,
  ////////////////////////////////////////
  SAXPARSER_STATE_IN_HTML = 0,
  ////////////////////////////////////////
  SAXPARSER_STATE_IN_HEAD,
  SAXPARSER_STATE_IN_BODY,
  ////////////////////////////////////////
  //SAXPARSER_STATE_IN_HEAD_TITLE,
  ////////////////////////////////////////
  SAXPARSER_STATE_IN_BODY_DIV_CONTENT,
  SAXPARSER_STATE_IN_TBODY_CONTENT,
  SAXPARSER_STATE_IN_TR_CONTENT,
  ////////////////////////////////////////
  SAXPARSER_STATE_READ_SYMBOL,
  SAXPARSER_STATE_READ_WKN,
  SAXPARSER_STATE_READ_ISIN,
  SAXPARSER_STATE_READ_CHANGE,
  SAXPARSER_STATE_READ_CHANGE_2,
  SAXPARSER_STATE_READ_DATE,
  SAXPARSER_STATE_READ_VALUE
};
struct Test_I_SAXParserContext
 : Stream_Module_HTMLParser_SAXParserContextBase
{
  Test_I_SAXParserContext ()
   : Stream_Module_HTMLParser_SAXParserContextBase ()
   , record (NULL)
   , state (SAXPARSER_STATE_INVALID)
  {}

  struct Test_I_StockRecord* record;
  enum Test_I_SAXParserState state;
};

//class Test_I_Stream_Message;
//class Test_I_Stream_SessionMessage;
//struct Test_I_HTTPGet_StreamConfiguration;
//struct Test_I_HTTPGet_ModuleHandlerConfiguration;
//static constexpr const char stream_name_string_[] =
//    ACE_TEXT_ALWAYS_CHAR ("HTTPGetStream");
//typedef Stream_Configuration_T<stream_name_string_,
//                               struct Common_Parser_FlexAllocatorConfiguration,
//                               struct Test_I_HTTPGet_StreamConfiguration,
//                               struct Stream_ModuleConfiguration,
//                               struct Test_I_HTTPGet_ModuleHandlerConfiguration> Test_I_HTTPGet_StreamConfiguration_t;
//typedef Stream_Base_T<ACE_MT_SYNCH,
//                      Common_TimePolicy_t,
//                      stream_name_string_,
//                      enum Stream_ControlType,
//                      enum Stream_SessionMessageType,
//                      enum Stream_StateMachine_ControlState,
//                      struct Test_I_HTTPGet_StreamState,
//                      struct Test_I_HTTPGet_StreamConfiguration,
//                      Test_I_Statistic_t,
//                      Test_I_StatisticHandlerReactor_t,
//                      struct Common_Parser_FlexAllocatorConfiguration,
//                      struct Stream_ModuleConfiguration,
//                      struct Test_I_HTTPGet_ModuleHandlerConfiguration,
//                      struct Test_I_HTTPGet_SessionData,
//                      Test_I_HTTPGet_SessionData_t,
//                      Stream_ControlMessage_t,
//                      Test_I_Stream_Message,
//                      Test_I_Stream_SessionMessage> Test_I_StreamBase_t;
struct Test_I_HTTPGet_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_HTTPGet_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , allocatorConfiguration (NULL)
   , closeAfterReception (HTTP_DEFAULT_CLOSE_AFTER_RECEPTION)
   , configuration (NULL)
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , fileName ()
   , HTTPForm ()
   , HTTPHeaders ()
   , libreOfficeHost (TEST_I_DEFAULT_PORT,
                      ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
                      AF_INET)
   , libreOfficeRc ()
   , libreOfficeSheetStartColumn (0)
   , libreOfficeSheetStartRow (TEST_I_DEFAULT_LIBREOFFICE_START_ROW - 1)
   , mode (STREAM_MODULE_HTMLPARSER_MODE_SAX)
   , parserConfiguration (NULL)
   , stockItems ()
   , streamConfiguration (NULL)
   , URL ()
   , waitForConnect (true)
  {}

  struct Common_Parser_FlexAllocatorConfiguration* allocatorConfiguration;
  bool                                             closeAfterReception;      // HTTP get module
  struct Test_I_HTTPGet_Configuration*             configuration;
  Net_IINETConnection_t*                           connection; // net source/IO module
  Net_ConnectionConfigurations_t*                  connectionConfigurations;
  Test_I_HTTPGet_InetConnectionManager_t*          connectionManager; // net source/IO module
  std::string                                      fileName; // spreadsheet writer module
  HTTP_Form_t                                      HTTPForm; // HTTP get module
  HTTP_Headers_t                                   HTTPHeaders; // HTTP get module
  ACE_INET_Addr                                    libreOfficeHost; // spreadsheet writer module
  std::string                                      libreOfficeRc; // spreadsheet writer module
  unsigned int                                     libreOfficeSheetStartColumn; // spreadsheet writer module
  unsigned int                                     libreOfficeSheetStartRow; // spreadsheet writer module
  enum Stream_Module_HTMLParser_Mode               mode; // HTML parser module
  struct HTTP_ParserConfiguration*                 parserConfiguration;
  Test_I_StockItems_t                              stockItems; // HTTP get module
  Test_I_HTTPGet_StreamConfiguration_t*            streamConfiguration; // net source module
  std::string                                      URL; // HTTP get module
  bool                                             waitForConnect; // HTTP get module
};
//typedef std::map<std::string,
//                 struct Test_I_HTTPGet_ModuleHandlerConfiguration> Test_I_HTTPGet_ModuleHandlerConfigurations_t;
//typedef Test_I_HTTPGet_ModuleHandlerConfigurations_t::const_iterator Test_I_HTTPGet_ModuleHandlerConfigurationsConstIterator_t;

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
   , sessionData (NULL)
  {}

  struct Test_I_HTTPGet_SessionData* sessionData;
};

//typedef std::map<std::string,
//                 Test_I_HTTPGet_ConnectionConfiguration_t> Test_I_HTTPGet_ConnectionConfigurations_t;
//typedef Test_I_HTTPGet_ConnectionConfigurations_t::iterator Test_I_HTTPGet_ConnectionConfigurationIterator_t;

struct Test_I_HTTPGet_Configuration
 : Test_I_Configuration
{
  Test_I_HTTPGet_Configuration ()
   : Test_I_Configuration ()
   , parserConfiguration ()
   , connectionConfigurations ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
  {}

  // **************************** parser data **********************************
  struct HTTP_ParserConfiguration      parserConfiguration;
  Net_ConnectionConfigurations_t       connectionConfigurations;
  Test_I_HTTPGet_StreamConfiguration_t streamConfiguration;
  Test_I_HTTPGet_StreamConfiguration_t streamConfiguration_2; // connection-
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Stream_Message,
                                          Test_I_Stream_SessionMessage> Test_I_MessageAllocator_t;

#endif
