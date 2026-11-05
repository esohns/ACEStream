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

#ifndef STREAM_MODULE_SCHEDULER_H
#define STREAM_MODULE_SCHEDULER_H

#include <atomic>
#include <chrono>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_task_base_asynch.h"
#include "stream_task_base_synch.h"

#include "stream_misc_common.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_misc_scheduler_module_name_string[];

//////////////////////////////////////////

class Stream_MasterClock
{
 public:
  enum ClockMode
  {
    CLOCK_MODE_STOPWATCH = 0,
    CLOCK_MODE_AUDIO_SAMPLES
  };

  void
  initialize (enum ClockMode mode_in,
              int sampleRate_in)
  {
    start_ = std::chrono::high_resolution_clock::now ();
    mode_ = mode_in;
    sampleRate_ = sampleRate_in;
    totalSamplesRendered_ = 0;
  }

  inline void advance (int numAudioSamples_in) { totalSamplesRendered_ += numAudioSamples_in; }

  double
  getTimeInSeconds () const
  {
    switch (mode_)
    {
      case CLOCK_MODE_STOPWATCH:
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        std::chrono::steady_clock::time_point now =
          std::chrono::high_resolution_clock::now ();
#elif defined (ACE_LINUX)
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> now =
          std::chrono::high_resolution_clock::now ();
#else
#error missing implementation, aborting
#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX
        std::chrono::duration<double> elapsed_time = now - start_;
        return elapsed_time.count  ();
      }
      case CLOCK_MODE_AUDIO_SAMPLES:
        return static_cast<double> (totalSamplesRendered_.load ()) / sampleRate_;
      default:
        ACE_ASSERT (false);
        break;
    } // end SWITCH

    return 0.0;
  }

 private:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::chrono::steady_clock::time_point start_;
#elif defined (ACE_LINUX)
  std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start_;
#else
#error missing implementation, aborting
#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX
  enum ClockMode                        mode_ = CLOCK_MODE_STOPWATCH;
  int                                   sampleRate_ = 48000;
  std::atomic<ACE_INT64>                totalSamplesRendered_ = 0;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ///////////////////////////////
          typename MediaType,
          ///////////////////////////////
          typename UserDataType>
class Stream_Module_VideoScheduler_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Module_VideoScheduler_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_VideoScheduler_T () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message handle
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_VideoScheduler_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_VideoScheduler_T (const Stream_Module_VideoScheduler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_VideoScheduler_T& operator= (const Stream_Module_VideoScheduler_T&))

  // enqueue MB_STOP --> stop worker thread(s)
  virtual void stop (bool = true,   // stop inbound ? : outbound
                     bool = true,   // wait for completion ?
                     bool = false); // high priority ? (i.e. do not wait for queued messages)

  // override (part of) ACE_Task_Base
  virtual int svc (void);
  int svc_2 (void); // outbound dispatch

  void dispatch (DataMessageType*); // next message

  bool                                  isFirstDispatchingThread_;
  typename inherited::MESSAGE_QUEUE_T   queue_; // inbound-
  typename inherited::MESSAGE_QUEUE_T   queue_2_; // outbound-
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HANDLE                                task_; // inbound dispatch task
  HANDLE                                task_2_; // outbound dispatch task
#endif // ACE_WIN32 || ACE_WIN64

  void updatePCR (ACE_UINT64); // PCR (27 MHz)
  double getElapsedTimeInSeconds (ACE_UINT64); // PTS/DTS (in 90kHz ticks)

  bool                                  isPCRInitialized_;
  ACE_UINT64                            initialPCR_; // (90 kHz)
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ///////////////////////////////
          typename MediaType,
          ///////////////////////////////
          typename UserDataType>
class Stream_Module_AudioScheduler_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Module_AudioScheduler_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Module_AudioScheduler_T () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_AudioScheduler_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_AudioScheduler_T (const Stream_Module_AudioScheduler_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_AudioScheduler_T& operator= (const Stream_Module_AudioScheduler_T&))

  //int frameSize_;
  bool isFirstDataMessage_;
  int  sampleRate_;
};

// include template definition
#include "stream_misc_scheduler.inl"

#endif
