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

#include "ace/Synch.h"
#include "test_u_camsave_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

#include "stream_stat_defines.h"

#include "stream_vis_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif

Stream_CamSave_Stream::Stream_CamSave_Stream ()
 : inherited ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING))
#else
 , converter_ (this,
               ACE_TEXT_ALWAYS_CHAR (MODULE_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING))
 , decoder_ (this,
             ACE_TEXT_ALWAYS_CHAR (MODULE_DEC_DECODER_LIBAV_DECODER_DEFAULT_NAME_STRING))
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING))
#endif
 , statisticReport_ (this,
                     ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , display_ (this,
             ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING))
 , displayNull_ (this,
                 ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_RENDERER_NULL_MODULE_NAME))
#else
 , display_ (this,
             ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_GTK_CAIRO_DEFAULT_NAME_STRING))
#endif
 , encoder_ (this,
             ACE_TEXT_ALWAYS_CHAR (MODULE_DEC_ENCODER_AVI_DEFAULT_NAME_STRING))
 , fileWriter_ (this,
                ACE_TEXT_ALWAYS_CHAR (MODULE_FILE_SINK_DEFAULT_NAME_STRING))
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
                  ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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

  if (!ACE_OS::strcmp (name_in.c_str (),
                       ACE_TEXT_ALWAYS_CHAR (MODULE_VIS_RENDERER_NULL_MODULE_NAME)))
    return const_cast<Stream_CamSave_MediaFoundation_DisplayNull_Module*> (&displayNull_);

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
                ACE_TEXT ("%s: failed to IMFMediaSession::Start(): \"%s\", returning\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    // clean up
    PropVariantClear (&property_s);

    return;
  } // end IF
  PropVariantClear (&property_s);

  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
                  ACE_TEXT ("%s: failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
  //if (count == 0);
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
                ACE_TEXT ("%s: failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT ("%s: received MEEndOfPresentation\n"),
                ACE_TEXT (stream_name_string_)));
    break;
  }
  case MEError:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: received MEError: \"%s\"\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (status).c_str ())));
    break;
  }
  case MESessionClosed:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                ACE_TEXT (stream_name_string_)));
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
    //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    //media_source_p->Release ();
//continue_:
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    break;
  }
  case MESessionEnded:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionEnded, closing sesion\n"),
                ACE_TEXT (stream_name_string_)));
    result = mediaSession_->Close ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    break;
  }
  case MESessionCapabilitiesChanged:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionCapabilitiesChanged\n"),
                ACE_TEXT (stream_name_string_)));
    break;
  }
  case MESessionNotifyPresentationTime:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionNotifyPresentationTime\n"),
                ACE_TEXT (stream_name_string_)));
    break;
  }
  case MESessionStarted:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionStarted\n"),
                ACE_TEXT (stream_name_string_)));
    break;
  }
  case MESessionStopped:
  { // status MF_E_INVALIDREQUEST: 0xC00D36B2L
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionStopped, stopping\n"),
                ACE_TEXT (stream_name_string_)));
    if (isRunning ())
      stop (false,
            true);
    break;
  }
  case MESessionTopologySet:
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: received MESessionTopologySet (status was: \"%s\")\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (status).c_str ())));
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
                ACE_TEXT ("%s: received MESessionTopologyStatus: \"%s\"\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::topologyStatusToString (topology_status).c_str ())));
    break;
  }
  default:
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: received unknown/invalid media session event (type was: %d), continuing\n"),
                ACE_TEXT (stream_name_string_),
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
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
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
  delete_out = false;

  // *NOTE*: one problem is that any module that was NOT enqueued onto the
  //         stream (e.g. because initialize() failed) needs to be explicitly
  //         close()d
  modules_out.push_back (&fileWriter_);
  modules_out.push_back (&encoder_);
  //modules_out.push_back (&displayNull_);
  modules_out.push_back (&display_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  modules_out.push_back (&converter_);
  modules_out.push_back (&decoder_);
#endif
  modules_out.push_back (&statisticReport_);
  modules_out.push_back (&source_);

  return true;
}

bool
Stream_CamSave_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
  struct Stream_CamSave_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
  struct Stream_CamSave_ModuleHandlerConfiguration* configuration_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_CamSave_MediaFoundation_Source* source_impl_p = NULL;
#else
  Stream_CamSave_V4L_Source* source_impl_p = NULL;
#endif

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  session_data_p =
    &const_cast<struct Stream_CamSave_SessionData&> (inherited::sessionData_->getR ());
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());

  configuration_p =
      dynamic_cast<struct Stream_CamSave_ModuleHandlerConfiguration*> (&(*iterator).second.second);

  // sanity check(s)
  ACE_ASSERT (configuration_p);

  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  session_data_p->sourceFormat.height =
      configuration_p->inputFormat.fmt.pix.height;
  session_data_p->sourceFormat.width =
      configuration_p->inputFormat.fmt.pix.width;
  session_data_p->frameRate = configuration_p->frameRate;
  session_data_p->inputFormat = configuration_p->inputFormat;
//  if (!Stream_Module_Device_Tools::getFormat (configuration_in.moduleHandlerConfiguration->fileDescriptor,
//                                              session_data_r.v4l2Format))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Module_Device_Tools::getFormat(%d), aborting\n"),
//                configuration_in.moduleHandlerConfiguration->fileDescriptor));
//    return false;
//  } // end IF
//  if (!Stream_Module_Device_Tools::getFrameRate (configuration_in.moduleHandlerConfiguration->fileDescriptor,
//                                                 session_data_r.v4l2FrameRate))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Module_Device_Tools::getFrameRate(%d), aborting\n"),
//                configuration_in.moduleHandlerConfiguration->fileDescriptor));
//    return false;
//  } // end IF
  session_data_p->format = configuration_p->format;
#endif
  session_data_p->targetFileName = configuration_p->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Camera Source ************************
  source_impl_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      dynamic_cast<Stream_CamSave_MediaFoundation_Source*> (source_.writer ());
#else
      dynamic_cast<Stream_CamSave_V4L_Source*> (source_.writer ());
#endif
  if (!source_impl_p)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_CamSave_MediaFoundation_CamSource> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_CamSave_V4L_CamSource> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
#endif
    goto error;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  IMFTopology* topology_p = NULL;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFMediaType* media_type_p = NULL;

  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  COM_initialized = true;

  if (configuration_p->session)
  {
    ULONG reference_count = configuration_p->session->AddRef ();
    mediaSession_ = configuration_p->session;

    if (!Stream_Module_Device_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

    // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
    //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
    //         --> (try to) wait for the next MESessionTopologySet event
    // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
    //         still fails with MF_E_INVALIDREQUEST)
    do
    {
      result_2 = mediaSession_->GetFullTopology (flags,
                                                 0,
                                                 &topology_p);
    } while (result_2 == MF_E_INVALIDREQUEST);
    if (FAILED (result_2)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (topology_p);

    if (configuration_p->sampleGrabberNodeId)
      goto continue_;
    if (!Stream_Module_Device_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                             configuration_p->sampleGrabberNodeId))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    ACE_ASSERT (configuration_p->sampleGrabberNodeId);

    goto continue_;
  } // end IF

  if (!Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology (configuration_p->deviceName,
                                                                              configuration_p->inputFormat,
                                                                              source_impl_p,
                                                                              NULL,
                                                                              //configuration_p->window,
                                                                              configuration_p->sampleGrabberNodeId,
                                                                              configuration_p->rendererNodeId,
                                                                              topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (configuration_p->deviceName.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;
#if defined (_DEBUG)
  Stream_Module_Device_MediaFoundation_Tools::dump (topology_p);
#endif

continue_:
  if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                     configuration_p->inputFormat))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: capture format: \"%s\"\n"),
              ACE_TEXT (stream_name_string_),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (configuration_p->inputFormat).c_str ())));
#endif

  if (session_data_p->inputFormat)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_p->inputFormat);
  ACE_ASSERT (!session_data_p->inputFormat);
  session_data_p->inputFormat =
    static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
  if (!session_data_p->inputFormat)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, continuing\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_OS::memset (session_data_p->inputFormat, 0, sizeof (struct _AMMediaType));
  ACE_ASSERT (!session_data_p->inputFormat->pbFormat);
  if (!Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                    configuration_p->sampleGrabberNodeId,
                                                                    media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::getOutputFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  result_2 = MFInitAMMediaTypeFromMFMediaType (media_type_p,
                                               GUID_NULL,
                                               session_data_p->inputFormat);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFInitAMMediaTypeFromMFMediaType(): \"%m\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF
  media_type_p->Release ();
  media_type_p = NULL;
  ACE_ASSERT (session_data_p->inputFormat);

  //HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
    mediaSession_ = NULL;
  } // end IF

  ACE_ASSERT (!mediaSession_);
  if (!Stream_Module_Device_MediaFoundation_Tools::setTopology (topology_p,
                                                                mediaSession_,
                                                                true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  topology_p->Release ();
  topology_p = NULL;
  ACE_ASSERT (mediaSession_);

  if (!configuration_p->session)
  {
    ULONG reference_count = mediaSession_->AddRef ();
    configuration_p->session = mediaSession_;
  } // end IF
#endif

  source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  source_.arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (session_data_p->direct3DDevice)
  {
    session_data_p->direct3DDevice->Release ();
    session_data_p->direct3DDevice = NULL;
  } // end IF
  if (session_data_p->inputFormat)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_p->inputFormat);
  session_data_p->direct3DManagerResetToken = 0;
  if (session_data_p->session)
  {
    session_data_p->session->Release ();
    session_data_p->session = NULL;
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
Stream_CamSave_Stream::collect (struct Stream_CamSave_StatisticData& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;

  Stream_CamSave_Statistic_WriterTask_t* statistic_impl_p =
    dynamic_cast<Stream_CamSave_Statistic_WriterTask_t*> (statisticReport_.writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_CamSave_Statistic_WriterTask_t> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF

  // synch access
  struct Stream_CamSave_SessionData& session_data_r =
    const_cast<struct Stream_CamSave_SessionData&> (inherited::sessionData_->getR ());
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      return false;
    } // end IF
  } // end IF

  session_data_r.statistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (stream_name_string_)));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
  else
    session_data_r.statistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (stream_name_string_)));
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
//   // delegate to this module
//   return (runtimeStatistic_impl->report ());

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
