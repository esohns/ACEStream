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

#ifndef STREAM_DEV_MIC_SOURCE_ALSA_H
#define STREAM_DEV_MIC_SOURCE_ALSA_H

#include <string>

#include "alsa/asoundlib.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#include "stream_dev_common.h"

extern const char libacestream_default_dev_mic_source_alsa_module_name_string[];

static void stream_dev_mic_source_alsa_async_callback (snd_async_handler_t*);

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
          typename StatisticHandlerType>
class Stream_Dev_Mic_Source_ALSA_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                                      StatisticHandlerType,
                                      struct Stream_UserData>
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                                      StatisticHandlerType,
                                      struct Stream_UserData> inherited;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Mic_Source_ALSA_T (ISTREAM_T* = NULL,                                                         // stream handle
                                bool = false,                                                              // auto-start ?
                                enum Stream_HeadModuleConcurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE); // concurrency mode
  virtual ~Stream_Dev_Mic_Source_ALSA_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using inherited::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  //virtual void start ();
  //virtual void stop (bool = true,  // wait for completion ?
  //                   bool = true); // locked access ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

  // info
  bool isInitialized () const;

//  // implement (part of) Stream_ITaskBase
//  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
//                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  typedef Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
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
                                       StatisticHandlerType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_ALSA_T (const Stream_Dev_Mic_Source_ALSA_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_ALSA_T& operator= (const Stream_Dev_Mic_Source_ALSA_T&))

  struct Stream_Module_Device_ALSA_Capture_AsynchCBData asynchCBData_;
  struct _snd_async_handler*                            asynchHandler_;
  struct _snd_output*                                   debugOutput_;
  struct _snd_pcm*                                      deviceHandle_;
  bool                                                  isPassive_;
};

// include template definition
#include "stream_dev_mic_source_alsa.inl"

#endif
