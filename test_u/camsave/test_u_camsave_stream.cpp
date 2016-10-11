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
#include "stdafx.h"

#include "test_u_camsave_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

// initialize statics
ACE_Atomic_Op<ACE_Thread_Mutex,
              unsigned long> Stream_CamSave_Stream::currentSessionID = 0;

Stream_CamSave_Stream::Stream_CamSave_Stream ()
 : inherited (ACE_TEXT_ALWAYS_CHAR ("FileCopyStream"))
 , source_ (ACE_TEXT_ALWAYS_CHAR ("CamSource"),
            NULL,
            false)
 , statisticReport_ (ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                     NULL,
                     false)
 , display_ (ACE_TEXT_ALWAYS_CHAR ("Display"),
             NULL,
             false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , displayNull_ (ACE_TEXT_ALWAYS_CHAR ("DisplayNull"),
                 NULL,
                 false)
#endif
 , encoder_ (ACE_TEXT_ALWAYS_CHAR ("AVIEncoder"),
             NULL,
             false)
 , fileWriter_ (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                NULL,
                false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaSession_ (NULL)
 , referenceCount_ (1)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::Stream_CamSave_Stream"));

}

Stream_CamSave_Stream::~Stream_CamSave_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::~Stream_CamSave_Stream"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result) &&
        (result != MF_E_SHUTDOWN)) // already shut down...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
const Stream_Module_t*
Stream_CamSave_Stream::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::find"));

  if (ACE_OS::strcmp (name_in.c_str (),
                      ACE_TEXT_ALWAYS_CHAR ("DisplayNull")) == 0)
    return const_cast<Stream_CamSave_DisplayNull_Module*> (&displayNull_);

  return inherited::find (name_in);
}
void
Stream_CamSave_Stream::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::start"));

  // sanity check(s)
  ACE_ASSERT (mediaSession_);

  struct _GUID GUID_s = GUID_NULL;
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  HRESULT result = mediaSession_->Start (&GUID_s,      // time format
                                         &property_s); // start position
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::Start(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    PropVariantClear (&property_s);

    return;
  } // end IF
  PropVariantClear (&property_s);

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF

  inherited::start ();
}
void
Stream_CamSave_Stream::stop (bool waitForCompletion_in,
                             bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::stop"));

  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF

  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
}

HRESULT
Stream_CamSave_Stream::QueryInterface (const IID& IID_in,
                                       void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (Stream_CamSave_Stream, IMFAsyncCallback),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}
ULONG
Stream_CamSave_Stream::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
ULONG
Stream_CamSave_Stream::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  if (count == 0);
  //delete this;

  return count;
}
HRESULT
Stream_CamSave_Stream::GetParameters (DWORD* flags_out,
                                      DWORD* queue_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::GetParameters"));

  ACE_UNUSED_ARG (flags_out);
  ACE_UNUSED_ARG (queue_out);

  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  return E_NOTIMPL;
}
HRESULT
Stream_CamSave_Stream::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::Invoke"));

  HRESULT result = E_FAIL;
  IMFMediaEvent* media_event_p = NULL;
  MediaEventType event_type = MEUnknown;
  HRESULT status = E_FAIL;
  struct tagPROPVARIANT value;
  PropVariantInit (&value);

  // sanity check(s)
  ACE_ASSERT (result_in);
  ACE_ASSERT (mediaSession_);
  ACE_ASSERT (inherited::sessionData_);

  //Stream_CamSave_SessionData& session_data_r =
  //  const_cast<Stream_CamSave_SessionData&> (inherited::sessionData_->get ());

  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = media_event_p->GetType (&event_type);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetStatus (&status);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetValue (&value);
  ACE_ASSERT (SUCCEEDED (result));
  switch (event_type)
  {
  case MEEndOfPresentation:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MEEndOfPresentation...\n")));
    break;
  }
  case MEError:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("received MEError: \"%s\"\n"),
                ACE_TEXT (Common_Tools::error2String (status).c_str ())));
    break;
  }
  case MESessionClosed:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionClosed, shutting down...\n")));
    //IMFMediaSource* media_source_p = NULL;
    //if (!Stream_Module_Device_Tools::getMediaSource (mediaSession_,
    //                                                 media_source_p))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), continuing\n")));
    //  goto continue_;
    //} // end IF
    //ACE_ASSERT (media_source_p);
    //result = media_source_p->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSource::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    //media_source_p->Release ();
//continue_:
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    break;
  }
  case MESessionEnded:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionEnded, closing sesion...\n")));
    result = mediaSession_->Close ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    break;
  }
  case MESessionCapabilitiesChanged:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionCapabilitiesChanged...\n")));
    break;
  }
  case MESessionNotifyPresentationTime:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionNotifyPresentationTime...\n")));
    break;
  }
  case MESessionStarted:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionStarted...\n")));
    break;
  }
  case MESessionStopped:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionStopped, stopping...\n")));
    if (isRunning ())
      stop (false,
            true);
    break;
  }
  case MESessionTopologySet:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionTopologySet (status was: \"%s\")...\n"),
                ACE_TEXT (Common_Tools::error2String (status).c_str ())));
    break;
  }
  case MESessionTopologyStatus:
  {
    UINT32 attribute_value = 0;
    result = media_event_p->GetUINT32 (MF_EVENT_TOPOLOGY_STATUS,
                                       &attribute_value);
    ACE_ASSERT (SUCCEEDED (result));
    MF_TOPOSTATUS topology_status =
      static_cast<MF_TOPOSTATUS> (attribute_value);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("received MESessionTopologyStatus: \"%s\"...\n"),
                ACE_TEXT (Stream_Module_Device_Tools::topologyStatusToString (topology_status).c_str ())));
    break;
  }
  default:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("received unknown/invalid media session event (type was: %d), continuing\n"),
                event_type));
    break;
  }
  } // end SWITCH
  PropVariantClear (&value);
  media_event_p->Release ();

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  return S_OK;

error:
  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}
#endif

bool
Stream_CamSave_Stream::load (Stream_ModuleList_t& modules_out,
                             bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::load"));

  // initialize return value(s)
  //modules_out.clear ();
  delete_out = false;

  // *NOTE*: one problem is that any module that was NOT enqueued onto the
  //         stream (e.g. because initialize() failed) needs to be explicitly
  //         close()d
  modules_out.push_back (&fileWriter_);
  modules_out.push_back (&encoder_);
  //modules_out.push_back (&displayNull_);
  modules_out.push_back (&display_);
  modules_out.push_back (&statisticReport_);
  modules_out.push_back (&source_);

  return true;
}

bool
Stream_CamSave_Stream::initialize (const Stream_CamSave_StreamConfiguration& configuration_in,
                                   bool setupPipeline_in,
                                   bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in,
                              false,
                              resetSessionData_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::name ().c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);
  Stream_CamSave_SessionData& session_data_r =
    const_cast<Stream_CamSave_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.sessionID = ++Stream_CamSave_Stream::currentSessionID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  session_data_r.format = &configuration_in.moduleHandlerConfiguration->format;
  session_data_r.frameRate =
      &configuration_in.moduleHandlerConfiguration->frameRate;
  if (!Stream_Module_Device_Tools::getFormat (configuration_in.moduleHandlerConfiguration->fileDescriptor,
                                              *session_data_r.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getFormat(%d), aborting\n"),
                configuration_in.moduleHandlerConfiguration->fileDescriptor));
    return false;
  } // end IF
  if (!Stream_Module_Device_Tools::getFrameRate (configuration_in.moduleHandlerConfiguration->fileDescriptor,
                                                 *session_data_r.frameRate))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getFrameRate(%d), aborting\n"),
                configuration_in.moduleHandlerConfiguration->fileDescriptor));
    return false;
  } // end IF
#endif
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->targetFileName;

  //// ---------------------------------------------------------------------------
  //// sanity check(s)
  //ACE_ASSERT (configuration_in.moduleConfiguration);

  // ******************* Camera Source ************************
  Stream_CamSave_Source* source_impl_p =
    dynamic_cast<Stream_CamSave_Source*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Strean_CamSave_CamSource> failed, aborting\n")));
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  IMFTopology* topology_p = NULL;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFMediaType* media_type_p = NULL;
  HRESULT result = E_FAIL;

  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    goto error;
  } // end IF
  COM_initialized = true;

  if (configuration_in.moduleHandlerConfiguration->session)
  {
    ULONG reference_count =
      configuration_in.moduleHandlerConfiguration->session->AddRef ();
    mediaSession_ = configuration_in.moduleHandlerConfiguration->session;

    if (!Stream_Module_Device_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
      goto error;
    } // end IF

    // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
    //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
    //         --> (try to) wait for the next MESessionTopologySet event
    // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
    //         still fails with MF_E_INVALIDREQUEST)
    do
    {
      result = mediaSession_->GetFullTopology (flags,
                                               0,
                                               &topology_p);
    } while (result == MF_E_INVALIDREQUEST);
    if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (topology_p);

    if (configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId)
      goto continue_;
    if (!Stream_Module_Device_Tools::getSampleGrabberNodeId (topology_p,
                                                             configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId);

    goto continue_;
  } // end IF

  if (!Stream_Module_Device_Tools::loadVideoRendererTopology (configuration_in.moduleHandlerConfiguration->device,
                                                              configuration_in.moduleHandlerConfiguration->format,
                                                              source_impl_p,
                                                              NULL,
                                                              //configuration_in.moduleHandlerConfiguration->window,
                                                              configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
                                                              configuration_in.moduleHandlerConfiguration->rendererNodeId,
                                                              topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadVideoRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_in.moduleHandlerConfiguration->device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;
#if defined (_DEBUG)
  Stream_Module_Device_Tools::dump (topology_p);
#endif

continue_:
  if (!Stream_Module_Device_Tools::setCaptureFormat (topology_p,
                                                     configuration_in.moduleHandlerConfiguration->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("capture format: \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (configuration_in.moduleHandlerConfiguration->format).c_str ())));
#endif

  if (session_data_r.format)
    Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);
  ACE_ASSERT (!session_data_r.format);
  session_data_r.format =
    static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
  if (!session_data_r.format)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, continuing\n")));
    goto error;
  } // end IF
  ACE_OS::memset (session_data_r.format, 0, sizeof (struct _AMMediaType));
  ACE_ASSERT (!session_data_r.format->pbFormat);
  if (!Stream_Module_Device_Tools::getOutputFormat (topology_p,
                                                    configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
                                                    media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  result_2 = MFInitAMMediaTypeFromMFMediaType (media_type_p,
                                               GUID_NULL,
                                               session_data_r.format);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFInitAMMediaTypeFromMFMediaType(): \"%m\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return false;
  } // end IF
  media_type_p->Release ();
  media_type_p = NULL;
  ACE_ASSERT (session_data_r.format);

  //HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
    mediaSession_ = NULL;
  } // end IF

  ACE_ASSERT (!mediaSession_);
  if (!Stream_Module_Device_Tools::setTopology (topology_p,
                                                mediaSession_,
                                                true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setTopology(), aborting\n")));
    goto error;
  } // end IF
  topology_p->Release ();
  topology_p = NULL;
  ACE_ASSERT (mediaSession_);

  if (!configuration_in.moduleHandlerConfiguration->session)
  {
    ULONG reference_count = mediaSession_->AddRef ();
    configuration_in.moduleHandlerConfiguration->session = mediaSession_;
  } // end IF
#endif

//  source_.initialize (*configuration_in.moduleConfiguration);
//  if (!source_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
//                source_.name ()));
//    goto error;
//  } // end IF
  if (!source_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                source_.name ()));
    goto error;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  source_.arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release ();
    session_data_r.direct3DDevice = NULL;
  } // end IF
  if (session_data_r.format)
    Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);
  session_data_r.resetToken = 0;
  if (session_data_r.session)
  {
    session_data_r.session->Release ();
    session_data_r.session = NULL;
  } // end IF
  if (mediaSession_)
  {
    mediaSession_->Release ();
    mediaSession_ = NULL;
  } // end IF

  if (COM_initialized)
    CoUninitialize ();
#endif

  return false;
}

bool
Stream_CamSave_Stream::collect (Stream_CamSave_StatisticData& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_CamSave_Statistic_WriterTask_t* statistic_impl =
    dynamic_cast<Stream_CamSave_Statistic_WriterTask_t*> (statisticReport_.writer ());
  if (!statistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_CamSave_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  Stream_CamSave_SessionData& session_data_r =
    const_cast<Stream_CamSave_SessionData&> (inherited::sessionData_->get ());
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  session_data_r.currentStatistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_impl->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic_T::collect(), continuing\n")));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IStatistic_T::collect(), aborting\n")));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result_2;
}

void
Stream_CamSave_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::report"));

//   Net_Module_Statistic_ReaderTask_t* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<Net_Module_Statistic_ReaderTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Net_Module_Statistic_ReaderTask_t> failed, returning\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module...
//   return (runtimeStatistic_impl->report ());

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
