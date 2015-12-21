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

#include "ace/Log_Msg.h"

#include "aviriff.h"
//#include "Streams.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_module_dev_defines.h"
#include "stream_module_dev_tools.h"

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::Stream_Module_CamSource_DirectShow_T (bool isActive_in,
                                                                                          bool autoStart_in)
 : inherited (NULL,         // lock handle
              isActive_in,  // active ?
              autoStart_in, // auto-start ?
              true)         // *NOTE*: when working in 'passive' mode, enabling
                            //         this utilizes the calling thread. Note
                            //         that this potentially renders state
                            //         transitions during processing a tricky
                            //         affair, as the calling thread may be
                            //         holding the lock --> check carefully
 , COMInitialized_ (false)
 , isInitialized_ (false)
 , ROTID_ (0)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
 , isFirst_ (true)
 , ICaptureGraphBuilder2_ (NULL)
 , IMediaControl_ (NULL)
 , IMediaEventEx_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::Stream_Module_CamSource_DirectShow_T"));

}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::~Stream_Module_CamSource_DirectShow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::~Stream_Module_CamSource_DirectShow_T"));

  int result = -1;

  if (timerID_ != -1)
  {
    const void* act_p = NULL;
    result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                  &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  timerID_));
    else
      ACE_DEBUG ((LM_WARNING, // this should happen in END_SESSION
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  timerID_));
  } // end IF

  if (ROTID_)
  {
    IRunningObjectTable* ROT_p = NULL;
    result = GetRunningObjectTable (0, &ROT_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto continue_;
    } // end IF
    ACE_ASSERT (ROT_p);
    result = ROT_p->Revoke (ROTID_);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d) \"%s\", continuing\n"),
                  ROTID_,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    ROT_p->Release ();
  } // end IF

continue_:
  if (inherited::configuration_)
    if (inherited::configuration_->windowController)
    {
      inherited::configuration_->windowController->put_Owner (NULL);
      inherited::configuration_->windowController->Release ();
      inherited::configuration_->windowController = NULL;
    } // end IF
  if (IMediaEventEx_)
  {
    IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
    IMediaEventEx_->Release ();
  } // end IF
  if (IMediaControl_)
  {
    IMediaControl_->StopWhenReady ();
    IMediaControl_->Release ();
  } // end IF
  if (ICaptureGraphBuilder2_)
    ICaptureGraphBuilder2_->Release ();
  if (COMInitialized_)
    CoUninitialize (); // <-- this is a bug
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::initialize"));

  int result = -1;

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (first_run)
  {
    first_run = false;

    HRESULT result = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    COM_initialized = true;
  } // end IF

  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (timerID_ != -1)
    {
      const void* act_p = NULL;
      result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                    &act_p);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    timerID_));
    } // end IF
    timerID_ = -1;

    if (ROTID_)
    {
      IRunningObjectTable* ROT_p = NULL;
      result = GetRunningObjectTable (0, &ROT_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        ROTID_ = 0;

        goto continue_;
      } // end IF
      ACE_ASSERT (ROT_p);
      result = ROT_p->Revoke (ROTID_);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d) \"%s\", continuing\n"),
                    ROTID_,
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      ROT_p->Release ();
      ROTID_ = 0;
    } // end IF

continue_:
    ACE_ASSERT (inherited::configuration_);
    if (inherited::configuration_->windowController)
    {
      inherited::configuration_->windowController->put_Owner (NULL);
      inherited::configuration_->windowController->Release ();
      inherited::configuration_->windowController = NULL;
    } // end IF
    if (IMediaEventEx_)
    {
      IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
      IMediaEventEx_->Release ();
      IMediaEventEx_ = NULL;
    } // end IF
    if (IMediaControl_)
    {
      IMediaControl_->StopWhenReady ();
      IMediaControl_->Release ();
      IMediaControl_ = NULL;
    } // end IF
    if (ICaptureGraphBuilder2_)
    {
      ICaptureGraphBuilder2_->Release ();
      ICaptureGraphBuilder2_ = NULL;
    } // end IF
    isFirst_ = true;

    isInitialized_ = false;
  } // end IF

  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

//done:
  if (COM_initialized)
    CoUninitialize ();

  return isInitialized_;
}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_CamSource_DirectShow_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::handleDataMessage"));

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_CamSource_DirectShow_T<LockType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::handleSessionMessage"));

  int result = -1;
  IRunningObjectTable* ROT_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
      message_inout->get ();
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      if (inherited::configuration_->streamConfiguration->statisticReportingInterval)
      {
        // schedule regular statistics collection...
        ACE_Time_Value interval (STREAM_STATISTIC_COLLECTION, 0);
        ACE_ASSERT (timerID_ == -1);
        ACE_Event_Handler* handler_p = &statisticCollectionHandler_;
        timerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                        NULL,                       // argument
                                                                        COMMON_TIME_NOW + interval, // first wakeup time
                                                                        interval);                  // interval
        if (timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
          return;
        } // end IF
        //        ACE_DEBUG ((LM_DEBUG,
        //                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for interval %#T...\n"),
        //                    timerID_,
        //                    &interval));
      } // end IF

      // *NOTE*: must be called once per thread, this may not work in a
      //         thread-pool...
      HRESULT result_2 = S_FALSE;
      if (!COMInitialized_)
      {
        result_2 = CoInitializeEx (NULL, COINIT_MULTITHREADED);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
          break;
        } // end IF
        COMInitialized_ = true;
      } // end IF

      if (!initialize_DirectShow (inherited::configuration_->device,
                                  inherited::configuration_->window,
                                  inherited::configuration_->area,
                                  ICaptureGraphBuilder2_,
                                  inherited::configuration_->windowController))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize_DirectShow(), aborting\n")));
        break;
      } // end IF
      ACE_ASSERT (ICaptureGraphBuilder2_);
      ACE_ASSERT (inherited::configuration_->windowController);

      // start previewing video data
      result_2 = IMediaControl_->Run ();
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMediaControl::Run() \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF

      // register graph in the ROT (GraphEdit.exe)
      IMoniker* moniker_p = NULL;
      WCHAR wsz[BUFSIZ];
      result_2 = GetRunningObjectTable (0, &ROT_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      ACE_ASSERT (ROT_p);
      IGraphBuilder* builder_p = NULL;
      result_2 = ICaptureGraphBuilder2_->GetFiltergraph (&builder_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ICaptureGraphBuilder2::GetFiltergraph() \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // clean up
        ROT_p->Release ();

        break;
      } // end IF
      ACE_ASSERT (ROT_p);
      result_2 =
        StringCchPrintfW (wsz, NUMELMS (wsz),
                          L"FilterGraph %08x pid %08x\0", (DWORD_PTR)builder_p,
                          GetCurrentProcessId ());
      result_2 = CreateItemMoniker (L"!", wsz,
                                    &moniker_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CreateItemMoniker() \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // clean up
        ROT_p->Release ();
        builder_p->Release ();

        break;
      } // end IF

      // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference 
      // to the object.  Using this flag will cause the object to remain 
      // registered until it is explicitly revoked with the Revoke() method. 
      // Not using this flag means that if GraphEdit remotely connects 
      // to this graph and then GraphEdit exits, this object registration  
      // will be deleted, causing future attempts by GraphEdit to fail until 
      // this application is restarted or until the graph is registered again. 
      result_2 =
        ROT_p->Register (ROTFLAGS_REGISTRATIONKEEPSALIVE,
                         builder_p, moniker_p,
                         &ROTID_);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IRunningObjectTable::Register() \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // clean up
        ROT_p->Release ();
        builder_p->Release ();
        moniker_p->Release ();

        break;
      } // end IF
      ROT_p->Release ();
      builder_p->Release ();
      moniker_p->Release ();

      break;
    }
    case STREAM_SESSION_END:
    {
      if (timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                      &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      timerID_));
        timerID_ = -1;
      } // end IF

      // *NOTE*: must be called once per thread, this may not work in a
      //         thread-pool...
      HRESULT result_2 = S_FALSE;
      bool COM_initialized = false;
      if (!COM_initialized)
      {
        result_2 = CoInitializeEx (NULL, COINIT_MULTITHREADED);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
          break;
        } // end IF
        COM_initialized = true;
      } // end IF

      // deregister graph from the ROT (GraphEdit.exe) ?
      if (ROTID_)
      {
        result_2 = GetRunningObjectTable (0, &ROT_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        
          // clean up
          ROTID_ = 0;

          goto continue_;
        } // end IF
        ACE_ASSERT (ROT_p);
        result_2 = ROT_p->Revoke (ROTID_);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d) \"%s\", continuing\n"),
                      ROTID_,
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        ROT_p->Release ();
        ROTID_ = 0;
      } // end IF

continue_:
      // Relinquish ownership (IMPORTANT!) of the video window. 
      // Failing to call put_Owner can lead to assert failures within 
      // the video renderer, as it still assumes that it has a valid 
      // parent window
      if (inherited::configuration_->windowController)
      {
        //result_2 =
        //  inherited::configuration_->windowController->put_Owner (NULL);
        //if (FAILED (result_2))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        inherited::configuration_->windowController->Release ();
        inherited::configuration_->windowController = NULL;
      } // end IF

      if (IMediaEventEx_)
      {
        IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
        IMediaEventEx_->Release ();
        IMediaEventEx_ = NULL;
      } // end IF

      // stop previewing video data ?
      if (IMediaControl_)
      {
        result_2 = IMediaControl_->StopWhenReady ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaControl::StopWhenReady() \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        IMediaControl_->Release ();
        IMediaControl_ = NULL;
      } // end IF

      if (ICaptureGraphBuilder2_)
      {
        ICaptureGraphBuilder2_->Release ();
        ICaptureGraphBuilder2_ = NULL;
      } // end IF

      // *NOTE*: must be called once per thread, this may not work in a
      //         thread-pool...
      if (COM_initialized)
      {
        CoUninitialize ();
        COM_initialized = false;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timestamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Module_CamSource_DirectShow_T<LockType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ConfigurationType,
                                     StreamStateType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     StatisticContainerType>::BufferCB (double sampleTime_in,
                                                                        BYTE* buffer_in,
                                                                        long bufferLen_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::BufferCB"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Module_CamSource_DirectShow_T<LockType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ConfigurationType,
                                     StreamStateType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     StatisticContainerType>::SampleCB (double sampleTime_in,
                                                                        IMediaSample* IMediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::SampleCB"));

  ACE_UNUSED_ARG (sampleTime_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  int result = -1;
  ProtocolMessageType* message_p = NULL;
  HRESULT result_2 = E_FAIL;

  if (isFirst_)
  {
    // *TODO*: remove type inference
    message_p =
      allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);

    struct _rifflist RIFF_list;
    ACE_OS::memset (&RIFF_list, 0, sizeof (struct _rifflist));
    RIFF_list.fcc = FCC('RIFF');
    //RIFF_list.cb = 0; // total size - 8
    RIFF_list.fccListType = FCC('AVI ');
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list),
                              sizeof (struct _rifflist));

    struct _rifflist RIFF_list_hdrl;
    ACE_OS::memset (&RIFF_list_hdrl, 0, sizeof (struct _rifflist));
    RIFF_list_hdrl.fcc = FCC('LIST');
    //RIFF_list_hdrl.cb = 0; // header LIST (up until and including JUNK)
    RIFF_list_hdrl.fccListType = FCC('hdrl');
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list_hdrl),
                              sizeof (struct _rifflist));

    struct _AMMediaType* media_type_p = NULL;
    result_2 = IMediaSample_in->GetMediaType (&media_type_p);
    if (FAILED (result_2) || !media_type_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMediaSample::GetMediaType(): \"%m\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return result_2;
    } // end IF
    ACE_ASSERT (media_type_p);

    struct _avimainheader AVI_header_avih;
    ACE_OS::memset (&AVI_header_avih, 0, sizeof (struct _avimainheader));
    AVI_header_avih.fcc = ckidMAINAVIHEADER;
    AVI_header_avih.cb = sizeof (struct _avimainheader) - 8;
    //AVI_header_avih.dwMicroSecPerFrame = 0; // unreliable
    AVI_header_avih.dwMaxBytesPerSec = 0; // unreliable
    AVI_header_avih.dwPaddingGranularity = 2048; // AVI_HEADERSIZE
    AVI_header_avih.dwFlags = AVIF_WASCAPTUREFILE;
    AVI_header_avih.dwTotalFrames = 0; // unreliable
    //AVI_header_avih.dwInitialFrames = 0;
    AVI_header_avih.dwStreams = 1;
    AVI_header_avih.dwSuggestedBufferSize = 0; // unreliable
    AVI_header_avih.dwWidth = 0;
    AVI_header_avih.dwHeight = 0;
    //AVI_header_avih.dwReserved = {0, 0, 0, 0};
    result = message_p->copy (reinterpret_cast<char*> (&AVI_header_avih),
                              sizeof (struct _avimainheader));
    //DeleteMediaType (media_type_p);
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);

    struct _rifflist RIFF_list_strl;
    ACE_OS::memset (&RIFF_list_strl, 0, sizeof (struct _rifflist));
    RIFF_list_strl.fcc = FCC('LIST');
    RIFF_list_strl.cb = 0; // header LIST (up until and including JUNK)
    RIFF_list_strl.fccListType = ckidSTREAMLIST;
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list_strl),
                              sizeof (struct _rifflist));

    struct _avistreamheader AVI_header_strh;
    ACE_OS::memset (&AVI_header_strh, 0, sizeof (struct _avistreamheader));
    AVI_header_strh.fcc = ckidSTREAMHEADER;
    AVI_header_strh.cb = sizeof (struct _avistreamheader) - 8;
    AVI_header_strh.fccType = streamtypeVIDEO;
    //AVI_header_strh.fccHandler = 0;
    //AVI_header_strh.dwFlags = AVISF_DISABLED | AVISF_VIDEO_PALCHANGES;
    AVI_header_strh.wPriority = 0;
    AVI_header_strh.wLanguage = 0;
    AVI_header_strh.dwInitialFrames = 0;
    AVI_header_strh.dwScale = 0;
    AVI_header_strh.dwRate = 0;
    //AVI_header_strh.dwStart = 0;
    AVI_header_strh.dwLength = 0;
    AVI_header_strh.dwSuggestedBufferSize = 0;
    AVI_header_strh.dwQuality = -1; // default
    AVI_header_strh.dwSampleSize = 0;
    //AVI_header_strh.rcFrame = {0, 0, 0, 0};
    result = message_p->copy (reinterpret_cast<char*> (&AVI_header_strh),
                              sizeof (struct _avistreamheader));

    struct tagBITMAPINFOHEADER AVI_header_strf;
    ACE_OS::memset (&AVI_header_strf, 0, sizeof (struct tagBITMAPINFOHEADER));
    AVI_header_strf.biSize = 0;
    AVI_header_strf.biWidth = 0;
    AVI_header_strf.biHeight = 0;
    AVI_header_strf.biPlanes = 0;
    AVI_header_strf.biBitCount = 0;
    AVI_header_strf.biCompression = 0;
    AVI_header_strf.biSizeImage = 0;
    AVI_header_strf.biXPelsPerMeter = 0;
    AVI_header_strf.biYPelsPerMeter = 0;
    AVI_header_strf.biClrUsed = 0;
    AVI_header_strf.biClrImportant = 0;
    result = message_p->copy (reinterpret_cast<char*> (&AVI_header_strf),
                              sizeof (struct tagBITMAPINFOHEADER));

    //struct _avisuperindex AVI_header_indx;
    //ACE_OS::memset (&AVI_header_indx, 0, sizeof (struct _avisuperindex));
    //AVI_header_indx.fcc = 0;
    //AVI_header_indx.cb = 0;
    //AVI_header_indx.wLongsPerEntry = 0;
    //AVI_header_indx.bIndexSubType = 0;
    //AVI_header_indx.bIndexType = 0;
    //AVI_header_indx.nEntriesInUse = 0;
    //AVI_header_indx.dwChunkId = 0;
    //AVI_header_indx.dwReserved = 0;
    ////AVI_header_indx.aIndex = 0;
    //struct _avisuperindex_entry AVI_header_indx_entry_0;
    //ACE_OS::memset (&AVI_header_indx_entry_0, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_1;
    //ACE_OS::memset (&AVI_header_indx_entry_1, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_2;
    //ACE_OS::memset (&AVI_header_indx_entry_2, 0, sizeof (struct _avisuperindex_entry));
    //struct _avisuperindex_entry AVI_header_indx_entry_3;
    //ACE_OS::memset (&AVI_header_indx_entry_3, 0, sizeof (struct _avisuperindex_entry));
    //result =
    //  message_p->copy (reinterpret_cast<char*> (&AVI_header_indx),
    //                   sizeof (struct _avisuperindex));

    DWORD junk = FCC('JUNK');
    result = message_p->copy (reinterpret_cast<char*> (&junk),
                              sizeof (DWORD));
    unsigned int padding = 2048 - message_p->length ();
    ACE_OS::memset (message_p->wr_ptr (), 0, padding);
    message_p->wr_ptr (padding);

    struct _rifflist RIFF_list_movi;
    ACE_OS::memset (&RIFF_list_movi, 0, sizeof (struct _rifflist));
    RIFF_list_movi.fcc = FCC('LIST');
    //RIFF_list_movi.cb = 0;
    RIFF_list_movi.fccListType = FCC('movi');
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_list_movi),
                              sizeof (struct _rifflist));

    struct _riffchunk RIFF_chunk_db;
    ACE_OS::memset (&RIFF_chunk_db, 0, sizeof (struct _riffchunk));
    RIFF_chunk_db.fcc = FCC('00db');
    //RIFF_chunk_db.cb = 0;
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_chunk_db),
                              sizeof (struct _riffchunk));

    result = inherited::putq (message_p, NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                    inherited::name ()));

      // clean up
      message_p->release ();

      return E_FAIL;
    } // end IF

    isFirst_ = false;
  } // end IF
  else
  {
    // *TODO*: remove type inference
    message_p =
      allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                  inherited::configuration_->streamConfiguration->bufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);

    struct _riffchunk RIFF_chunk_db;
    ACE_OS::memset (&RIFF_chunk_db, 0, sizeof (struct _riffchunk));
    RIFF_chunk_db.fcc = FCC ('00db');
    //RIFF_chunk_db.cb = 0;
    result = message_p->copy (reinterpret_cast<char*> (&RIFF_chunk_db),
                              sizeof (struct _riffchunk));

    result = inherited::putq (message_p, NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                    inherited::name ()));

      // clean up
      message_p->release ();

      return E_FAIL;
    } // end IF
  } // end IF

  // *TODO*: remove type inference
  message_p =
    allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("allocateMessage(%d) failed: \"%m\", aborting\n"),
                inherited::configuration_->streamConfiguration->bufferSize));
    return E_FAIL;
  } // end IF
  ACE_ASSERT (message_p);
  typename ProtocolMessageType::DATA_T& data_r =
    const_cast<typename ProtocolMessageType::DATA_T&> (message_p->get ());
  ULONG ref_count = IMediaSample_in->AddRef ();
  data_r.sample = IMediaSample_in;
  data_r.sampleTime = sampleTime_in;

  BYTE* buffer_p = NULL;
  result_2 = IMediaSample_in->GetPointer (&buffer_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::GetPointer(): \"%m\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result_2;
  } // end IF
  ACE_ASSERT (buffer_p);
  message_p->base (reinterpret_cast<char*> (buffer_p),
                   IMediaSample_in->GetSize (),
                   ACE_Message_Block::DONT_DELETE);
  message_p->wr_ptr (IMediaSample_in->GetSize ());

  result = inherited::putq (message_p, NULL);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited::name ()));

    // clean up
    message_p->release ();

    return E_FAIL;
  } // end IF

  return S_OK;
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HRESULT
Stream_Module_CamSource_DirectShow_T<LockType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ConfigurationType,
                                     StreamStateType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     StatisticContainerType>::QueryInterface (const IID&,
                                                                              void **)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::QueryInterface"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ULONG
Stream_Module_CamSource_DirectShow_T<LockType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ConfigurationType,
                                     StreamStateType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     StatisticContainerType>::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::AddRef"));

  return 1;
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ULONG
Stream_Module_CamSource_DirectShow_T<LockType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ConfigurationType,
                                     StreamStateType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     StatisticContainerType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::Release"));

  return 0;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::upStream (Stream_Base_t* streamBase_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::upStream"));

  ACE_UNUSED_ARG (streamBase_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Base_t*
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::upStream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::upStream"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL;)
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_CamSource_DirectShow_T<LockType,
                                     SessionMessageType,
                                     ProtocolMessageType,
                                     ConfigurationType,
                                     StreamStateType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     StatisticContainerType>::initialize_DirectShow (const std::string& deviceName_in,
                                                                                     const HWND windowHandle_in,
                                                                                     const GdkRectangle& windowArea_in,
                                                                                     ICaptureGraphBuilder2*& ICaptureGraphBuilder2_inout,
                                                                                     IVideoWindow*& IVideoWindow_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::initialize_DirectShow"));

  // initialize return value(s)
  if (ICaptureGraphBuilder2_inout)
  {
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;
  } // end IF
  if (IVideoWindow_inout)
  {
    IVideoWindow_inout->Release ();
    IVideoWindow_inout = NULL;
  } // end IF

  HRESULT result;
  ICreateDevEnum* enumerator_p = NULL;
  result = CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                             CLSCTX_INPROC_SERVER, IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (enumerator_p);

  IEnumMoniker* enum_moniker_p = NULL;
  result =
    enumerator_p->CreateClassEnumerator (CLSID_VideoInputDeviceCategory,
                                         &enum_moniker_p,
                                         0);
  if (result != S_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(CLSID_VideoInputDeviceCategory): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    enumerator_p->Release ();

    result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    return false;
  } // end IF
  ACE_ASSERT (enum_moniker_p);
  enumerator_p->Release ();

  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p  = NULL;
  VARIANT variant;
  while (enum_moniker_p->Next (1, &moniker_p, NULL) == S_OK)
  {
    ACE_ASSERT (moniker_p);

    properties_p = NULL;
    result = moniker_p->BindToStorage (0, 0, IID_PPV_ARGS (&properties_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      enum_moniker_p->Release ();
      moniker_p->Release ();

      return false;
    } // end IF
    ACE_ASSERT (properties_p);

    VariantInit (&variant);
    result = properties_p->Read (L"FriendlyName", &variant, 0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyBag::Read(FriendlyName): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      enum_moniker_p->Release ();
      moniker_p->Release ();
      properties_p->Release ();

      return false;
    } // end IF
    properties_p->Release ();
    ACE_Wide_To_Ascii converter (variant.bstrVal);
    VariantClear (&variant);

    if (deviceName_in.empty () ||
        (ACE_OS::strcmp (deviceName_in.c_str (),
                         converter.char_rep ()) == 0))
      break;

    moniker_p->Release ();
    moniker_p = NULL;
  } // end WHILE
  enum_moniker_p->Release ();
  if (!moniker_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no capture device found, aborting\n")));
    return false;
  } // end IF

  result = CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL,
                             CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2,
                             (void**)&ICaptureGraphBuilder2_inout);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_CaptureGraphBuilder2): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    moniker_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (ICaptureGraphBuilder2_inout);

  IGraphBuilder* builder_p = NULL;
  result = CoCreateInstance (CLSID_FilterGraph, NULL,
                             CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
                             (void**)&builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    moniker_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (builder_p);

  // retrieve interfaces for media control and the video window 
  result = builder_p->QueryInterface (IID_IMediaControl,
                                      (void**)&IMediaControl_);
  if (FAILED (result))
    goto error;
  result = builder_p->QueryInterface (IID_IMediaEventEx,
                                      (void**)&IMediaEventEx_);
  if (FAILED (result))
    goto error;
  result = builder_p->QueryInterface (IID_IVideoWindow,
                                      (void**)&IVideoWindow_inout);
  if (FAILED (result))
    goto error;
  goto continue_;
error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  // clean up
  moniker_p->Release ();
  builder_p->Release ();
  ICaptureGraphBuilder2_inout->Release ();
  ICaptureGraphBuilder2_inout = NULL;

  return false;
continue_:
  ACE_ASSERT (IMediaControl_);
  ACE_ASSERT (IMediaEventEx_);
  ACE_ASSERT (IVideoWindow_inout);
  //// set the window handle used to process graph events
  //result =
  //  IMediaEventEx_->SetNotifyWindow ((OAHWND)windowHandle_in,
  //                                   MODULE_DEV_CAM_UI_WIN32_WM_GRAPHNOTIFY, 0);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  moniker_p->Release ();
  //  builder_p->Release ();
  //  ICaptureGraphBuilder2_inout->Release ();
  //  ICaptureGraphBuilder2_inout = NULL;

  //  return false;
  //} // end IF

  result = ICaptureGraphBuilder2_inout->SetFiltergraph (builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICaptureGraphBuilder2::SetFiltergraph(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    moniker_p->Release ();
    builder_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF

  IBaseFilter* filter_p, *filter_2, *filter_3, *filter_4;
  filter_p = NULL;
  result = moniker_p->BindToObject (0, 0, IID_IBaseFilter,
                                    (void**)&filter_p);
  moniker_p->Release ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMoniker::BindToObject(IID_IBaseFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (filter_p);
  result = builder_p->AddFilter (filter_p, L"Capture Filter");
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n")));

    // clean up
    filter_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF

  // grab
  filter_2 = NULL;
  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                             (void**)&filter_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (filter_2);
  result = builder_p->AddFilter (filter_2, L"Sample Grabber");
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ISampleGrabber* grabber_p = NULL;
  result = filter_2->QueryInterface (IID_ISampleGrabber,
                                     (void**)&grabber_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (grabber_p);
  grabber_p->SetBufferSamples (false);
  result = grabber_p->SetCallback (this, 0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    grabber_p->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  //AM_MEDIA_TYPE media_type;
  //result = grabber_p->SetMediaType (&media_type);
  grabber_p->Release ();

  // decompress
  filter_3 = NULL;
  result = CoCreateInstance (CLSID_AVIDec, NULL,
                             CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                             (void**)&filter_3);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_AVIDec): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (filter_3);
  result = builder_p->AddFilter (filter_2, L"AVI Decompressor");
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    filter_3->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF

  // render to a window (GtkDrawingArea)
  filter_4 = NULL;
  result = CoCreateInstance (CLSID_VideoRenderer, NULL,
                             CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                             (void**)&filter_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_VideoRenderer): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    filter_3->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  ACE_ASSERT (filter_4);
  result = builder_p->AddFilter (filter_4, L"Video Renderer");
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    filter_3->Release ();
    filter_4->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF

  result =
    ICaptureGraphBuilder2_inout->RenderStream (//&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                               NULL, &MEDIATYPE_Video,
                                               filter_p,
                                               NULL,
                                               filter_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICaptureGraphBuilder::RenderStream(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    filter_p->Release ();
    filter_2->Release ();
    filter_3->Release ();
    filter_4->Release ();
    ICaptureGraphBuilder2_inout->Release ();
    ICaptureGraphBuilder2_inout = NULL;

    return false;
  } // end IF
  filter_p->Release ();
  filter_2->Release ();
  filter_3->Release ();
  filter_4->Release ();

  result = IVideoWindow_inout->put_MessageDrain ((OAHWND)windowHandle_in);
  if (FAILED (result))
    goto error_2;
  //result = IVideoWindow_inout->put_Owner ((OAHWND)windowHandle_in);
  //if (FAILED (result))
  //  goto error_2;
  result = IVideoWindow_inout->put_WindowStyle (WS_CHILD | WS_CLIPCHILDREN);
  if (FAILED (result))
    goto error_2;
  result =
    IVideoWindow_inout->SetWindowPosition (windowArea_in.x,
                                           windowArea_in.y,
                                           windowArea_in.width,
                                           windowArea_in.height);
  if (FAILED (result))
    goto error_2;
  goto continue_2;
error_2:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to configure IVideoWindow (was: 0x%@): \"%s\", aborting\n"),
              windowHandle_in,
              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  // clean up
  moniker_p->Release ();
  builder_p->Release ();
  ICaptureGraphBuilder2_inout->Release ();
  ICaptureGraphBuilder2_inout = NULL;

  return false;
continue_2:

  return true;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::svc"));

  int result = -1;
  int result_2 = -1;
  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  ProtocolMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;

  // step1: start processing data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    result = inherited::getq (message_block_p,
                              NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));

        // signal the controller ?
        if (!finished)
        {
          finished = true;
          inherited::finished ();
        } // end IF

        break;
      } // end IF
    } // end IF

    ACE_ASSERT (message_block_p);
    ACE_Message_Block::ACE_Message_Type message_type =
      message_block_p->msg_type ();
    if (message_type == ACE_Message_Block::MB_STOP)
    {
      // *NOTE*: when close()d manually (i.e. user abort), 'finished' will not
      //         have been set at this stage

      // signal the controller ?
      if (!finished)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("session aborted...\n")));

        finished = true;
        inherited::finished (); // queue session end

        result = inherited::putq (message_block_p,
                                  NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::name ()));

          // clean up
          message_block_p->release ();
          message_block_p = NULL;

          break;
        } // end IF

        continue;
      } // end IF

      // clean up
      message_block_p->release ();
      message_block_p = NULL;

      result_2 = 0;

      break; // aborted
    } // end IF

    // process manually
    inherited::handleMessage (message_block_p,
                              stop_processing);
  } while (true);

  return result_2;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ProtocolMessageType*
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_out = NULL;

  if (inherited::configuration_->streamConfiguration->messageAllocator)
  {
    try
    {
      // *TODO*: remove type inference
      message_out =
          static_cast<ProtocolMessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_out = NULL;
    }
  } // end IF
  else
  {
    ACE_NEW_NORETURN (message_out,
                      ProtocolMessageType (requestedSize_in));
  } // end ELSE
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_CamSource_DirectShow_T<LockType,
                           SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_DirectShow_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // step1: update session state
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

//  // step2: create session data object container
//  SessionDataContainerType* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    SessionDataContainerType (inherited::sessionData_,
//                                              false));
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
//    return false;
//  } // end IF

  // step3: send the statistic data downstream
//  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       *inherited::sessionData_,
                                       inherited::configuration_->streamConfiguration->messageAllocator);
}
