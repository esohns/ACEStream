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

#include "stream_control_message.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"
//#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
//#include "test_i_message.h"
//#include "test_i_session_message.h"

#include "test_i_http_get_connection_manager_common.h"

struct Test_I_StockItem
{
  inline Test_I_StockItem ()
   : /*description ()
   ,*/ ISIN ()
   , symbol ()
   , WKN ()
   , isStock (true)
  {};

  inline bool operator== (const Test_I_StockItem& rhs_in)
  {
    return (ISIN == rhs_in.ISIN);
  };

  //std::string description;
  std::string ISIN;
  std::string symbol;
  std::string WKN;
  bool isStock;
};

struct Test_I_StockRecord
{
  inline Test_I_StockRecord ()
   : change (0.0)
   , item (NULL)
   , timeStamp (ACE_Time_Value::zero)
   , value (0.0)
  {};

  inline bool operator== (const Test_I_StockRecord& rhs_in)
  {
    // sanity check(s)
    ACE_ASSERT (rhs_in.item);
    ACE_ASSERT (item);

    return (*item == *rhs_in.item);
  };

  double            change;
  Test_I_StockItem* item;
  ACE_Time_Value    timeStamp;
  double            value;
};

struct Test_I_MessageData
{
  inline Test_I_MessageData ()
   : HTTPRecord (NULL)
   , HTMLDocument (NULL)
   , stockItem ()
  {};
  inline ~Test_I_MessageData ()
  {
    if (HTTPRecord)
      delete HTTPRecord;
    if (HTMLDocument)
      xmlFreeDoc (HTMLDocument);
  };

  HTTP_Record*     HTTPRecord;
  xmlDocPtr        HTMLDocument;
  Test_I_StockItem stockItem;
};
typedef Stream_DataBase_T<Test_I_MessageData> Test_I_MessageData_t;

struct less_stock_item
{
  bool operator () (const struct Test_I_StockItem& lhs_in,
                    const struct Test_I_StockItem& rhs_in) const
  {
    return (lhs_in.ISIN < rhs_in.ISIN);
  }
};
typedef std::set<Test_I_StockItem, less_stock_item> Test_I_StockItems_t;
typedef Test_I_StockItems_t::iterator Test_I_StockItemsIterator_t;

typedef std::list<Test_I_StockRecord> Test_I_StockRecords_t;
typedef Test_I_StockRecords_t::const_iterator Test_I_StockRecordsIterator_t;

struct Test_I_HTTPGet_Configuration;
struct Test_I_HTTPGet_StreamConfiguration;
struct Test_I_HTTPGet_UserData
 : Stream_UserData
{
  inline Test_I_HTTPGet_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_HTTPGet_Configuration*       configuration;
  Test_I_HTTPGet_StreamConfiguration* streamConfiguration;
};

struct Test_I_HTTPGet_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_HTTPGet_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  Test_I_HTTPGet_UserData* userData;
};

struct Test_I_Stream_SessionData
 : Stream_SessionData
{
  inline Test_I_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , data ()
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   //, parserContext (NULL)
   , targetFileName ()
   , userData (NULL)
  {};

  inline Test_I_Stream_SessionData& operator+= (const Test_I_Stream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connectionState =
      (connectionState ? connectionState : rhs_in.connectionState);
    data.insert (data.end (), rhs_in.data.begin (), rhs_in.data.end ());
    //parserContext = (parserContext ? parserContext : rhs_in.parserContext);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    //userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_I_ConnectionState*                   connectionState;
  Test_I_StockRecords_t                     data; // html parser/spreadsheet writer module
  enum Stream_Decoder_CompressionFormatType format; // decompressor module
  //Test_I_SAXParserContext*                  parserContext; // html parser/handler module
  std::string                               targetFileName; // file writer module
  Test_I_HTTPGet_UserData*                  userData;
};
typedef Stream_SessionData_T<Test_I_Stream_SessionData> Test_I_Stream_SessionData_t;

struct Test_I_Stream_State
 : Stream_State
{
  inline Test_I_Stream_State ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_I_Stream_SessionData* currentSessionData;
  Test_I_HTTPGet_UserData*   userData;
};

enum Test_I_SAXParserState
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
  SAXPARSER_STATE_IN_SYMBOL_H1_CONTENT,
  ////////////////////////////////////////
  SAXPARSER_STATE_READ_CHANGE,
  SAXPARSER_STATE_READ_DATE,
  SAXPARSER_STATE_READ_ISIN_WKN,
  SAXPARSER_STATE_READ_SYMBOL,
  SAXPARSER_STATE_READ_VALUE
};
struct Test_I_SAXParserContext
 : Stream_Module_HTMLParser_SAXParserContextBase
{
  inline Test_I_SAXParserContext ()
   : Stream_Module_HTMLParser_SAXParserContextBase ()
   , data (NULL)
   , state (SAXPARSER_STATE_INVALID)
  {};

  Test_I_StockRecord*   data;
  Test_I_SAXParserState state;
};

struct Test_I_HTTPGet_StreamConfiguration;
struct Test_I_HTTPGet_ModuleHandlerConfiguration;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      int,
                      Stream_SessionMessageType,
                      Stream_StateMachine_ControlState,
                      Test_I_Stream_State,
                      Test_I_HTTPGet_StreamConfiguration,
                      Test_I_RuntimeStatistic_t,
                      Stream_ModuleConfiguration,
                      Test_I_HTTPGet_ModuleHandlerConfiguration,
                      Test_I_Stream_SessionData,   // session data
                      Test_I_Stream_SessionData_t, // session data container (reference counted)
                      ACE_Message_Block,
                      Test_I_Stream_Message,
                      Test_I_Stream_SessionMessage> Test_I_StreamBase_t;
struct Test_I_HTTPGet_ModuleHandlerConfiguration
 : Test_I_Stream_ModuleHandlerConfiguration
{
  inline Test_I_HTTPGet_ModuleHandlerConfiguration ()
   : Test_I_Stream_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , connection (NULL)
   , connectionManager (NULL)
   , fileName ()
   , HTTPForm ()
   , HTTPHeaders ()
//, hostName ()
   , libreOfficeHost (TEST_I_DEFAULT_PORT,
                      ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
                      ACE_ADDRESS_FAMILY_INET)
   , libreOfficeRc ()
   , libreOfficeSheetStartColumn (0)
   , libreOfficeSheetStartRow (TEST_I_DEFAULT_LIBREOFFICE_START_ROW - 1)
   , socketHandlerConfiguration (NULL)
   , stockItems ()
   , stream (NULL)
   , URL ()
  {};

  Test_I_HTTPGet_Configuration*              configuration;
  Test_I_IConnection_t*                      connection; // net source/IO module
  Test_I_HTTPGet_InetConnectionManager_t*    connectionManager; // net source/IO module
  std::string                                fileName; // spreadsheet writer module
  HTTP_Form_t                                HTTPForm; // HTTP get module
  HTTP_Headers_t                             HTTPHeaders; // HTTP get module
  ACE_INET_Addr                              libreOfficeHost; // spreadsheet writer module
  std::string                                libreOfficeRc; // spreadsheet writer module
  unsigned int                               libreOfficeSheetStartColumn; // spreadsheet writer module
  unsigned int                               libreOfficeSheetStartRow; // spreadsheet writer module
  Test_I_HTTPGet_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_I_StockItems_t                        stockItems; // HTTP get module
  Test_I_StreamBase_t*                       stream; // net source module
  std::string                                URL; // HTTP get module
};

struct Test_I_HTTPGet_StreamConfiguration
 : Test_I_Stream_Configuration
{
  inline Test_I_HTTPGet_StreamConfiguration ()
   : Test_I_Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_I_HTTPGet_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Test_I_HTTPGet_Configuration
 : Test_I_Configuration
{
  inline Test_I_HTTPGet_Configuration ()
   : Test_I_Configuration ()
   , socketHandlerConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  Test_I_HTTPGet_SocketHandlerConfiguration socketHandlerConfiguration;
  Test_I_HTTPGet_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_I_HTTPGet_StreamConfiguration        streamConfiguration;
  Test_I_HTTPGet_UserData                   userData;
};

typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Test_I_AllocatorConfiguration,
                                Test_I_Stream_Message,
                                Test_I_Stream_SessionMessage> Test_I_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<Test_I_AllocatorConfiguration,
                                          Test_I_ControlMessage_t,
                                          Test_I_Stream_Message,
                                          Test_I_Stream_SessionMessage> Test_I_MessageAllocator_t;

#endif
