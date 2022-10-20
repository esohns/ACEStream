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

#ifndef STREAM_MODULE_VIS_TARGET_GDI_T_H
#define STREAM_MODULE_VIS_TARGET_GDI_T_H

#include "ace/Global_Macros.h"

#include "common_ui_ifullscreen.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_vis_gdi_module_name_string[];

LRESULT CALLBACK libacestream_window_proc_cb (HWND, UINT, WPARAM, LPARAM);

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType>
class Stream_Vis_Target_GDI_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_UI_IFullscreen
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Vis_Target_GDI_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_GDI_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_UI_IFullscreen
  virtual void toggle ();

 protected:
  HDC                       context_;
  struct tagBITMAPINFO      header_;
  bool                      notify_;
  Common_Image_Resolution_t resolution_;
  HWND                      window_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_GDI_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_GDI_T (const Stream_Vis_Target_GDI_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_GDI_T& operator= (const Stream_Vis_Target_GDI_T&))

  // helper types
  typedef Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  MediaType> OWN_TYPE_T;

  // override (part of) ACE_Task_Base
  virtual int svc ();

  HWND createWindow ();
};

// include template definition
#include "stream_vis_target_gdi.inl"

#endif
