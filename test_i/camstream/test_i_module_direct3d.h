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

#ifndef TEST_I_MODULE_DIRECT3D_H
#define TEST_I_MODULE_DIRECT3D_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_vis_target_direct3d.h"

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
          typename SessionDataContainerType>
class Test_I_DirectShow_Module_Direct3D_T
 : public Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType>
{
 public:
  Test_I_DirectShow_Module_Direct3D_T ();
  virtual ~Test_I_DirectShow_Module_Direct3D_T ();

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Module_Direct3D_T (const Test_I_DirectShow_Module_Direct3D_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_DirectShow_Module_Direct3D_T& operator= (const Test_I_DirectShow_Module_Direct3D_T&))

  // helper types
  typedef Test_I_DirectShow_Module_Direct3D_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              SessionDataType,
                                              SessionDataContainerType> OWN_TYPE_T;
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
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType>
class Test_I_MediaFoundation_Module_Direct3D_T
 : public Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType>
{
 public:
  Test_I_MediaFoundation_Module_Direct3D_T ();
  virtual ~Test_I_MediaFoundation_Module_Direct3D_T ();

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Module_Direct3D_T (const Test_I_MediaFoundation_Module_Direct3D_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_MediaFoundation_Module_Direct3D_T& operator= (const Test_I_MediaFoundation_Module_Direct3D_T&))

  // helper types
  typedef Test_I_MediaFoundation_Module_Direct3D_T<ACE_SYNCH_USE,
                                                   TimePolicyType,
                                                   ConfigurationType,
                                                   ControlMessageType,
                                                   DataMessageType,
                                                   SessionMessageType,
                                                   SessionDataType,
                                                   SessionDataContainerType> OWN_TYPE_T;
};

// include template definition
#include "test_i_module_direct3d.inl"

#endif
