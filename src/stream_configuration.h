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

#ifndef STREAM_CONFIGURATION_H
#define STREAM_CONFIGURATION_H

#include <map>
#include <string>
#include <utility>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfobjects.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_configuration.h"
#include "common_defines.h"
#include "common_idumpstate.h"
#include "common_iinitialize.h"

#include "common_timer_common.h"

#include "stream_common.h"
#include "stream_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#else
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
class ACE_Notification_Strategy;
class Stream_IAllocator;

struct Stream_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Stream_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
};

struct Stream_AllocatorConfiguration
 : Common_AllocatorConfiguration
{
  Stream_AllocatorConfiguration ()
   : Common_AllocatorConfiguration ()
  {
    defaultBufferSize = STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE;
  }
};

struct Common_ParserConfiguration;
class Stream_IOutboundDataNotify;
struct Stream_ModuleHandlerConfiguration
{
  Stream_ModuleHandlerConfiguration ()
   : allocatorConfiguration (NULL)
   , concurrency (STREAM_HEADMODULECONCURRENCY_PASSIVE)
   , crunchMessages (STREAM_MODULE_DEFAULT_CRUNCH_MESSAGES)
#if defined (_DEBUG)
   , debug (false)
#endif // _DEBUG
   , demultiplex (false)
   , finishOnDisconnect (false)
   , hasHeader (false)
   , hasReentrantSynchronousSubDownstream (true)
   , inbound (false)
   , lock (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , messageAllocator (NULL)
   , outboundNotificationHandle (NULL)
   , parserConfiguration (NULL)
   , passive (true)
   , printFinalReport (false)
   , pushStatisticMessages (true)
   , reportingInterval (0)
   , slurpFiles (false)
   , socketHandle (ACE_INVALID_HANDLE)
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , statisticReportingInterval (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
   , timerManager (NULL)
  {}

  struct Stream_AllocatorConfiguration* allocatorConfiguration;
  enum Stream_HeadModuleConcurrency     concurrency;                          // head module(s)
  // *NOTE*: this option may be useful for (downstream) modules that only work
  //         on CONTIGUOUS buffers (i.e. cannot parse chained message blocks)
  bool                                  crunchMessages;
#if defined (_DEBUG)
  bool                                  debug;
#endif // _DEBUG
  bool                                  demultiplex;                          // message handler module
  bool                                  finishOnDisconnect;                   // header module(s)
  bool                                  hasHeader;
  // *WARNING*: when false, this 'locks down' the pipeline head module; i.e. it
  //            will hold the 'stream lock' during all message processing to
  //            support (down)stream synchronization. This really only makes
  //            sense in fully synchronous layouts with asynchronous sources, or
  //            'concurrent' scenarios, with non-reentrant modules. Note that
  //            this overhead is not negligible
  //            --> disable only if absolutely necessary
  bool                                  hasReentrantSynchronousSubDownstream; // head module(s)
  bool                                  inbound;                              // statistic[/IO] module(s)
  ACE_SYNCH_RECURSIVE_MUTEX*            lock;                                 // display/message handler module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type       mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  Stream_IAllocator*                    messageAllocator;
  Stream_IOutboundDataNotify*           outboundNotificationHandle;           // IO module(s)
  struct Common_ParserConfiguration*    parserConfiguration;                  // parser module(s)
  bool                                  passive;                              // network/device/... module(s)
  bool                                  printFinalReport;                     // statistic module(s)
  bool                                  pushStatisticMessages;                // source/statistic/... module(s)
  unsigned int                          reportingInterval;                    // (statistic) reporting interval (second(s)) [0: off]
  bool                                  slurpFiles;                           // file source module(s)
  ACE_HANDLE                            socketHandle;                         // network module(s)
  ACE_Time_Value                        statisticCollectionInterval;          // source/statistic/... module(s)
  ACE_Time_Value                        statisticReportingInterval;           // [ACE_Time_Value::zero: off]
  Common_ITimer_t*                      timerManager;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_DirectShow_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
   Stream_DirectShow_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , outputFormat ()
  {}

  struct _AMMediaType outputFormat;
};

struct Stream_MediaFoundation_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
   Stream_MediaFoundation_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , outputFormat (NULL)
  {}

  IMFMediaType* outputFormat;
};
#else
struct Stream_V4L_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
   Stream_V4L_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , outputFormat ()
  {}

  struct Stream_MediaFramework_V4L_MediaType outputFormat;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_Configuration;
struct Stream_ModuleConfiguration
{
  Stream_ModuleConfiguration ()
   : generateUniqueNames (false) // module-
   , notify (NULL)
   , stream (NULL)
  {}

  bool              generateUniqueNames;
  Stream_INotify_t* notify; // *WARNING*: automatically set; DON'T TOUCH
  // *NOTE*: modules can use this to temporarily relinquish the stream lock
  //         while they wait on some condition, in order to avoid deadlocks
  //         --> to be used primarily in 'non-concurrent' (see above) scenarios
  Stream_IStream_t* stream; // *WARNING*: automatically set; DON'T TOUCH
};

struct Stream_Configuration
{
  Stream_Configuration ()
   : branches ()
   , cloneModule (false) // *NOTE*: cloneModule ==> deleteModule
   , deleteModule (false)
   , finishOnDisconnect (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , messageAllocator (NULL)
   , module (NULL)
   , moduleBranch ()
   , notificationStrategy (NULL)
   , printFinalReport (false)
   , resetSessionData (true)
   , serializeOutput (false)
   , sessionId (0)
   , setupPipeline (true)
   , userData (NULL)
  {}

  Stream_Branches_t               branches; // distributor(s) *TODO*
  bool                            cloneModule; // final-
  bool                            deleteModule; // final-
  bool                            finishOnDisconnect; // (network) i/o streams
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  Stream_IAllocator*              messageAllocator;
  Stream_Module_t*                module; // final-
  std::string                     moduleBranch; // final- {"": main branch}
  ACE_Notification_Strategy*      notificationStrategy;
  bool                            printFinalReport;
  bool                            resetSessionData;
  // *IMPORTANT NOTE*: in a multi-threaded environment, threads MAY be
  //                   dispatching the reactor notification queue concurrently
  //                   (most notably, ACE_TP_Reactor)
  //                   --> enforce proper serialization
  bool                            serializeOutput;
  Stream_SessionId_t              sessionId;
  bool                            setupPipeline;

  struct Stream_UserData*         userData;
};

template <//const char* StreamName,
          ////////////////////////////////
          typename AllocatorConfigurationType,
          typename ConfigurationType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Stream_Configuration_T
 : public std::map<std::string,                                // key:   module name
                   std::pair<ModuleConfigurationType,
                             ModuleHandlerConfigurationType> > // value: (pair of) module/handler configuration
 , public Common_IDumpState
{
  typedef std::map<std::string,
                   std::pair<ModuleConfigurationType,
                             ModuleHandlerConfigurationType> > inherited;

 public:
  // convenient types
  typedef std::map<std::string,
                   std::pair<ModuleConfigurationType,
                             ModuleHandlerConfigurationType> > MAP_T;
  typedef typename MAP_T::iterator ITERATOR_T;
  typedef typename MAP_T::const_iterator CONST_ITERATOR_T;

  Stream_Configuration_T ();
  inline virtual ~Stream_Configuration_T () {}

  bool initialize (const ModuleConfigurationType&,        // 'default' module configuration
                   const ModuleHandlerConfigurationType&, // 'default' module handler configuration
                   const AllocatorConfigurationType&,
                   const ConfigurationType&);

  virtual void dump_state () const;

  AllocatorConfigurationType allocatorConfiguration_;
  ConfigurationType          configuration_;
  bool                       isInitialized_;
};

// include template definition
#include "stream_configuration.inl"

//////////////////////////////////////////

//extern const char empty_string_[];
typedef Stream_Configuration_T<//empty_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_Configuration,
                               struct Stream_ModuleConfiguration,
                               struct Stream_ModuleHandlerConfiguration> Stream_Configuration_t;

#endif
