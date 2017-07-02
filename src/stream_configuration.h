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

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_idumpstate.h"
//#include "common_iget.h"
#include "common_iinitialize.h"

#include "stream_common.h"
#include "stream_defines.h"

// forward declarations
class Stream_IAllocator;

struct Common_ParserConfiguration;
struct Stream_AllocatorConfiguration;
struct Stream_ModuleHandlerConfiguration
{
  inline Stream_ModuleHandlerConfiguration ()
   : allocatorConfiguration (NULL)
   , bufferSize (STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , concurrency (STREAM_HEADMODULECONCURRENCY_PASSIVE)
   , concurrent (true)
   , crunchMessages (STREAM_MODULE_DEFAULT_CRUNCH_MESSAGES)
   , demultiplex (false)
   , hasHeader (false)
   , messageAllocator (NULL)
   , parserConfiguration (NULL)
   , passive (true)
   , printFinalReport (false)
   , reportingInterval (0)
   , statisticCollectionInterval (ACE_Time_Value::zero)
   , statisticReportingInterval (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
   //, stateMachineLock (NULL)
   , stream (NULL)
   , subscribersLock (NULL)
  {};
  // *NOTE*: add a (NOP) virtual function here to allow dynamic_cast to derived
  //         classes
  inline virtual ~Stream_ModuleHandlerConfiguration () {};

  struct Stream_AllocatorConfiguration* allocatorConfiguration;
  unsigned int                          bufferSize;
  enum Stream_HeadModuleConcurrency     concurrency;                 // head module(s)
  // *WARNING*: when disabled, this 'locks down' the pipeline head module. It
  //            will then hold the 'stream lock' during message processing to
  //            support (down)stream synchronization. This really only makes
  //            sense in fully synchronous layouts, or 'concurrent' scenarios
  //            with non-reentrant modules
  //            --> disable only if you know what you are doing
  bool                                  concurrent;                  // head module(s)
  // *NOTE*: this option may be useful for (downstream) modules that only work
  //         on CONTIGUOUS buffers (i.e. cannot parse chained message blocks)
  bool                                  crunchMessages;
  bool                                  demultiplex;                 // message handler module
  bool                                  hasHeader;
  Stream_IAllocator*                    messageAllocator;
  struct Common_ParserConfiguration*    parserConfiguration;         // parser module(s)
  bool                                  passive;                     // network/device/... module(s)

  bool                                  printFinalReport;            // statistic module
  unsigned int                          reportingInterval;           // (statistic) reporting interval (second(s)) [0: off]
  ACE_Time_Value                        statisticCollectionInterval; // head module(s)
  ACE_Time_Value                        statisticReportingInterval;  // [ACE_Time_Value::zero: off]

  //ACE_SYNCH_MUTEX*                   stateMachineLock;            // head module(s)

  // *NOTE*: modules can use this to temporarily relinquish the stream lock
  //         while they wait on some condition, in order to avoid deadlocks
  //         --> to be used primarily in 'non-concurrent' (see above) scenarios
  Stream_IStream_t*                     stream;

  ACE_SYNCH_RECURSIVE_MUTEX*            subscribersLock;
};

struct Stream_Configuration;
struct Stream_ModuleConfiguration
{
  inline Stream_ModuleConfiguration ()
   : notify (NULL)
   , streamConfiguration (NULL)
  {};

  Stream_INotify_t*            notify;
  // *TODO*: remove this ASAP
  struct Stream_Configuration* streamConfiguration;
};

struct Stream_AllocatorConfiguration
{
  inline Stream_AllocatorConfiguration ()
   : defaultBufferSize (STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , paddingBytes (0)
  {};

  unsigned int defaultBufferSize;

  // *NOTE*: add x bytes to each malloc(), override as needed
  //         (e.g. flex requires additional 2 YY_END_OF_BUFFER_CHARs). Note that
  //         this affects the ACE_Data_Block capacity, not its allotted size
  unsigned int paddingBytes;
};

struct Stream_Configuration
{
  inline Stream_Configuration ()
   : cloneModule (false) // *NOTE*: cloneModule ==> deleteModule
   , deleteModule (false)
   , finishOnDisconnect (false)
   , messageAllocator (NULL)
   , module (NULL)
   , moduleConfiguration (NULL)
   , notificationStrategy (NULL)
   , printFinalReport (false)
   , resetSessionData (true)
   , serializeOutput (false)
   , sessionID (0)
   , setupPipeline (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (COMMON_DEFAULT_WIN32_MEDIA_FRAMEWORK == COMMON_WIN32_FRAMEWORK_MEDIAFOUNDATION)
#endif
   , userData (NULL)
  {};

  bool                               cloneModule; // final-
  bool                               deleteModule; // final-
  bool                               finishOnDisconnect;
  Stream_IAllocator*                 messageAllocator;
  Stream_Module_t*                   module; // final-
  struct Stream_ModuleConfiguration* moduleConfiguration;
  ACE_Notification_Strategy*         notificationStrategy;
  bool                               printFinalReport;
  bool                               resetSessionData;
  // *IMPORTANT NOTE*: in a multi-threaded environment, threads MAY be
  //                   dispatching the reactor notification queue concurrently
  //                   (most notably, ACE_TP_Reactor)
  //                   --> enforce proper serialization
  bool                               serializeOutput;
  Stream_SessionId_t                 sessionID;
  bool                               setupPipeline;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                               useMediaFoundation;
#endif

  struct Stream_UserData*            userData;
};

template <const char* StreamName,
          ////////////////////////////////
          typename AllocatorConfigurationType,
          typename ConfigurationType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Stream_Configuration_T
 : public std::map<std::string,                    // key:   module name
                   ModuleHandlerConfigurationType> // value: module configuration
 //, public Common_IGetR_T<StreamConfigurationType>
 //, public Common_IInitialize_T<ConfigurationType>
 , public Common_IInitialize_T<ModuleHandlerConfigurationType>
 , public Common_IDumpState
{
  typedef std::map<std::string,
                   ModuleHandlerConfigurationType> inherited;

 public:
  // convenient types
  typedef std::map<std::string,
                   ModuleHandlerConfigurationType> MAP_T;
  typedef typename std::map<std::string,
                            ModuleHandlerConfigurationType>::iterator ITERATOR_T;
  typedef typename std::map<std::string,
                            ModuleHandlerConfigurationType>::const_iterator CONST_ITERATOR_T;

  Stream_Configuration_T ();
  inline virtual ~Stream_Configuration_T () {};

  // implement Common_IGet_T/Common_IInitialize_T
  //inline virtual const StreamConfigurationType& get () { return configuration_; } const;
  bool initialize (const AllocatorConfigurationType&,
                   const ConfigurationType&);
  virtual bool initialize (const ModuleHandlerConfigurationType&); // default module handler configuration

  virtual void dump_state () const;

  AllocatorConfigurationType allocatorConfiguration_;
  ConfigurationType          configuration_;
  ModuleConfigurationType    moduleConfiguration_;
  std::string                name_;

 private:
  bool                       isInitialized_;
};

// include template definition
#include "stream_configuration.inl"

//////////////////////////////////////////

extern const char empty_string_[];
typedef Stream_Configuration_T<empty_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_Configuration,
                               struct Stream_ModuleConfiguration,
                               struct Stream_ModuleHandlerConfiguration> Stream_Configuration_t;

#endif
