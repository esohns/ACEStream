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

#ifndef TEST_U_STREAM_COMMON_H
#define TEST_U_STREAM_COMMON_H

#include <list>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_file_common.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "stream_document_defines.h"

#include "document_defines.h"
#include "test_u_session_message.h"

// forward declarations
struct Test_U_AllocatorConfiguration;
class Test_U_Message;
//class Test_U_SessionMessage;

struct Test_U_MessageData
{
  Test_U_MessageData ()
  {}
  virtual ~Test_U_MessageData ()
  {}
  inline void operator+= (struct Test_U_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};
typedef Stream_DataBase_T<struct Test_U_MessageData> Test_U_MessageData_t;

//struct Test_U_SessionData
// : Stream_SessionData
//{
//  Test_U_SessionData ()
//   : Stream_SessionData ()
//  {}

//  struct Test_U_SessionData& operator+= (const struct Test_U_SessionData& rhs_in)
//  {
//    // *NOTE*: the idea is to 'merge' the data
//    Stream_SessionData::operator+= (rhs_in);

//    return *this;
//  }
//};
//typedef Stream_SessionData_T<struct Test_U_SessionData> Test_U_SessionData_t;

typedef Stream_ISessionDataNotify_T<struct Test_U_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_Notification_t;
typedef std::list<Test_U_Notification_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;

// forward declarations
//extern const char stream_name_string_[];
struct Test_U_Document_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_Configuration,
                               struct Test_U_Document_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
//typedef Net_ConnectionConfiguration_T<struct Common_AllocatorConfiguration,
//                                      Branch_StreamConfiguration_t,
//                                      NET_TRANSPORTLAYER_TCP> Branch_ConnectionConfiguration_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          Branch_ConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Branch_IConnection_t;
//typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
//                                 ACE_INET_Addr,
//                                 Branch_ConnectionConfiguration_t,
//                                 struct Net_StreamConnectionState,
//                                 Net_StreamStatistic_t,
//                                 struct Net_UserData> Branch_ConnectionManager_t;
struct Test_U_Document_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_Document_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , fileIdentifier ()
   , fileName ()
   , libreOfficeHost (STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_SERVER_PORT,
                      ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
                      AF_INET)
   , libreOfficeRc ()
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , queue (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

  Common_File_Identifier        fileIdentifier;
  std::string                   fileName;
  ACE_INET_Addr                 libreOfficeHost; // spreadsheet writer module
  std::string                   libreOfficeRc; // spreadsheet writer module
  bool                          printProgressDot;         // file writer module
  bool                          pushStatisticMessages;
  ACE_Message_Queue_Base*       queue;
  Test_U_StreamConfiguration_t* streamConfiguration;      // net source module
  Test_U_Notification_t*        subscriber;
  Test_U_Subscribers_t*         subscribers;
};

struct Test_U_StreamState
 : Stream_State
{
  Test_U_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   //, userData (NULL)
  {}

  struct Test_U_SessionData* sessionData;

  //struct Branch_UserData*    userData;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

#endif
