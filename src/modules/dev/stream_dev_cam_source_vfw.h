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

#ifndef STREAM_DEV_CAM_SOURCE_VIDEOFORWINDOWS_H
#define STREAM_DEV_CAM_SOURCE_VIDEOFORWINDOWS_H

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#include "Vfw.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"

#include "common_time_common.h"

#include "common_ui_windowtype_converter.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dev_common.h"

extern const char libacestream_default_dev_cam_source_vfw_module_name_string[];

LRESULT CALLBACK acestream_vfw_error_cb (HWND, int, LPTSTR);
LRESULT CALLBACK acestream_vfw_status_cb (HWND, int, LPTSTR);
LRESULT CALLBACK acestream_vfw_control_cb (HWND, int);
LRESULT CALLBACK acestream_vfw_video_cb (HWND, LPVIDEOHDR);

struct acestream_vfw_cbdata
{
  Stream_IAllocator*      allocator;
  ACE_Message_Queue_Base* queue;
  Stream_SessionId_t      sessionId;
};

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          typename UserDataType,
          ////////////////////////////////
          typename MediaType>
class Stream_Dev_Cam_Source_VfW_T
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
                                      UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_UI_WindowTypeConverter_T<HWND>
 , public Common_IGet_2_T<HWND>
 , public Common_IGetR_2_T<struct tagCapDriverCaps>
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
                                      UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
  typedef Common_UI_WindowTypeConverter_T<HWND> inherited3;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Cam_Source_VfW_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Cam_Source_VfW_T ();

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
                                    UserDataType>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)

  // implement Common_IGet_T
  inline virtual const HWND get_2 () const { return window_; }
  inline virtual const struct tagCapDriverCaps& getR_2 () const { return capabilities_; }

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_VfW_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_VfW_T (const Stream_Dev_Cam_Source_VfW_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_VfW_T& operator= (const Stream_Dev_Cam_Source_VfW_T&))

  // helper methods
  bool initialize_VfW (const struct Stream_Device_Identifier&, // device identifier
                       HWND);                                  // (target) window handle [NULL: new window]

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  struct tagCapDriverCaps     capabilities_;
  struct acestream_vfw_cbdata CBData_;
  bool                        passive_;
  bool                        preview_; // 'preview' mode ? : 'overlay' mode
  HWND                        window_; // capture-
};

// include template definition
#include "stream_dev_cam_source_vfw.inl"

#endif
