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

#ifndef STREAM_MODULE_CAMSOURCE_V4L_H
#define STREAM_MODULE_CAMSOURCE_V4L_H

#include "linux/videodev2.h"
#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif /* __cplusplus */

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"
#include "stream_lib_tools.h"

#include "stream_dev_common.h"
#include "stream_dev_tools.h"

extern const char libacestream_default_dev_cam_source_v4l_module_name_string[];

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
          typename SessionManagerType,
          typename TimerManagerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_CamSource_V4L_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
 , public Stream_MediaFramework_MediaTypeConverter_T<struct Stream_MediaFramework_V4L_MediaType>
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
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<struct Stream_MediaFramework_V4L_MediaType> inherited2;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Module_CamSource_V4L_T (ISTREAM_T* = NULL); // stream handle
  virtual ~Stream_Module_CamSource_V4L_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using inherited::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
//  virtual void report () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_CamSource_V4L_T (const Stream_Module_CamSource_V4L_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_CamSource_V4L_T& operator= (const Stream_Module_CamSource_V4L_T&))

  virtual int svc (void);

  // helper methods
  bool putStatisticMessage (const StatisticContainerType&) const; // statistic

  int                       captureFileDescriptor_; // capture
  int                       overlayFileDescriptor_; // preview

  Stream_Device_BufferMap_t bufferMap_;
  bool                      isPassive_; // foreign device descriptor(s) ?
};

// include template definition
#include "stream_dev_cam_source_v4l.inl"

#endif
