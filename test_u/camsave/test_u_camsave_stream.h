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

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mfidl.h"
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
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        //////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        //////////////////
                        int,
                        int,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        //////////////////
                        Stream_CamSave_StreamConfiguration,
                        //////////////////
                        Stream_CamSave_StatisticData,
                        //////////////////
                        Stream_ModuleConfiguration,
                        Stream_CamSave_ModuleHandlerConfiguration,
                        //////////////////
                        Stream_CamSave_SessionData,   // session data
                        Stream_CamSave_SessionData_t, // session data container (reference counted)
                        Stream_CamSave_SessionMessage,
                        Stream_CamSave_Message>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public IMFAsyncCallback
#endif
{
 public:
  Stream_CamSave_Stream ();
  virtual ~Stream_CamSave_Stream ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // override (part of) Stream_IStreamControl_T
  virtual Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?

  // implement IMFAsyncCallback
  STDMETHODIMP STDMETHODCALLTYPE QueryInterface (const IID&,
                                                 void**);
  virtual ULONG STDMETHODCALLTYPE AddRef ();
  virtual ULONG STDMETHODCALLTYPE Release ();
  STDMETHODIMP GetParameters (DWORD*,  // return value: flags
                              DWORD*); // return value: queue handle
  STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle
#endif
  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&); // return value: module list

  // implement Common_IInitialize_T
  virtual bool initialize (const Stream_CamSave_StreamConfiguration&, // configuration
                           bool = true,                               // setup pipeline ?
                           bool = true);                              // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Stream_CamSave_StatisticData&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                        //////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        //////////////////
                        int,
                        int,
                        Stream_StateMachine_ControlState,
                        Stream_State,
                        //////////////////
                        Stream_CamSave_StreamConfiguration,
                        //////////////////
                        Stream_CamSave_StatisticData,
                        //////////////////
                        Stream_ModuleConfiguration,
                        Stream_CamSave_ModuleHandlerConfiguration,
                        //////////////////
                        Stream_CamSave_SessionData,   // session data
                        Stream_CamSave_SessionData_t, // session data container (reference counted)
                        Stream_CamSave_SessionMessage,
                        Stream_CamSave_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_Stream (const Stream_CamSave_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CamSave_Stream& operator= (const Stream_CamSave_Stream&))

  // modules
  Stream_CamSave_Module_Source_Module           source_;
  Stream_CamSave_Module_RuntimeStatistic_Module runtimeStatistic_;
  Stream_CamSave_Module_Display_Module          display_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_CamSave_Module_DisplayNull_Module      displayNull_;
#endif
  Stream_CamSave_Module_AVIEncoder_Module       encoder_;
  Stream_CamSave_Module_FileWriter_Module       fileWriter_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // media session
  IMFMediaSession*                              mediaSession_;
  ULONG                                         referenceCount_;
#endif

  static ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> currentSessionID;
};

#endif
