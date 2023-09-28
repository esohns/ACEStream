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

#ifndef STREAM_MODULE_VIS_TARGET_DIRECT2D_T_H
#define STREAM_MODULE_VIS_TARGET_DIRECT2D_T_H

#include "basetsd.h"
#include "d2d1.h"
#include "guiddef.h"
#include "windef.h"

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "stream_vis_target_win32_base.h"

extern const char libacestream_default_vis_direct2d_module_name_string[];

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
class Stream_Vis_Target_Direct2D_T
 : public Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType>
{
  typedef Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType> inherited;

 public:
  typedef typename inherited::ISTREAM_T ISTREAM_T;
  Stream_Vis_Target_Direct2D_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_Direct2D_T ();

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
  // helper methods
  bool initialize_Direct2D (HWND,     // (target-) window handle
                            REFGUID); // (input-) format

  ID2D1Bitmap*           bitmap_;
  ID2D1Factory*          factory_;
  struct _GUID           format_;
  UINT32                 pitch_;
  ID2D1HwndRenderTarget* renderTarget_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct2D_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct2D_T (const Stream_Vis_Target_Direct2D_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct2D_T& operator= (const Stream_Vis_Target_Direct2D_T&))

  // override (part of) ACE_Task_Base
  virtual int svc ();
};

// include template definition
#include "stream_vis_target_direct2d.inl"

#endif
