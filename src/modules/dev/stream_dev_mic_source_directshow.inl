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

#include <evcode.h>
#include <strsafe.h>
#include <vfwmsgs.h>

#include "ace/Log_Msg.h"

#include "common_log_tools.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_directshow_tools.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::Stream_Dev_Mic_Source_DirectShow_T (ISTREAM_T* stream_in)
 : inherited (stream_in,                            // stream handle
              false,                                // auto-start ?
              STREAM_HEADMODULECONCURRENCY_PASSIVE, // concurrency
              true)                                 // generate session messages ?
 , isFirst_ (true)
 , lock_ ()
 //, eventHandle_ (ACE_INVALID_HANDLE)
 , IAMDroppedFrames_ (NULL)
 , ICaptureGraphBuilder2_ (NULL)
 , IGraphBuilder_ (NULL)
 , IMediaControl_ (NULL)
 , IMediaEventEx_ (NULL)
 //, manageCOM_ (false)
 , ROTID_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::Stream_Dev_Mic_Source_DirectShow_T"));

  // *NOTE*: there are two threads running svc(): the invoking thread ('this' is
  //         a 'passive' module by default) and a thread processing DirectShow
  //         filter graph events (spawned in handleSessionMessage()). Currently,
  //         this hybrid mode of operation inflects a specific configuration
  //         setup.
  //         Specifically, ACE_Task_Base::thr_count_ and
  //         Common_Task_Base_T::threadCount_ need to be coordinated to maintain
  //         a consistent control flow:
  //         - increase ACE_Task_Base::thr_count_ so the first thread leaving
  //           svc() does not shut down the message queue (see: svc() below)
  //         - increase the threadCount_ so start() will actually spawn a new
  //           thread
  inherited::threadCount_ = 2;

  //         Note that as this module is a 'live' source, it is stop()ped by the
  //         user.. The first thread leaving svc() is the calling thread, which
  //         joins the DirectShow event processing thread (see:
  //         handleSessionMessage() below).
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::~Stream_Dev_Mic_Source_DirectShow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::~Stream_Dev_Mic_Source_DirectShow_T"));

  HRESULT result = E_FAIL;

  if (unlikely (ROTID_))
    Stream_MediaFramework_DirectShow_Tools::removeFromROT (ROTID_);

//continue_:
  if (unlikely (IMediaEventEx_))
  {
    result = IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(NULL,0,0): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    IMediaEventEx_->Release ();
  } // end IF
  if (unlikely (IMediaControl_))
  {
    result = IMediaControl_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    IMediaControl_->Release ();
  } // end IF
  if (unlikely (IAMDroppedFrames_))
    IAMDroppedFrames_->Release ();

  if (unlikely (IGraphBuilder_))
    IGraphBuilder_->Release ();
  if (unlikely (ICaptureGraphBuilder2_))
    ICaptureGraphBuilder2_->Release ();

  //if (manageCOM_)
  //  CoUninitialize ();
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
bool
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::initialize (const ConfigurationType& configuration_in,
                                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::initialize"));

  HRESULT result = E_FAIL;

  //// initialize COM ?
  //if (isFirst_)
  //{
  //  isFirst_ = false;

  //  if (configuration_in.manageCOM)
  //  {
  //    result = CoInitializeEx (NULL,
  //                             (COINIT_MULTITHREADED    |
  //                              COINIT_DISABLE_OLE1DDE  |
  //                              COINIT_SPEED_OVER_MEMORY));
  //    if (FAILED (result)) // RPC_E_CHANGED_MODE : 0x80010106L
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
  //                  inherited::mod_->name (),
  //                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //    manageCOM_ = true;
  //  } // end IF
  //} // end IF

  if (unlikely (inherited::isInitialized_))
  {
    isFirst_ = false;

    //eventHandle_ = ACE_INVALID_HANDLE;

    if (ROTID_)
    {
      Stream_MediaFramework_DirectShow_Tools::removeFromROT (ROTID_);
      ROTID_ = 0;
    } // end IF

//continue_:
    if (IMediaEventEx_)
    {
      result = IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(NULL,0,0): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      IMediaEventEx_->Release (); IMediaEventEx_ = NULL;
    } // end IF
    if (IMediaControl_)
    {
      result = IMediaControl_->Stop ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      IMediaControl_->Release (); IMediaControl_ = NULL;
    } // end IF
    if (IAMDroppedFrames_)
    {
      IAMDroppedFrames_->Release (); IAMDroppedFrames_ = NULL;
    } // end IF

    if (IGraphBuilder_)
    {
      IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
    } // end IF
    if (ICaptureGraphBuilder2_)
    {
      ICaptureGraphBuilder2_->Release (); ICaptureGraphBuilder2_ = NULL;
    } // end IF

    //manageCOM_ = false;
  } // end IF
  //manageCOM_ = configuration_in.manageCOM;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
void
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
#if defined (_DEBUG)
      std::string log_file_name;
#endif // _DEBUG

      if (inherited::configuration_->statisticCollectionInterval !=
          ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_),                                          // event handler handle
                                            NULL,                                                                     // asynchronous completion token
                                            COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                            inherited::configuration_->statisticCollectionInterval);                  // interval
        if (unlikely (inherited::timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
//                    inherited::mod_->name (),
//                    inherited::timerId_,
//                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      //bool COM_initialized = false;
      bool is_running = false;
      bool is_active = false;
      HRESULT result_2 = E_FAIL;

      //if (manageCOM_)
      //{
      //  result_2 = CoInitializeEx (NULL,
      //                            (COINIT_MULTITHREADED    |
      //                             COINIT_DISABLE_OLE1DDE  |
      //                             COINIT_SPEED_OVER_MEMORY));
      //  if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
      //                inherited::mod_->name (),
      //                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      //  COM_initialized = true;
      //} // end IF

      ISampleGrabber* sample_grabber_p = NULL;
      ULONG reference_count = 0;

      if (inherited::configuration_->builder)
      {
        // sanity check(s)
        ACE_ASSERT (!IGraphBuilder_);
        ACE_ASSERT (!IMediaControl_);
        ACE_ASSERT (!IMediaEventEx_);
        ACE_ASSERT (!IAMDroppedFrames_);

        reference_count = inherited::configuration_->builder->AddRef ();
        IGraphBuilder_ = inherited::configuration_->builder;

        IBaseFilter* filter_p = NULL;
        result_2 =
          IGraphBuilder_->FindFilterByName (STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO,
                                            &filter_p);
        if (FAILED (result_2))
          goto error_3;
        ACE_ASSERT (filter_p);
        result_2 = filter_p->QueryInterface (IID_PPV_ARGS (&IAMDroppedFrames_));
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IAMDroppedFrames): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        filter_p->Release (); filter_p = NULL;

        result_2 =
          IGraphBuilder_->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB,
                                            &filter_p);
        if (FAILED (result_2))
          goto error_3;
        ACE_ASSERT (filter_p);
        result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                             (void**)&sample_grabber_p);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        filter_p->Release (); filter_p = NULL;

        goto continue_;

error_3:
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF

      ACE_ASSERT (!ICaptureGraphBuilder2_);
      ACE_ASSERT (!IAMDroppedFrames_);

      // *TODO*: remove type inferences
      if (unlikely (!initialize_DirectShow (inherited::configuration_->deviceIdentifier.identifier._string,
                                            inherited::configuration_->audioOutput,
                                            ICaptureGraphBuilder2_,
                                            IAMDroppedFrames_,
                                            sample_grabber_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize_DirectShow(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (ICaptureGraphBuilder2_);
      ACE_ASSERT (IAMDroppedFrames_);

      result_2 = ICaptureGraphBuilder2_->GetFiltergraph (&IGraphBuilder_);
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ICaptureGraphBuilder2::GetFiltergraph(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF

continue_:
      ACE_ASSERT (IGraphBuilder_);
      ACE_ASSERT (sample_grabber_p);

      result_2 = sample_grabber_p->SetBufferSamples (false);
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      result_2 = sample_grabber_p->SetCallback (this, 0);
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      sample_grabber_p->Release (); sample_grabber_p = NULL;

      // retrieve interfaces for media control and the video window
      result_2 =
        IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&IMediaControl_));
      if (unlikely (FAILED (result_2)))
        goto error_2;
      result_2 =
        IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&IMediaEventEx_));
      if (unlikely (FAILED (result_2)))
        goto error_2;

      //result = IMediaEventEx_->GetEventHandle ((OAEVENT*)&eventHandle_);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to IMediaEventEx::GetEventHandle(): \"%s\", aborting\n"),
      //              inherited::mod_->name (),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      //  goto error_2;
      //} // end IF

      goto continue_2;

error_2:
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;

continue_2:
      ACE_ASSERT (IMediaControl_);
      ACE_ASSERT (IMediaEventEx_);

      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: capture format: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (session_data_r.formats.back ()).c_str ())));

      log_file_name =
        Common_Log_Tools::getLogDirectory (ACE_TEXT_ALWAYS_CHAR (""),
                                           0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += STREAM_LIB_DIRECTSHOW_LOGFILE_NAME;
      Stream_MediaFramework_DirectShow_Tools::debug (IGraphBuilder_,
                                                     log_file_name);
#endif // _DEBUG

      // start audio data capture
      result_2 = IMediaControl_->Run ();
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaControl::Run(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      is_running = true;
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started DirectShow graph\n"),
                  inherited::mod_->name ()));
#endif // _DEBUG

      // register graph in the ROT (graphedt.exe)
      if (unlikely (!Stream_MediaFramework_DirectShow_Tools::addToROT (IGraphBuilder_,
                                                                       ROTID_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::addToROT(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // process DirectShow filter graph events
      ACE_thread_t thread_id;
      inherited::TASK_BASE_T::start (thread_id);
      ACE_UNUSED_ARG (thread_id);
      is_active = inherited::TASK_BASE_T::isRunning ();
      ACE_ASSERT (is_active);

      break;

error:
      if (is_active)
        inherited::TASK_BASE_T::stop ();
      if (is_running)
      {
        result_2 = IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      } // end IF

      if (IAMDroppedFrames_)
      {
        IAMDroppedFrames_->Release (); IAMDroppedFrames_ = NULL;
      } // end IF

      if (IGraphBuilder_)
      {
        IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
      } // end IF
      if (ICaptureGraphBuilder2_)
      {
        ICaptureGraphBuilder2_->Release (); ICaptureGraphBuilder2_ = NULL;
      } // end IF

      //if (session_data_r.format)
      //  Stream_Device_Tools::deleteMediaType (session_data_r.format);

      //if (manageCOM_ && COM_initialized)
      //  CoUninitialize ();

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message
      { ACE_GUARD (typename inherited::LOCK_T, aGuard, inherited::lock_);
        if (sessionEndProcessed_)
          break; // done
        sessionEndProcessed_ = true;
      } // end lock scope

      if (likely (inherited::timerId_ != -1))
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      // *TODO*: (without more overhead,) this can only assme that the calling
      //         thread is the same as the one that dispatched the session begin
      //         message, which is not a safe assumption
      //bool COM_initialized = false;
      HRESULT result_2 = E_FAIL;

      //if (manageCOM_)
      //{
      //  result_2 = CoInitializeEx (NULL,
      //                             (COINIT_MULTITHREADED    |
      //                              COINIT_DISABLE_OLE1DDE  |
      //                              COINIT_SPEED_OVER_MEMORY));
      //  if (FAILED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
      //                inherited::mod_->name (),
      //                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      //  COM_initialized = true;
      //} // end IF

      // deregister graph from the ROT (GraphEdit.exe) ?
      if (likely (ROTID_))
      {
        Stream_MediaFramework_DirectShow_Tools::removeFromROT (ROTID_);
        ROTID_ = 0;
      } // end IF

//continue_3:
      if (likely (IMediaEventEx_))
      {
        result_2 = IMediaEventEx_->SetNotifyWindow (NULL,
                                                    0,
                                                    NULL);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      } // end IF

      if (likely (IMediaControl_))
      {
        // stop capturing audio data
        // *NOTE*: this tries to block until the graph has stopped, so that no
        //         data messages arrive after the session end has been signalled
        //         see also: https://msdn.microsoft.com/en-us/library/ee493380.aspx
        //result_2 = IMediaControl_->Stop ();
        result_2 = IMediaControl_->Pause ();
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Pause(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        result_2 = IMediaControl_->StopWhenReady ();
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::StopWhenReady(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: stopped DirectShow graph\n"),
                    inherited::mod_->name ()));
#endif // _DEBUG
        IMediaControl_->Release (); IMediaControl_ = NULL;
      } // end IF

      // signal/wait for the event processing thread
      if (//(eventHandle_ == ACE_INVALID_HANDLE) ||
          !IGraphBuilder_)
        goto continue_4;

      IMediaEventSink* event_sink_p = NULL;
      result_2 = IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&event_sink_p));
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IMediaEventSink): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto continue_4;
      } // end IF

      result_2 = event_sink_p->Notify (EC_USERABORT,
                                       NULL,
                                       NULL);
      if (unlikely (FAILED (result_2)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaEventSink::Notify(EC_USERABORT): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      event_sink_p->Release (); event_sink_p = NULL;
      //if (!::SetEvent (eventHandle_))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ::SetEvent(%@): \"%s\", continuing\n"),
      //              inherited::mod_->name (),
      //              eventHandle_,
      //              ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));

      inherited::TASK_BASE_T::wait (false); // do not wait for the message queue

      if (likely (IMediaEventEx_))
      {
        IMediaEventEx_->Release (); IMediaEventEx_ = NULL;
      } // end IF

continue_4:
      if (likely (IAMDroppedFrames_))
      {
        IAMDroppedFrames_->Release (); IAMDroppedFrames_ = NULL;
      } // end IF

      if (likely (IGraphBuilder_))
      {
        IGraphBuilder_->Release (); IGraphBuilder_ = NULL;
      } // end IF
      if (likely (ICaptureGraphBuilder2_))
      {
        ICaptureGraphBuilder2_->Release (); ICaptureGraphBuilder2_ = NULL;
      } // end IF

      //if (manageCOM_ && COM_initialized)
      //  CoUninitialize ();

      // *NOTE*: there already is a MB_STOP message left in the queue
      //if (likely (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      //  inherited::TASK_BASE_T::stop (false, // wait for completion ?
      //                                true); // N/A

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
bool
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (IAMDroppedFrames_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  // step1: collect data
  HRESULT result = E_FAIL;
  long average_frame_size = 0;

  // *NOTE*: "...Some filters that expose this interface do not implement the
  //         GetDroppedInfo or GetAverageFrameSize method."
  result = IAMDroppedFrames_->GetAverageFrameSize (&average_frame_size);
  if (unlikely (FAILED (result)))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: failed to IAMDroppedFrames::GetAverageFrameSize(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  ACE_UNUSED_ARG (average_frame_size);

  long captured_frames = 0;
  result = IAMDroppedFrames_->GetNumNotDropped (&captured_frames);
  //result = IAMDroppedFrames_->GetNumNotDropped (reinterpret_cast<long*> (&(data_out.capturedFrames)));
  if (unlikely (FAILED (result)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMDroppedFrames::GetNumNotDropped(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  result =
    IAMDroppedFrames_->GetNumDropped (reinterpret_cast<long*> (&(data_out.droppedFrames)));
  if (unlikely (FAILED (result)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMDroppedFrames::GetNumDropped(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  //// step2: send the information downstream
  //if (!inherited::putStatisticMessage (data_out)) // data container
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n")));
  //              inherited::mod_->name ()));
  //  return false;
  //} // end IF

  return true;
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
HRESULT
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::NotifyRelease (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::NotifyRelease"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
HRESULT
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::BufferCB (double sampleTime_in,
                                                                BYTE* buffer_in,
                                                                long bufferLen_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::BufferCB"));

  ACE_UNUSED_ARG (sampleTime_in);
  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferLen_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}
template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
HRESULT
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::SampleCB (double sampleTime_in,
                                                                IMediaSample* IMediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::SampleCB"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  int result = -1;

  ULONG reference_count = IMediaSample_in->AddRef ();

  DataMessageType* message_p = NULL;
  //try {
  //  message_p = dynamic_cast<DataMessageType*> (IMediaSample_in);
  //} catch (...) {
  //  //ACE_DEBUG ((LM_ERROR,
  //  //            ACE_TEXT ("%s: failed to dynamic_cast<DataMessageType*>(0x%@), continuing\n"),
  //  //            inherited::mod_->name (),
  //  //            IMediaSample_in));
  //  message_p = NULL;
  //}
  if (unlikely (!message_p))
  {
    // *TODO*: remove type inference
    message_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      return E_FAIL;
    } // end IF
    ACE_ASSERT (message_p);
    typename DataMessageType::DATA_T& data_r =
      const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
    data_r.sample = IMediaSample_in;
    data_r.sampleTime = sampleTime_in;

    long size = IMediaSample_in->GetSize ();
    BYTE* buffer_p = NULL;
    HRESULT result_2 = IMediaSample_in->GetPointer (&buffer_p);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaSample::GetPointer(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      return result_2;
    } // end IF
    ACE_ASSERT (buffer_p);
    message_p->base (reinterpret_cast<char*> (buffer_p),
                     size,
                     ACE_Message_Block::DONT_DELETE);
    message_p->wr_ptr (size);
  } // end IF

  if (inherited::configuration_->sinus)
  { ACE_ASSERT (inherited::sessionData_);
    SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->getR ());
    ACE_ASSERT (!session_data_r.formats.empty ());
    const struct _AMMediaType& media_type_r =
      session_data_r.formats.front ();
    ACE_ASSERT (InlineIsEqualGUID (media_type_r.formattype, FORMAT_WaveFormatEx));
    struct tWAVEFORMATEX* waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_r.pbFormat);
    static double stream_dev_mic_source_directshow_sinus_phase = 0.0;
    Stream_Module_Decoder_Tools::sinus (inherited::configuration_->sinusFrequency,
                                        waveformatex_p->nSamplesPerSec,
                                        waveformatex_p->nBlockAlign,
                                        waveformatex_p->nChannels,
                                        reinterpret_cast<uint8_t*> (message_p->rd_ptr ()),
                                        (message_p->length () / waveformatex_p->nBlockAlign),
                                        stream_dev_mic_source_directshow_sinus_phase);
  } // end IF

  result = inherited::put_next (message_p, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    return E_FAIL;
  } // end IF

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
int
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::svc"));

  int result = -1;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, -1);
    if (!isFirst_)
      goto continue_;
    isFirst_ = false;
  } // end lock scope

  // *NOTE*: prevent the deactivation of the message queue when the event
  //         processing thread joins (see: Stream_HeadModuleTaskBase_T::close())
  inherited::thr_count_++;

  // *NOTE*: use the calling threads' context (start() blocks)
  result = inherited::svc ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::svc(): \"%m\"\n"),
                inherited::mod_->name ()));

  inherited::thr_count_--;

  return result;

continue_:
  HRESULT result_2 = E_FAIL;
  long event_code = -1;
  LONG_PTR parameter_1, parameter_2;
  bool done = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (IMediaEventEx_);

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: event processing worker thread (id: %t) starting\n"),
              inherited::mod_->name ()));
#endif // _DEBUG

  // process DirectShow events

  // *NOTE*: (this being a 'live' source,) EC_COMPLETE is never sent
  //         --> process EC_COMPLETE from any renderer(s)
  result_2 = IMediaEventEx_->CancelDefaultHandling (EC_COMPLETE);
  if (unlikely (FAILED (result_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaEventEx::CancelDefaultHandling(EC_COMPLETE): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return -1;
  } // end IF

  do
  {
    result_2 = IMediaEventEx_->GetEvent (&event_code,
                                         &parameter_1,
                                         &parameter_2,
                                         INFINITE);
    if (unlikely (FAILED (result_2)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaEventEx::GetEvent(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      break;
    } // end IF

    switch (event_code)
    {
      case EC_COMPLETE:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received EC_COMPLETE (final status: \"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (static_cast<HRESULT> (parameter_1)).c_str ())));
        result = 0;
        done = true;
        break;
      }
      case EC_USERABORT:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received EC_USERABORT, returning\n"),
                    inherited::mod_->name ()));
        result = 0;
        done = true;
        break;
      }
      case EC_ERRORABORT:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received EC_ERRORABORT (status was: \"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (static_cast<HRESULT> (parameter_1)).c_str ())));
        done = true;
        break;
      }
      case EC_CLOCK_CHANGED:
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received EC_CLOCK_CHANGED, continuing\n"),
                    inherited::mod_->name ()));
        break;
      case EC_PAUSED:
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received EC_PAUSED (status was: \"%s\"), continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (static_cast<HRESULT> (parameter_1)).c_str ())));
        break;
      case EC_TIME:
      case EC_REPAINT:
      case EC_STREAM_ERROR_STOPPED:
      case EC_STREAM_ERROR_STILLPLAYING:
      case EC_ERROR_STILLPLAYING:
      case EC_PALETTE_CHANGED:
      case EC_VIDEO_SIZE_CHANGED:
      case EC_QUALITY_CHANGE:
      case EC_SHUTTING_DOWN:
      case EC_OPENING_FILE:
      case EC_BUFFERING_DATA:
      case EC_FULLSCREEN_LOST:
      case EC_ACTIVATE:
      case EC_NEED_RESTART:
      case EC_WINDOW_DESTROYED:
      case EC_DISPLAY_CHANGED:
      case EC_STARVATION:
      case EC_OLE_EVENT:
      case EC_NOTIFY_WINDOW:
      case EC_STREAM_CONTROL_STOPPED:
      case EC_STREAM_CONTROL_STARTED:
      case EC_END_OF_SEGMENT:
      case EC_SEGMENT_STARTED:
      case EC_LENGTH_CHANGED:
      case EC_DEVICE_LOST:
      case EC_SAMPLE_NEEDED:
      case EC_PROCESSING_LATENCY:
      case EC_SAMPLE_LATENCY:
      case EC_SCRUB_TIME:
      case EC_STEP_COMPLETE:
      case EC_TIMECODE_AVAILABLE:
      case EC_EXTDEVICE_MODE_CHANGE:
      case EC_STATE_CHANGE:
      case EC_GRAPH_CHANGED:
      case EC_CLOCK_UNSET:
      case EC_VMR_RENDERDEVICE_SET:
      case EC_VMR_SURFACE_FLIPPED:
      case EC_VMR_RECONNECTION_FAILED:
      case EC_PREPROCESS_COMPLETE:
      case EC_CODECAPI_EVENT:
      //////////////////////////////////
      case EC_SKIP_FRAMES:
      case EC_PLEASE_REOPEN:
      case EC_STATUS:
      case EC_MARKER_HIT:
      case EC_LOADSTATUS:
      case EC_FILE_CLOSED:
      case EC_ERRORABORTEX:
      //case EC_NEW_PIN:
      //case EC_RENDER_FINISHED:
      case EC_EOS_SOON:
      case EC_CONTENTPROPERTY_CHANGED:
      case EC_BANDWIDTHCHANGE:
      case EC_VIDEOFRAMEREADY:
      default:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received invalid/unknown event (was: \"0x%x\")...\n"),
                    inherited::mod_->name (),
                    event_code));
        break;
      }
    } // end SWITCH

    result_2 = IMediaEventEx_->FreeEventParams (event_code,
                                                parameter_1,
                                                parameter_2);
    if (unlikely (FAILED (result_2)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaEventEx::FreeEventParams(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      break;
    } // end IF

    if (unlikely (done))
      break;
  } while (true);

  return result;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType>
bool
Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   ConfigurationType,
                                   StreamControlType,
                                   StreamNotificationType,
                                   StreamStateType,
                                   SessionDataType,
                                   SessionDataContainerType,
                                   StatisticContainerType,
                                   TimerManagerType>::initialize_DirectShow (const std::string& deviceIdentifier_in,
                                                                             int audioOutput_in,
                                                                             ICaptureGraphBuilder2*& ICaptureGraphBuilder2_out,
                                                                             IAMDroppedFrames*& IAMDroppedFrames_out,
                                                                             ISampleGrabber*& ISampleGrabber_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_DirectShow_T::initialize_DirectShow"));

  // sanity check(s)
  ACE_ASSERT (!IAMDroppedFrames_out);
  ACE_ASSERT (!ISampleGrabber_out);

  HRESULT result = E_FAIL;

  if (!ICaptureGraphBuilder2_out)
  {
    CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&ICaptureGraphBuilder2_out));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoCreateInstance(CLSID_CaptureGraphBuilder2): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
  } // end IF
  ACE_ASSERT (ICaptureGraphBuilder2_out);

  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  IGraphBuilder* graph_builder_p = NULL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;
  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (deviceIdentifier_in,
                                                        CLSID_AudioInputDeviceCategory,
                                                        graph_builder_p,
                                                        buffer_negotiation_p,
                                                        stream_config_p,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (deviceIdentifier_in.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graph_builder_p);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (stream_config_p);

  result = ICaptureGraphBuilder2_out->SetFiltergraph (graph_builder_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ICaptureGraphBuilder2::SetFiltergraph(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  //// *NOTE*: (re-)Connect()ion of the video renderer input pin fails
  ////         consistently, so reuse is not feasible
  ////         --> rebuild the whole graph from scratch each time
  //if (!Stream_Device_Tools::reset (IGraphBuilder_in))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_Device_Tools::reset(), aborting\n")));
  //              inherited::mod_->name ()));
  //  return false;
  //} // end IF

  // retrieve interfaces for media control and the video window
  result = graph_builder_p->QueryInterface (IID_PPV_ARGS (&IMediaControl_));
  if (FAILED (result))
    goto error_2;
  result = graph_builder_p->QueryInterface (IID_PPV_ARGS (&IMediaEventEx_));
  if (FAILED (result))
    goto error_2;

  goto continue_;

error_2:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
              inherited::mod_->name (),
              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  goto error;

continue_:
  ACE_ASSERT (IMediaControl_);
  ACE_ASSERT (IMediaEventEx_);

  // set the window handle used to process graph events
  //result =
  //  IMediaEventEx_->SetNotifyWindow ((OAHWND)windowHandle_in,
  //                                   STREAM_DEV_CAM_UI_WIN32_WM_GRAPHNOTIFY, 0);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(): \"%s\", aborting\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

  IBaseFilter* filter_p = NULL;
  result =
    graph_builder_p->FindFilterByName (STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO,
                                       &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result = filter_p->QueryInterface (IID_PPV_ARGS (&IAMDroppedFrames_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IAMDroppedFrames): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IAMDroppedFrames_out);

  //// convert PCM to WAV ?
  //struct _AMMediaType* media_type_p = NULL;
  //if (!Stream_Device_Tools::getCaptureFormat (graph_builder_p,
  //                                                   media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_Device_Tools::getCaptureFormat(), aborting\n"),
  //              inherited::mod_->name ()));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (media_type_p);
  //struct _GUID media_subtype = media_type_p->subtype;
  //Stream_Device_Tools::deleteMediaType (media_type_p);
  //struct _GUID converter_guid = CLSID_Colour;
  //LPCWSTR converter_name = STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_PCM;
  //bool needs_converter = false;
  //if (media_subtype == MEDIASUBTYPE_PCM)
  //{} // end IF
  //else if (media_subtype == MEDIASUBTYPE_WAV)
  //{} // end IF
  //else
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: invalid/unknown media subtype (was: \"%s\"), aborting\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (Stream_Device_Tools::mediaSubTypeToString (media_subtype).c_str ())));
  //  goto error;
  //} // end ELSE
  IBaseFilter* filter_2 = NULL;
  //if (!windowHandle_in) goto continue_2;
  //result =
  //  graph_builder_p->FindFilterByName (converter_name,
  //                                     &filter_2);
  //if (FAILED (result))
  //{
  //  if (result != VFW_E_NOT_FOUND)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
  //                inherited::mod_->name (),
  //                ACE_TEXT_WCHAR_TO_TCHAR (converter_name),
  //                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //    goto error;
  //  } // end IF

  //  result = CoCreateInstance (converter_guid, NULL,
  //                             CLSCTX_INPROC_SERVER,
  //                             IID_PPV_ARGS (&filter_2));
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to CoCreateInstance() converter %s: \"%s\", aborting\n"),
  //                inherited::mod_->name (),
  //                ACE_TEXT (Stream_Device_Tools::mediaSubTypeToString (media_subtype).c_str ()),
  //                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //    goto error;
  //  } // end IF
  //  ACE_ASSERT (filter_2);
  //  result =
  //    graph_builder_p->AddFilter (filter_2,
  //                                converter_name);
  //  if (FAILED (result))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
  //                inherited::mod_->name (),
  //                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //    goto error;
  //  } // end IF
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s: added \"%s\"\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT_WCHAR_TO_TCHAR (converter_name)));
  //} // end IF
  //ACE_ASSERT (filter_2);

  // grab
  IBaseFilter* filter_3 = NULL;
  result =
    graph_builder_p->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB,
                                       &filter_3);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_3));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoCreateInstance(CLSID_SampleGrabber): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_3);
    result = graph_builder_p->AddFilter (filter_3,
                                         STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: added \"%s\"\n"),
    //            inherited::mod_->name (),
    //            ACE_TEXT_WCHAR_TO_TCHAR (STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));
  } // end IF
  ACE_ASSERT (filter_3);
  result = filter_3->QueryInterface (IID_ISampleGrabber,
                                     (void**)&ISampleGrabber_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (ISampleGrabber_out);

  // render to a sound card ?
//continue_2:
  IBaseFilter* filter_4 = NULL;
  result =
    graph_builder_p->FindFilterByName ((audioOutput_in ? STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                                       : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL),
                                       &filter_4);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT_WCHAR_TO_TCHAR ((audioOutput_in ? STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                                           : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL)),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    result = CoCreateInstance ((audioOutput_in ? CLSID_AudioRender
                                               : CLSID_NullRenderer), NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_4));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  (audioOutput_in ? ACE_TEXT ("CLSID_AudioRender")
                                  : ACE_TEXT ("CLSID_NullRenderer")),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_4);
    result =
      graph_builder_p->AddFilter (filter_4,
                                  (audioOutput_in ? STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                                                  : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: added \"%s\"\n"),
    //            inherited::mod_->name (),
    //            ACE_TEXT_WCHAR_TO_TCHAR ((audioOutput_in ? STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
    //                                                     : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL))));
  } // end IF
  ACE_ASSERT (filter_4);

//  // render to a window (GtkDrawingArea) ?
//continue_2:
//  IBaseFilter* filter_4 = NULL;
//  result =
//    graph_builder_p->FindFilterByName ((windowHandle_in ? STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                        : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL),
//                                       &filter_4);
//  if (FAILED (result))
//  {
//    if (result != VFW_E_NOT_FOUND)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                            : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL)),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      goto error;
//    } // end IF
//
//    result = CoCreateInstance ((windowHandle_in ? CLSID_VideoRenderer
//                                                : CLSID_NullRenderer), NULL,
//                               CLSCTX_INPROC_SERVER,
//                               IID_PPV_ARGS (&filter_4));
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
//                  (windowHandle_in ? ACE_TEXT ("CLSID_VideoRenderer")
//                                   : ACE_TEXT ("CLSID_NullRenderer")),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      goto error;
//    } // end IF
//    ACE_ASSERT (filter_4);
//    result =
//      graph_builder_p->AddFilter (filter_4,
//                                  (windowHandle_in ? STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                   : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL));
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      goto error;
//    } // end IF
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("added \"%s\"\n"),
//                ACE_TEXT_WCHAR_TO_TCHAR ((windowHandle_in ? STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
//                                                          : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL))));
//  } // end IF
//  ACE_ASSERT (filter_4);
//
  struct _AllocatorProperties allocator_properties;
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  allocator_properties.cbAlign = 1;
  allocator_properties.cbBuffer = 0;
  allocator_properties.cbPrefix = 0;
  allocator_properties.cBuffers =
    STREAM_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
//  result = buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    goto error;
//  } // end IF

  //result =
  //  ICaptureGraphBuilder2_in->RenderStream (//&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
  //                                          &PIN_CATEGORY_CAPTURE, NULL,
  //                                          filter_p,
  //                                          filter_2,
  //                                          //NULL,
  //                                          filter_4);
  //if (FAILED (result)) // E_INVALIDARG = 0x80070057, 0x80040217 = VFW_E_CANNOT_CONNECT ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ICaptureGraphBuilder::RenderStream(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error_2;
  //} // end IF
  graph_entry.filterName = STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  graph_configuration.push_back (graph_entry);
  //filter_pipeline.push_back (converter_name);
  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB;
  graph_configuration.push_back (graph_entry);
  graph_entry.filterName =
    (audioOutput_in ? STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO
                    : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL);
  graph_configuration.push_back (graph_entry);
  if (!Stream_MediaFramework_DirectShow_Tools::connect (graph_builder_p,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

#if defined (_DEBUG)
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result = buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              inherited::mod_->name (),
              allocator_properties.cBuffers,
              allocator_properties.cbBuffer,
              allocator_properties.cbAlign,
              allocator_properties.cbPrefix));
#endif // _DEBUG

  // clean up
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (filter_4)
    filter_4->Release ();

  // *NOTE*: apparently, this is necessary
  //         (see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd373396(v=vs.85).aspx)
  graph_builder_p->Release (); graph_builder_p = NULL;
  stream_config_p->Release (); stream_config_p = NULL;

  return true;

error:
  if (graph_builder_p)
    graph_builder_p->Release ();
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  if (stream_config_p)
    stream_config_p->Release ();

  if (ISampleGrabber_out)
  {
    ISampleGrabber_out->Release (); ISampleGrabber_out = NULL;
  } // end IF
  if (IAMDroppedFrames_out)
  {
    IAMDroppedFrames_out->Release (); IAMDroppedFrames_out = NULL;
  } // end IF
  if (ICaptureGraphBuilder2_out)
  {
    ICaptureGraphBuilder2_out->Release (); ICaptureGraphBuilder2_out = NULL;
  } // end IF

  return false;
}
