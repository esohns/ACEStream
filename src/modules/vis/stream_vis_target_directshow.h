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

#ifndef STREAM_MODULE_VIS_TARGET_DIRECTSHOW_H
#define STREAM_MODULE_VIS_TARGET_DIRECTSHOW_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include <dshow.h>
#include <strmif.h>
#include <windef.h>

#include "common_time_common.h"

#include "stream_misc_directshow_asynch_source_filter.h"
#include "stream_misc_directshow_target.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          typename SessionDataType,
          ////////////////////////////////
          typename FilterConfigurationType, // DirectShow-
          typename PinConfigurationType>    // DirectShow Filter (Output) Pin-
class Stream_Vis_Target_DirectShow_T
 : public Stream_Misc_DirectShow_Target_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          struct _AMMediaType>
 , public Stream_Misc_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        FilterConfigurationType,
                                                        PinConfigurationType,
                                                        struct _AMMediaType>
{
 public:
  Stream_Vis_Target_DirectShow_T ();
  virtual ~Stream_Vis_Target_DirectShow_T ();

  virtual bool initialize (const ConfigurationType&);
  using Stream_Misc_DirectShow_Asynch_Source_Filter_T::initialize;

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  typedef Stream_Misc_DirectShow_Target_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          FilterConfigurationType,
                                          PinConfigurationType,
                                          struct _AMMediaType> inherited;
  typedef Stream_Misc_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        FilterConfigurationType,
                                                        PinConfigurationType,
                                                        struct _AMMediaType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_DirectShow_T (const Stream_Vis_Target_DirectShow_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_DirectShow_T& operator= (const Stream_Vis_Target_DirectShow_T&))

  // helper methods
  bool initialize_DirectShow (const HWND,            // (target) window handle
                              const struct tagRECT&, // (target) window area
                              IGraphBuilder*,        // graph handle
                              IVideoWindow*&);       // return value: window control handle

  IVideoWindow* IVideoWindow_;
};

// include template definition
#include "stream_vis_target_directshow.inl"

#endif
