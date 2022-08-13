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
// *WORKAROUND*: mfobjects.h includes cguid.h, which requires this
#define __CGUID_H__
#include "ks.h"
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_configuration.h"
#include "common_defines.h"
#include "common_idumpstate.h"
#include "common_iinitialize.h"

#include "common_timer_common.h"

#include "common_signal_common.h"

#include "stream_common.h"
#include "stream_defines.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"
#include "stream_session_message_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#else
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
class ACE_Notification_Strategy;
class Stream_IAllocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMFMediaType;
#endif // ACE_WIN32 || ACE_WIN64

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

struct Common_FlexBisonParserConfiguration;
struct Stream_ModuleHandlerConfiguration
{
  Stream_ModuleHandlerConfiguration ()
   : allocatorConfiguration (NULL)
   , autoStart (false)
   , computeThroughput (false)
   , concurrency (STREAM_HEADMODULECONCURRENCY_PASSIVE)
#if defined (_DEBUG)
   , debug (false)
#endif // _DEBUG
   , defragmentMode (STREAM_DEFRAGMENT_INVALID)
   , demultiplex (false)
   , finishOnDisconnect (false)
   , flipImage (false)
   , generateSessionMessages (true)
   , hasReentrantSynchronousSubDownstream (true)
   , inbound (true)
   , lock (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , messageAllocator (NULL)
   , outboundNotificationHandle (NULL)
   , parserConfiguration (NULL)
   //, passData (true)
   , passive (true)
   , printFinalReport (false)
   , reportingInterval (0)
   , slurpFiles (false)
   , socketHandle (ACE_INVALID_HANDLE)
   , splitOnStep (false)
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , statisticReportingInterval (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S, 0)
   , timerManager (NULL)
  {}

  struct Common_AllocatorConfiguration*       allocatorConfiguration;
  bool                                        autoStart;                            // head module(s)
  bool                                        computeThroughput;                    // statistic/... module(s)
  // *NOTE*: valid operating modes (see also: put()):
  //         active    : dedicated worker thread(s) running svc()
  //         concurrent: in-line processing (i.e. concurrent put(), no workers)
  //                     [Data is supplied externally, e.g. event dispatch]
  //         passive   : in-line (invokes svc() on start())
  //                     [Note that in this case, stream processing is already
  //                     finished once the thread returns from start(), i.e.
  //                     there is no point in calling wait().]
  enum Stream_HeadModuleConcurrency           concurrency;                          // head module(s)
#if defined (_DEBUG)
  bool                                        debug;
#endif // _DEBUG
  enum Stream_MessageDefragmentMode           defragmentMode;                       // defragment module
  bool                                        demultiplex;                          // message handler module
//  struct Common_EventDispatchConfiguration*   dispatchConfiguration;
  bool                                        finishOnDisconnect;                   // head module(s)
  bool                                        flipImage;                            // ffmpeg (converter)
  bool                                        generateSessionMessages;              // head module(s)
  // *WARNING*: when false, this 'locks down' the pipeline head module; i.e. it
  //            will hold the 'stream lock' during all message processing to
  //            support (down)stream synchronization. This really only makes
  //            sense in fully synchronous layouts with asynchronous sources, or
  //            'concurrent' scenarios, with non-reentrant modules. Note that
  //            this overhead is not negligible
  //            --> disable only if absolutely necessary
  // *NOTE*: applies to the concurrent/synchronous sub-downstream (i.e. the
  //         sub-stream until the next asynchronous module). If disabled, this
  //         enforces that all messages pass through the sub-stream strictly
  //         sequentially. This may be necessary in asynchronously-supplied
  //         (i.e. 'concurrent') usage scenarios with non-reentrant modules
  //         (i.e. most 'synchronous' modules that maintain some kind of
  //         internal state, such as e.g. push-parsers), or streams that react
  //         to asynchronous events (such as connection resets, user aborts,
  //         signals, etc). Threads will then hold the 'stream lock' during
  //         message processing to support (down)stream synchronization.
  //         Note that this overhead is not negligible
  //         --> disable only if absolutely necessary
  bool                                        hasReentrantSynchronousSubDownstream; // head module(s)
  // *NOTE*: if this is an 'outbound' (i.e. data travels in two directions;
  //         e.g. network connection-) stream, any 'inbound' (i.e. writer-
  //         side) data [!] may (!) 'turn around' and travel back upstream for
  //         dispatch --> account for it only once
  bool                                        inbound;                              // statistic[/network IO] module(s)
  ACE_SYNCH_RECURSIVE_MUTEX*                  lock;                                 // display/message handler module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type             mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  Stream_IAllocator*                          messageAllocator;
  Stream_IOutboundDataNotify*                 outboundNotificationHandle;           // IO module(s)
  struct Common_FlexBisonParserConfiguration* parserConfiguration;                  // parser module(s)
  //bool                                        passData;                             // renderer module(s)
  bool                                        passive;                              // network/device/... module(s)
  bool                                        printFinalReport;                     // statistic module(s)
  unsigned int                                reportingInterval;                    // (statistic) reporting interval (second(s)) [0: off]
  bool                                        slurpFiles;                           // file source module(s)
  ACE_HANDLE                                  socketHandle;                         // network module(s)
  bool                                        splitOnStep;                          // file sink module(s)
  ACE_Time_Value                              statisticCollectionInterval;          // source/statistic/... module(s)
  ACE_Time_Value                              statisticReportingInterval;           // [ACE_Time_Value::zero: off]
  Common_ITimerCB_t*                          timerManager;
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

struct Common_AllocatorConfiguration;
struct Common_EventDispatchConfiguration;
struct Stream_Configuration
{
  Stream_Configuration ()
   : allocatorConfiguration (NULL)
   , branches ()
   , cloneModule (false) // *NOTE*: cloneModule ==> delete module
   , dispatchConfiguration (NULL)
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

  struct Common_AllocatorConfiguration*     allocatorConfiguration;
  Stream_Branches_t                         branches; // distributor(s) *TODO*
  bool                                      cloneModule; // final-
  struct Common_EventDispatchConfiguration* dispatchConfiguration;
  bool                                      finishOnDisconnect; // (network) i/o streams
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type           mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  Stream_IAllocator*                        messageAllocator;
  Stream_Module_t*                          module; // final-
  std::string                               moduleBranch; // final- {"": main branch}
  ACE_Notification_Strategy*                notificationStrategy;
  bool                                      printFinalReport;
  bool                                      resetSessionData;
  // *IMPORTANT NOTE*: in a multi-threaded environment, threads MAY be
  //                   dispatching the reactor notification queue concurrently
  //                   (most notably, ACE_TP_Reactor)
  //                   --> enforce proper serialization
  bool                                      serializeOutput;
  Stream_SessionId_t                        sessionId;
  bool                                      setupPipeline;

  struct Stream_UserData*                   userData;
};

template <//const char* StreamName,
          ////////////////////////////////
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType>
class Stream_Configuration_T
 : public std::map<std::string,                                  // key:   module name
                   std::pair<struct Stream_ModuleConfiguration*,
                             ModuleHandlerConfigurationType*> >  // value: (pair of) module/handler configuration
 , public Common_IDumpState
{
  typedef std::map<std::string,
                   std::pair<struct Stream_ModuleConfiguration*,
                             ModuleHandlerConfigurationType*> > inherited;

 public:
  // convenient types
  typedef std::map<std::string,
                   std::pair<struct Stream_ModuleConfiguration*,
                             ModuleHandlerConfigurationType*> > MAP_T;
  typedef typename MAP_T::iterator ITERATOR_T;
  typedef typename MAP_T::const_iterator CONST_ITERATOR_T;

  Stream_Configuration_T ();
  inline virtual ~Stream_Configuration_T () {}

  bool initialize (const struct Stream_ModuleConfiguration&, // 'default' module configuration
                   const ModuleHandlerConfigurationType&,    // 'default' module handler configuration
                   const ConfigurationType&);

  virtual void dump_state () const;

  ConfigurationType* configuration_;
  bool               isInitialized_;
};

// include template definition
#include "stream_configuration.inl"

//////////////////////////////////////////

//extern const char empty_string_[];
typedef Stream_Configuration_T<//empty_string_,
                               struct Stream_Configuration,
                               struct Stream_ModuleHandlerConfiguration> Stream_Configuration_t;

#endif
