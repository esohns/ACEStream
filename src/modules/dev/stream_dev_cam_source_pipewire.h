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

#ifndef STREAM_DEV_CAM_SOURCE_PIPEWIRE_H
#define STREAM_DEV_CAM_SOURCE_PIPEWIRE_H

#include "spa/param/video/format-utils.h"

#include "pipewire/pipewire.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dev_common.h"

void acestream_dev_cam_pw_on_stream_param_changed_cb (void* , uint32_t, const struct spa_pod*);
void acestream_dev_cam_pw_on_process_cb (void*);

extern const char libacestream_default_dev_cam_source_pipewire_module_name_string[];

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
          typename StatisticContainerType,
          typename StatisticHandlerType>
class Stream_Dev_Cam_Source_Pipewire_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      typename SessionMessageType::DATA_T::DATA_T,
                                      typename SessionMessageType::DATA_T,
                                      StatisticContainerType,
                                      StatisticHandlerType,
                                      struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<struct spa_video_info>
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
                                      typename SessionMessageType::DATA_T::DATA_T,
                                      typename SessionMessageType::DATA_T,
                                      StatisticContainerType,
                                      StatisticHandlerType,
                                      struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<struct spa_video_info> inherited2;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Cam_Source_Pipewire_T (ISTREAM_T* = NULL); // stream handle
  virtual ~Stream_Dev_Cam_Source_Pipewire_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

  // info
  bool isInitialized () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_Pipewire_T (const Stream_Dev_Cam_Source_Pipewire_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_Pipewire_T& operator= (const Stream_Dev_Cam_Source_Pipewire_T&))

  // override some task-based members
  virtual int svc (void);

  struct Stream_Device_Pipewire_Capture_CBData CBData_;
  struct pw_stream_events                      events_;
  bool                                         isPipewireMainLoopThread_;
  struct pw_main_loop*                         loop_;
  uint8_t                                      PODBuffer_[STREAM_DEV_PIPEWIRE_DEFAULT_POD_BUFFER_SIZE];
};

// include template definition
#include "stream_dev_cam_source_pipewire.inl"

#endif
