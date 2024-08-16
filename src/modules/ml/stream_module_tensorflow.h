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

#ifndef STREAM_MODULE_TENSORFLOW_H
#define STREAM_MODULE_TENSORFLOW_H

#include "tensorflow/c/c_api.h"
#if defined (TENSORFLOW_CC_SUPPORT)
//#undef Status
//#undef Success
#include "tensorflow/core/public/session.h"
#endif // TENSORFLOW_CC_SUPPORT

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

extern const char libacestream_default_ml_tensorflow_module_name_string[];

inline void libacestream_ml_tensorflow_module_free_buffer (void* data_in, size_t length_in) { delete [] (uint8_t*)data_in; }

template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Tensorflow_T
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Stream_Module_Tensorflow_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Tensorflow_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  TF_Session* session_;
  TF_Status*  status_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Tensorflow_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Tensorflow_T (const Stream_Module_Tensorflow_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Tensorflow_T& operator= (const Stream_Module_Tensorflow_T&))
};

//////////////////////////////////////////

#if defined (TENSORFLOW_CC_SUPPORT)
template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Module_Tensorflow_2
 : public Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Stream_Module_Tensorflow_2 (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_Tensorflow_2 ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  tensorflow::Session* session_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Tensorflow_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Tensorflow_2 (const Stream_Module_Tensorflow_2&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Tensorflow_2& operator= (const Stream_Module_Tensorflow_2&))
};
#endif // TENSORFLOW_CC_SUPPORT

//////////////////////////////////////////

// include template definition
#include "stream_module_tensorflow.inl"

#endif
