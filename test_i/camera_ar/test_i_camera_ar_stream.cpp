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

#include "test_i_camera_ar_stream.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_os_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_macros.h"

#include "stream_dec_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_tools.h"

#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_stat_defines.h"

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Stream_CameraAR_DirectShow_Stream::Stream_CameraAR_DirectShow_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING))
 //, statisticReport_ (this,
 //                    ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
#if defined (FFMPEG_SUPPORT)
 , convert_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING))
 , resize_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING))
#endif // FFMPEG_SUPPORT
 , flip_ (this,
          ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_RGB_HFLIP_DEFAULT_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_DirectShow_Stream::Stream_CameraAR_DirectShow_Stream"));

}

bool
Stream_CameraAR_DirectShow_Stream::load (Stream_ILayout* layout_in,
                                         bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_DirectShow_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  layout_in->append (&source_, NULL, 0);
  //modules_out.push_back (&statisticReport_);
#if defined (FFMPEG_SUPPORT)
  layout_in->append (&convert_, NULL, 0);
  layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
#endif // FFMPEG_SUPPORT
  layout_in->append (&flip_, NULL, 0);

  return true;
}

bool
Stream_CameraAR_DirectShow_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_DirectShow_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Stream_CameraAR_DirectShow_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator, iterator_2;
  Stream_CameraAR_DirectShow_Source* source_impl_p = NULL;
  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  ISampleGrabber* isample_grabber_p = NULL;
  std::string log_file_name;
  struct _AMMediaType media_type_s;
  Test_I_DirectShow_SessionManager_t* session_manager_p =
    Test_I_DirectShow_SessionManager_t::SINGLETON_T::instance ();

  iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));

  // sanity check(s)
  ACE_ASSERT (iterator != const_cast<inherited::CONFIGURATION_T&> (configuration_in).end ());
  ACE_ASSERT (session_manager_p);

  // ---------------------------------------------------------------------------
  // step1: set up directshow filter graph
  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED     |
                              COINIT_DISABLE_OLE1DDE   |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF
  COM_initialized = true;

  if ((*iterator).second.second->builder)
  {
    // *NOTE*: Stream_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    if (!Stream_MediaFramework_DirectShow_Tools::reset ((*iterator).second.second->builder))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::reset(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

    if (!Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation ((*iterator).second.second->builder,
                                                                       STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L,
                                                                       buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation(), continuing\n"),
                  ACE_TEXT (stream_name_string_)));
      //goto error;
    } // end IF
    //ACE_ASSERT (buffer_negotiation_p);

    goto continue_;
  } // end IF

  ACE_ASSERT ((*iterator).second.second->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph ((*iterator).second.second->deviceIdentifier,
                                                        CLSID_VideoInputDeviceCategory,
                                                        (*iterator).second.second->builder,
                                                        buffer_negotiation_p,
                                                        stream_config_p,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ((*iterator).second.second->deviceIdentifier.identifier._string)));
    goto error;
  } // end IF
  ACE_ASSERT ((*iterator).second.second->builder);
  //ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (stream_config_p);
  stream_config_p->Release (); stream_config_p = NULL;

continue_:
  if (!Stream_Device_DirectShow_Tools::setCaptureFormat ((*iterator).second.second->builder,
                                                         CLSID_VideoInputDeviceCategory,
                                                         configuration_in.configuration_->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_DirectShow_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  //// sanity check(s)
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration);

  //if (!Stream_Device_Tools::getDirect3DDevice (*(*iterator).second.second->direct3DConfiguration,
  //                                                    direct3D_manager_p,
  //                                                    reset_token))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_Device_Tools::getDirect3DDevice(), aborting\n"),
  //              ACE_TEXT (stream_name_string_)));
  //  goto error;
  //} // end IF
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration->handle);
  //ACE_ASSERT ((*iterator).second.second->direct3DConfiguration->handle);
  //ACE_ASSERT (direct3D_manager_p);
  //ACE_ASSERT (reset_token);
  //direct3D_manager_p->Release (); direct3D_manager_p = NULL;

  if (!Stream_Module_Decoder_Tools::loadVideoRendererGraph (CLSID_VideoInputDeviceCategory,
                                                            configuration_in.configuration_->format,
                                                            (*iterator).second.second->outputFormat,
                                                            NULL, // use NULL-VideoRenderer
                                                            //(*iterator).second.second->window,
                                                            (*iterator).second.second->builder,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadVideoRendererGraph(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  result_2 =
    (*iterator).second.second->builder->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                                         &filter_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result_2 = filter_p->QueryInterface (IID_ISampleGrabber,
                                       (void**)&isample_grabber_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (isample_grabber_p);
  filter_p->Release (); filter_p = NULL;

  result_2 = isample_grabber_p->SetBufferSamples (false);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result_2 = isample_grabber_p->SetCallback (source_impl_p, 0);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  isample_grabber_p->Release (); isample_grabber_p = NULL;

  if (buffer_negotiation_p)
  {
    ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
    // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
    //         if this is -1/0 (why ?)
    allocator_properties.cbAlign = 1;
    allocator_properties.cbBuffer =
      configuration_in.configuration_->allocatorConfiguration->defaultBufferSize;
    allocator_properties.cbPrefix = -1; // <-- use default
    allocator_properties.cBuffers =
      STREAM_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
    result_2 =
        buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
    if (FAILED (result_2)) // E_UNEXPECTED: 0x8000FFFF --> graph already connected
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      goto error;
    } // end IF
  } // end IF

  if (!Stream_MediaFramework_DirectShow_Tools::connect ((*iterator).second.second->builder,
                                                        graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  //         --> reconnect the AVI decompressor to the (connected) sample
  //             grabber; this seems to work
  if (!Stream_MediaFramework_DirectShow_Tools::connected ((*iterator).second.second->builder,
                                                          STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reconnecting...\n"),
                ACE_TEXT (stream_name_string_)));

    if (!Stream_MediaFramework_DirectShow_Tools::connectFirst ((*iterator).second.second->builder,
                                                               STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connectFirst(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::connected ((*iterator).second.second->builder,
                                                                 STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L));

  if (buffer_negotiation_p)
  {
#if defined (_DEBUG)
    ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
    result_2 =
      buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
    if (FAILED (result_2)) // E_FAIL (0x80004005)
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s/%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      //goto error;
    } // end IF
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: negotiated allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
                  ACE_TEXT (stream_name_string_),
                  allocator_properties.cBuffers,
                  allocator_properties.cbBuffer,
                  allocator_properties.cbAlign,
                  allocator_properties.cbPrefix));
#endif // _DEBUG
    buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;
  } // end IF

  result_2 =
    (*iterator).second.second->builder->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result_2 = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release (); media_filter_p = NULL;

  // ---------------------------------------------------------------------------
  // step3: allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<Stream_CameraAR_DirectShow_SessionData&> (session_manager_p->getR (inherited::id_));

  // ---------------------------------------------------------------------------
  // step4: initialize module(s)

  // ******************* Camera Source ************************
  source_impl_p =
    dynamic_cast<Stream_CameraAR_DirectShow_Source*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_CamSave_DirectShow_Source> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  // ---------------------------------------------------------------------------
  // step5: update session data
  ACE_ASSERT (session_data_p->formats.empty ());
  session_data_p->formats.push_back (configuration_in.configuration_->format);
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  if (!Stream_MediaFramework_DirectShow_Tools::getOutputFormat ((*iterator).second.second->builder,
                                                                STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L,
                                                                media_type_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getOutputFormat(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L)));
    goto error;
  } // end IF
  session_data_p->formats.push_back (media_type_s);
  //ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::matchMediaType (*session_data_p->sourceFormat, *(*iterator).second.second->sourceFormat));

  // ---------------------------------------------------------------------------
  // step6: initialize head module
  //source_impl_p->setP (&(inherited::state_));
  ////fileReader_impl_p->reset ();
  //// *NOTE*: push()ing the module will open() it
  ////         --> set the argument that is passed along (head module expects a
  ////             handle to the session data)
  //source_.arg (inherited::sessionData_);

  // step7: assemble stream
  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_r.fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_r.size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  if ((*iterator).second.second->builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF
  if (session_data_p)
  {
    if (session_data_p->direct3DDevice)
    {
      session_data_p->direct3DDevice->Release (); session_data_p->direct3DDevice = NULL;
    } // end IF
    Stream_MediaFramework_DirectShow_Tools::free (session_data_p->formats);
    session_data_p->resetToken = 0;
  } // end IF

  if (COM_initialized)
    CoUninitialize ();

  return false;
}

//////////////////////////////////////////

Stream_CameraAR_MediaFoundation_Stream::Stream_CameraAR_MediaFoundation_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING))
 //, statisticReport_ (this,
 //                    ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
#if defined (FFMPEG_SUPPORT)
 , convert_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING))
 , resize_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING))
#endif // FFMPEG_SUPPORT
 , flip_ (this,
          ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_RGB_HFLIP_DEFAULT_NAME_STRING))
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
 , mediaSession_ (NULL)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::Stream_CameraAR_MediaFoundation_Stream"));

}

Stream_CameraAR_MediaFoundation_Stream::~Stream_CameraAR_MediaFoundation_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::~Stream_CameraAR_MediaFoundation_Stream"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result) &&
        (result != MF_E_SHUTDOWN)) // already shut down...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

const Stream_Module_t*
Stream_CameraAR_MediaFoundation_Stream::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::find"));

  //if (!ACE_OS::strcmp (name_in.c_str (),
  //                     ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_RENDERER_NULL_MODULE_NAME)))
  //  return const_cast<Stream_CameraAR_MediaFoundation_DisplayNull_Module*> (&displayNull_);

  return inherited::find (name_in);
}

void
Stream_CameraAR_MediaFoundation_Stream::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::start"));

  // sanity check(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  inherited::start ();
}

void
Stream_CameraAR_MediaFoundation_Stream::stop (bool waitForCompletion_in,
                                              bool recurseUpstream_in,
                                              bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::stop"));

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  inherited::stop (waitForCompletion_in,
                   recurseUpstream_in,
                   highPriority_in);
}

HRESULT
Stream_CameraAR_MediaFoundation_Stream::QueryInterface (const IID& IID_in,
                                                            void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (Stream_CameraAR_MediaFoundation_Stream, IMFAsyncCallback),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}
ULONG
Stream_CameraAR_MediaFoundation_Stream::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
ULONG
Stream_CameraAR_MediaFoundation_Stream::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0);
  //delete this;

  return count;
}

HRESULT
Stream_CameraAR_MediaFoundation_Stream::GetParameters (DWORD* flags_out,
                                                       DWORD* queue_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::GetParameters"));

  ACE_UNUSED_ARG (flags_out);
  ACE_UNUSED_ARG (queue_out);

  // *NOTE*: "...If you want default values for both parameters, return
  //         E_NOTIMPL. ..."
  return E_NOTIMPL;
}

HRESULT
Stream_CameraAR_MediaFoundation_Stream::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::Invoke"));

  HRESULT result = E_FAIL;
  IMFMediaEvent* media_event_p = NULL;
  MediaEventType event_type = MEUnknown;
  HRESULT status = E_FAIL;
  struct tagPROPVARIANT value;
  PropVariantInit (&value);

  // sanity check(s)
  ACE_ASSERT (result_in);
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  ACE_ASSERT (media_event_p);
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
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionClosed:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                  ACE_TEXT (stream_name_string_)));
      //IMFMediaSource* media_source_p = NULL;
      //if (!Stream_Device_Tools::getMediaSource (mediaSession_,
      //                                          media_source_p))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Stream_Device_Tools::getMediaSource(), continuing\n")));
      //  goto continue_;
      //} // end IF
      //ACE_ASSERT (media_source_p);
      //result = media_source_p->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSource::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      //media_source_p->Release (); media_source_p = NULL;
  //continue_:
      // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
      //result = mediaSession_->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      break;
    }
    case MESessionEnded:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionEnded, closing sesion\n"),
                  ACE_TEXT (stream_name_string_)));
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
      result = mediaSession_->Close ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                    ACE_TEXT (stream_name_string_),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
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
        stop (false, // wait ?
              true,  // high priority ?
              true); // locked access ?
      break;
    }
    case MESessionTopologySet:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologySet (status was: \"%s\")\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
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
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (topology_status).c_str ())));
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
  media_event_p->Release (); media_event_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  return S_OK;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
error:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}

bool
Stream_CameraAR_MediaFoundation_Stream::load (Stream_ILayout* layout_in,
                                              bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  layout_in->append (&source_, NULL, 0);
  // modules_out.push_back (&statisticReport_);
#if defined(FFMPEG_SUPPORT)
  layout_in->append (&convert_, NULL, 0);
  layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
#endif // FFMPEG_SUPPORT
  layout_in->append (&flip_, NULL, 0);

  return true;
}

bool
Stream_CameraAR_MediaFoundation_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Stream_CameraAR_MediaFoundation_SessionData* session_data_p = NULL;
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Stream_CameraAR_MediaFoundation_Source* source_impl_p = NULL;
  Test_I_MediaFoundation_SessionManager_t* session_manager_p =
    Test_I_MediaFoundation_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<Stream_CameraAR_MediaFoundation_SessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* Camera Source ************************
  source_impl_p =
    dynamic_cast<Stream_CameraAR_MediaFoundation_Source*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Strean_CamSave_MediaFoundation_CamSource> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF

  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  IMFTopology* topology_p = NULL;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFMediaType* media_type_p = NULL;
  TOPOID node_id = 0, node_id_2 = 0;

  result_2 = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  COM_initialized = true;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  if ((*iterator).second.second->session)
  {
    ULONG reference_count = (*iterator).second.second->session->AddRef ();
    mediaSession_ = (*iterator).second.second->session;

    if (!Stream_MediaFramework_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Device_MediaFoundation_Tools::clear(), aborting\n"),
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
    ACE_ASSERT (topology_p);

    //if ((*iterator).second.second->sampleGrabberNodeId)
    //  goto continue_;
    if (!Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                              node_id))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF
    ACE_ASSERT (node_id);

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    goto continue_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  if (!Stream_Module_Decoder_Tools::loadVideoRendererTopology ((*iterator).second.second->deviceIdentifier,
                                                               configuration_in.configuration_->format,
                                                               source_impl_p,
                                                               NULL,
                                                               //configuration_p->window,
                                                               node_id,
                                                               node_id_2,
                                                               topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::loadVideoRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (Common_OS_Tools::GUIDToString ((*iterator).second.second->deviceIdentifier.identifier._guid))));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;
#if defined (_DEBUG)
  Stream_MediaFramework_MediaFoundation_Tools::dump (topology_p);
#endif // _DEBUG

continue_:
  if (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                              configuration_in.configuration_->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: capture format: \"%s\"\n"),
              ACE_TEXT (stream_name_string_),
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (configuration_in.configuration_->format).c_str ())));

  ACE_ASSERT (session_data_p->formats.empty ());
  media_type_p =
    Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_in.configuration_->format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  session_data_p->formats.push_back (media_type_p);
  media_type_p = NULL;

  if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                     node_id,
                                                                     media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  session_data_p->formats.push_back (media_type_p);
  media_type_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  //HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
    //result = mediaSession_->Shutdown ();
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
  ACE_ASSERT (!mediaSession_);
  if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                 mediaSession_,
                                                                 true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  topology_p->Release (); topology_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  if (!(*iterator).second.second->session)
  {
    ULONG reference_count = mediaSession_->AddRef ();
    (*iterator).second.second->session = mediaSession_;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

  if (configuration_in.configuration_->setupPipeline)
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
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (session_data_p->direct3DDevice)
  {
    session_data_p->direct3DDevice->Release (); session_data_p->direct3DDevice = NULL;
  } // end IF
  session_data_p->direct3DManagerResetToken = 0;
  Stream_MediaFramework_MediaFoundation_Tools::free (session_data_p->formats);
  if (session_data_p->session)
  {
    session_data_p->session->Release (); session_data_p->session = NULL;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  if (COM_initialized)
    CoUninitialize ();

  return false;
}
#else
Stream_CameraAR_Stream::Stream_CameraAR_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING))
 , statisticReport_ (this,
                     ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 , convert_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING))
 , resize_ (this,
            ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_LIBAV_RESIZE_DEFAULT_NAME_STRING))
 , flip_ (this,
          ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_RGB_HFLIP_DEFAULT_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_Stream::Stream_CameraAR_Stream"));

}

bool
Stream_CameraAR_Stream::load (Stream_ILayout* layout_in,
                              bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_Stream::load"));

  // initialize return value(s)
  delete_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  layout_in->append (&source_, NULL, 0);
  //layout_in->append (&statisticReport_, NULL, 0);
  layout_in->append (&convert_, NULL, 0);
  layout_in->append (&resize_, NULL, 0); // output is window size/fullscreen
  layout_in->append (&flip_, NULL, 0);

  return true;
}

bool
Stream_CameraAR_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CameraAR_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  ACE_ASSERT (configuration_in.configuration_);
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  Stream_CameraAR_V4L_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_V4L_SessionManager_t* session_manager_p =
      Test_I_V4L_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<Stream_CameraAR_V4L_SessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  ACE_ASSERT (session_data_p->formats.empty ());
  session_data_p->formats.push_back (configuration_in.configuration_->format);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
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
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
#endif // ACE_WIN32 || ACE_WIN64
