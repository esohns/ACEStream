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

#ifndef TEST_U_CAMSAVE_STREAM_H
#define TEST_U_CAMSAVE_STREAM_H

#include <ace/Atomic_Op.h>
#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfidl.h>
#endif

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_common_modules.h"
#include "test_u_camsave_message.h"
#include "test_u_camsave_session_message.h"

// forward declarations
class Stream_IAllocator;

class Stream_CamSave_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CamSave_StreamState,
                        struct Stream_CamSave_StreamConfiguration,
                        struct Stream_CamSave_StatisticData,
                        struct Stream_ModuleConfiguration,
                        struct Stream_CamSave_ModuleHandlerConfiguration,
                        struct Stream_CamSave_SessionData,   // session data
                        Stream_CamSave_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Stream_CamSave_Message,
                        Stream_CamSave_SessionMessage>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public IMFAsyncCallback
#endif
{
 public:
  Stream_CamSave_Stream ();
  virtual ~Stream_CamSave_Stream ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // override (part of) Stream_IStreamControl_T
  virtual const Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?

  // implement IMFAsyncCallback
  virtual STDMETHODIMP QueryInterface (const IID&,
                                       void**);
  virtual STDMETHODIMP_ (ULONG) AddRef ();
  virtual STDMETHODIMP_ (ULONG) Release ();
  virtual STDMETHODIMP GetParameters (DWORD*,  // return value: flags
                                      DWORD*); // return value: queue handle
  virtual STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle
#endif
  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const struct Stream_CamSave_StreamConfiguration&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (struct Stream_CamSave_StatisticData&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Stream_CamSave_StreamState,
                        struct Stream_CamSave_StreamConfiguration,
                        struct Stream_CamSave_StatisticData,
                        struct Stream_ModuleConfiguration,
                        struct Stream_CamSave_ModuleHandlerConfiguration,
                        struct Stream_CamSave_SessionData,   // session data
                        Stream_CamSave_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Stream_CamSave_Message,
                        Stream_CamSave_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_Stream (const Stream_CamSave_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_Stream& operator= (const Stream_CamSave_Stream&))

  // modules
  Stream_CamSave_Source_Module          source_;
  Stream_CamSave_StatisticReport_Module statisticReport_;
  Stream_CamSave_Display_Module         display_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_CamSave_DisplayNull_Module     displayNull_;
#endif
  Stream_CamSave_V4L2_AVIEncoder_Module encoder_;
  Stream_CamSave_FileWriter_Module      fileWriter_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // media session
  IMFMediaSession*                      mediaSession_;
  ULONG                                 referenceCount_;
#endif

  static ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> currentSessionID;
};

#endif
