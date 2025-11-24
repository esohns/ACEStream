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

#ifndef PARSER_STREAM_COMMON_H
#define PARSER_STREAM_COMMON_H

#include <list>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_parser_common.h"

#include "common_parser_bencoding_common.h"
#include "common_parser_bencoding_tools.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"
#include "stream_session_manager.h"

// forward declarations
struct Parser_AllocatorConfiguration;
class Parser_Message;
class Parser_SessionMessage;

struct Parser_MessageData
{
  Parser_MessageData ()
   : element ()
  {}
  ~Parser_MessageData ()
  {
    Common_Parser_Bencoding_Tools::free (element);
  }

  inline void operator+= (struct Parser_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  // inline operator Bencoding_Dictionary_t&() const { ACE_ASSERT (dictionary); return *dictionary; }

  struct Bencoding_Element element;
};
typedef Stream_DataBase_T<struct Parser_MessageData> Parser_MessageData_t;

struct Parser_SessionData
 : Stream_SessionData
{
  Parser_SessionData ()
   : Stream_SessionData ()
   //, format (STREAM_COMPRESSION_FORMAT_INVALID)
   //, targetFileName ()
  {};

  struct Parser_SessionData& operator+= (const struct Parser_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    //format =
    //targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
    //                                          : targetFileName);

    return *this;
  }

  //enum Stream_Decoder_CompressionFormatType format; // decompressor module
  //std::string                               targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Parser_SessionData> Parser_SessionData_t;

typedef Stream_ISessionDataNotify_T<struct Parser_SessionData,
                                    enum Stream_SessionMessageType,
                                    Parser_Message,
                                    Parser_SessionMessage> Parser_Notification_t;
typedef std::list<Parser_Notification_t*> Parser_Subscribers_t;
typedef Parser_Subscribers_t::iterator Parser_SubscribersIterator_t;

// forward declarations
//extern const char stream_name_string_[];
struct Parser_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_Configuration,
                               struct Parser_ModuleHandlerConfiguration> Parser_StreamConfiguration_t;
//typedef Net_ConnectionConfiguration_T<struct Common_Parser_FlexAllocatorConfiguration,
//                                      Parser_StreamConfiguration_t,
//                                      NET_TRANSPORTLAYER_TCP> Parser_ConnectionConfiguration_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          Parser_ConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Parser_IConnection_t;
//typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
//                                 ACE_INET_Addr,
//                                 Parser_ConnectionConfiguration_t,
//                                 struct Net_StreamConnectionState,
//                                 Net_StreamStatistic_t,
//                                 struct Net_UserData> Parser_ConnectionManager_t;
struct Parser_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  Parser_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , parserConfiguration (NULL)
   //, inbound (true)
   //, printProgressDot (false)
   , pushStatisticMessages (true)
   , queue (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   //, targetFileName ()
  {
    //crunchMessages = HTTP_DEFAULT_CRUNCH_MESSAGES; // HTTP parser module
    //passive = false;
  };

  struct Common_FlexBisonParserConfiguration* parserConfiguration;
  //bool                            inbound;                  // net io module
  //bool                            printProgressDot;         // file writer module
  bool                               pushStatisticMessages;
  ACE_Message_Queue_Base*            queue;
  Parser_StreamConfiguration_t*      streamConfiguration;      // net source module
  Parser_Notification_t*             subscriber;
  Parser_Subscribers_t*              subscribers;
  //std::string                     targetFileName;           // file writer module
};

struct Parser_StreamState
 : Stream_State
{
  Parser_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   //, userData (NULL)
  {};

  struct Parser_SessionData* sessionData;

  //struct Parser_UserData*    userData;
};

//////////////////////////////////////////

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Parser_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

#endif
