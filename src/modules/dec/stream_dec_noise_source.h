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

#ifndef STREAM_DEC_NOISE_SOURCE_H
#define STREAM_DEC_NOISE_SOURCE_H

#include <random>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_handler.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_dec_noise_source_module_name_string[];

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename MediaType>
class Stream_Dec_Noise_Source_T
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_ITimerHandler
{
  typedef Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Dec_Noise_Source_T (ISTREAM_T*); // stream handle
#else
  Stream_Dec_Noise_Source_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Dec_Noise_Source_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    StatisticContainerType,
                                    TimerManagerType,
                                    struct Stream_UserData>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

//  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ITimerHandler
  inline virtual const long get () const { return handler_.get (); }
  virtual void handle (const void*); // asynchronous completion token handle

 private:
  // convenient types
  typedef Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    StatisticContainerType,
                                    TimerManagerType,
                                    MediaType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Dec_Noise_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dec_Noise_Source_T (const Stream_Dec_Noise_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dec_Noise_Source_T& operator= (const Stream_Dec_Noise_Source_T&))

  // waveform generator state
  double                                      phase_;

  // noise generator state
  typedef std::uniform_real_distribution<long double> REAL_DISTRIBUTION_T;
  REAL_DISTRIBUTION_T                         realDistribution_;
  typedef std::uniform_int_distribution<uint64_t> INTEGER_DISTRIBUTION_T;
  INTEGER_DISTRIBUTION_T                      integerDistribution_;
  typedef std::uniform_int_distribution<int64_t> SIGNED_INTEGER_DISTRIBUTION_T;
  SIGNED_INTEGER_DISTRIBUTION_T               signedIntegerDistribution_;

  unsigned int                                bufferSize_;
  unsigned int                                frameSize_;
  Common_Timer_Handler                        handler_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                         mediaType_;
  HANDLE                                      task_;
#else
  struct Stream_MediaFramework_ALSA_MediaType mediaType_;
#endif // ACE_WIN32 || ACE_WIN64
};

// include template definition
#include "stream_dec_noise_source.inl"

#endif
