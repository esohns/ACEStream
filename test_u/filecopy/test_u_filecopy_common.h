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

#ifndef TEST_U_FILECOPY_COMMON_H
#define TEST_U_FILECOPY_COMMON_H

#include <list>
#include <map>
#include <string>

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "common_isubscribe.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "test_u_common.h"
#include "test_u_gtk_common.h"

// forward declarations
class Stream_IAllocator;
class Stream_Filecopy_Stream;
class Stream_Filecopy_Message;
class Stream_Filecopy_SessionMessage;

struct Stream_Filecopy_SessionData;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Stream_Filecopy_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_Filecopy_Message,
                                    Stream_Filecopy_SessionMessage> Stream_Filecopy_ISessionNotify_t;
typedef std::list<Stream_Filecopy_ISessionNotify_t*> Stream_Filecopy_Subscribers_t;
typedef Stream_Filecopy_Subscribers_t::iterator Stream_Filecopy_SubscribersIterator_t;

extern const char stream_name_string_[];
struct Stream_Filecopy_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_Configuration,
                               struct Stream_ModuleConfiguration,
                               struct Stream_Filecopy_ModuleHandlerConfiguration> Stream_Filecopy_StreamConfiguration_t;
struct Stream_Filecopy_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  inline Stream_Filecopy_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
  {};

  Stream_Filecopy_ISessionNotify_t* subscriber;  // (initial) subscriber
  Stream_Filecopy_Subscribers_t*    subscribers;
  std::string                       targetFileName;
};

struct Stream_Filecopy_SessionData
 : Stream_SessionData
{
  inline Stream_Filecopy_SessionData ()
   : Stream_SessionData ()
   , fileName ()
   , size (0)
   , targetFileName ()
  {};

  std::string  fileName;
  unsigned int size;
  std::string  targetFileName;
};
typedef Stream_SessionData_T<struct Stream_Filecopy_SessionData> Stream_Filecopy_SessionData_t;

struct Stream_Filecopy_SignalHandlerConfiguration
{
  inline Stream_Filecopy_SignalHandlerConfiguration ()
   : actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistics collecting interval (second(s)) [0: off]
};

struct Stream_Filecopy_Configuration
{
  inline Stream_Filecopy_Configuration ()
   : signalHandlerConfiguration ()
   , streamConfiguration ()
   , streamUserData ()
  {};

  struct Stream_Filecopy_SignalHandlerConfiguration signalHandlerConfiguration;
  Stream_Filecopy_StreamConfiguration_t             streamConfiguration;

  struct Stream_UserData                            streamUserData;
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_U_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_Filecopy_Message,
                                          Stream_Filecopy_SessionMessage> Stream_Filecopy_MessageAllocator_t;

typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_Filecopy_IStreamNotify_t;

typedef Common_ISubscribe_T<Stream_Filecopy_ISessionNotify_t> Stream_Filecopy_ISubscribe_t;

//////////////////////////////////////////

typedef std::map<ACE_thread_t, guint> Stream_Filecopy_PendingActions_t;
typedef Stream_Filecopy_PendingActions_t::iterator Stream_Filecopy_PendingActionsIterator_t;
typedef std::set<ACE_thread_t> Stream_Filecopy_CompletedActions_t;
typedef Stream_Filecopy_CompletedActions_t::iterator Stream_Filecopy_CompletedActionsIterator_t;
struct Stream_Filecopy_GTK_ProgressData
{
  inline Stream_Filecopy_GTK_ProgressData ()
   : completedActions ()
   , copied (0)
//   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
   , size (0)
  {};

  Stream_Filecopy_CompletedActions_t completedActions;
  size_t                             copied; // bytes
//  GdkCursorType                      cursorType;
  struct Common_UI_GTKState*         GTKState;
  Stream_Filecopy_PendingActions_t   pendingActions;
  size_t                             size; // bytes
};

struct Stream_Filecopy_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Stream_Filecopy_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , progressData ()
   , stream (NULL)
   , subscribers ()
  {};

  struct Stream_Filecopy_Configuration*   configuration;
  struct Stream_Filecopy_GTK_ProgressData progressData;
  Stream_Filecopy_Stream*                 stream;
  Stream_Filecopy_Subscribers_t           subscribers;
};

struct Stream_Filecopy_ThreadData
{
  inline Stream_Filecopy_ThreadData ()
   : CBData (NULL)
   , sessionID (0)
  {};

  struct Stream_Filecopy_GTK_CBData* CBData;
  size_t                             sessionID;
};

typedef Common_UI_GtkBuilderDefinition_T<struct Stream_Filecopy_GTK_CBData> Stream_Filecopy_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Stream_Filecopy_GTK_CBData> Stream_Filecopy_GTK_Manager_t;
typedef ACE_Singleton<Stream_Filecopy_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> FILECOPY_UI_GTK_MANAGER_SINGLETON;

#endif
