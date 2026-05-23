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

#ifndef TEST_U_GSTREAMER_PIPEWIRE_SOURCE_H
#define TEST_U_GSTREAMER_PIPEWIRE_SOURCE_H

#include "gst/gst.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "common_ui_windowtype_converter.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_streammodule_base.h"

#include "stream_dev_common.h"

#include "stream_lib_mediatype_converter.h"

gboolean test_u_gstreamer_pipewire_source_bus_cb (GstBus*,
                                                  GstMessage*,
                                                  gpointer);
GstFlowReturn test_u_gstreamer_pipewire_source_new_sample_cb (GstElement*,
                                                              gpointer);

struct Test_U_GStreamer_Pipewire_Source_CBData
{
  Test_U_GStreamer_Pipewire_Source_CBData ()
   : allocator (NULL)
   , loop (NULL)
   , nodeId (0)
   , queue (NULL)
  {}

  Stream_IAllocator*      allocator;
  GMainLoop*              loop;
  ACE_UINT32              nodeId; // pipewire session-
  ACE_Message_Queue_Base* queue;
};

//////////////////////////////////////////

extern const char test_u_gstreamer_pipewire_source_module_name_string[];

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType, // implements Common_ITimer
          typename UserDataType,
          ////////////////////////////////
          typename MediaType>
class Test_U_GStreamer_Pipewire_Source_T
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public Common_UI_WindowTypeConverter_T<HWND>
#else
#if defined (X11_SUPPORT)
 , public Common_UI_WindowTypeConverter_T<Window>
#endif // X11_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
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
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef Common_UI_WindowTypeConverter_T<HWND> inherited3;
#else
#if defined (X11_SUPPORT)
  typedef Common_UI_WindowTypeConverter_T<Window> inherited3;
#endif // X11_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Test_U_GStreamer_Pipewire_Source_T (ISTREAM_T*); // stream handle
  virtual ~Test_U_GStreamer_Pipewire_Source_T ();

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
                                    StatisticContainerType,
                                    SessionManagerType,
                                    TimerManagerType,
                                    UserDataType>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_GStreamer_Pipewire_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_GStreamer_Pipewire_Source_T (const Test_U_GStreamer_Pipewire_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_GStreamer_Pipewire_Source_T& operator= (const Test_U_GStreamer_Pipewire_Source_T&))

  // helper methods
  bool initialize_GStreamer (ACE_UINT32,                     // pipewire (session) node id
                             const struct Common_UI_Window&, // (target) window handle [NULL: NullSink]
                             const MediaType&);              // (source) media type

  virtual int svc (void);

  guint                                          busWatchId_;
  struct Test_U_GStreamer_Pipewire_Source_CBData CBData_;
  bool                                           isFirst_;
  bool                                           notifyAbort_;
  GstPipeline*                                   pipeline_;
  Stream_SessionId_t                             sessionId_;
};

// include template definition
#include "test_u_gstreamer_pipewire_source.inl"

#endif
