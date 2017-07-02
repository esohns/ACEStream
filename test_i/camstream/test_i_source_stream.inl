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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <qedit.h>
#endif

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif

#include "test_i_common_modules.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                  ConfigurationType,
                                  HandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  MessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::Test_I_Source_DirectShow_Stream_T ()
 : inherited ()
 , graphBuilder_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_DirectShow_Stream_T::Test_I_Source_DirectShow_Stream_T"));

}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                  ConfigurationType,
                                  HandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  MessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::~Test_I_Source_DirectShow_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_DirectShow_Stream_T::~Test_I_Source_DirectShow_Stream_T"));

  // *NOTE*: implements an ordered shutdown on destruction
  inherited::shutdown ();

  if (graphBuilder_)
    graphBuilder_->Release ();
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                  ConfigurationType,
                                  HandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  MessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::load (Stream_ModuleList_t& modules_out,
                                                        bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_DirectShow_Stream_T::load"));

  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
  if ((*iterator).second.window)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_Source_DirectShow_Display_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR ("Display"),
                                                             NULL,
                                                             false),
                    false);
    modules_out.push_back (module_p);
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  //else
//  //{
//  //  ACE_NEW_RETURN (module_p,
//  //                  Test_I_Source_DirectShow_DisplayNull_Module (ACE_TEXT_ALWAYS_CHAR ("DisplayNull"),
//  //                                                               NULL,
//  //                                                               false),
//  //                  false);
//  //  modules_out.push_back (module_p);
//  //} // end ELSE
//#endif
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR ("NetTarget"),
                                   NULL,
                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Source_DirectShow_StatisticReport_Module (this,
                                                                   ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                                   NULL,
                                                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_DirectShow_CamSource_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR ("CamSource"),
                                                             NULL,
                                                             false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                  ConfigurationType,
                                  HandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  MessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  ConnectorType>::initialize (const CONFIGURATION_T& configuration_in)
#else
                                  ConnectorType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_DirectShow_Stream_T::initialize"));

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  Test_I_Stream_DirectShow_CamSource* source_impl_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  // ******************* Camera Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("CamSource")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT ("CamSource")));
    return false;
  } // end IF
  source_impl_p =
    dynamic_cast<Test_I_Stream_DirectShow_CamSource*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_I_Stream_DirectShow_CamSource> failed, aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    return false;
  } // end IF

  // ---------------------------------------------------------------------------

  struct _AllocatorProperties allocator_properties;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  bool COM_initialized = false;
  bool release_builder = false;
  HRESULT result = E_FAIL;
  ULONG reference_count = 0;
  IAMStreamConfig* stream_config_p = NULL;
  IMediaFilter* media_filter_p = NULL;
  IDirect3DDeviceManager9* direct3D_manager_p = NULL;
  struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters;
  Stream_Module_Device_DirectShow_Graph_t graph_configuration;
  struct Stream_Module_Device_DirectShow_GraphEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  ISampleGrabber* isample_grabber_p = NULL;
  std::string log_file_name;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED     |
                            COINIT_DISABLE_OLE1DDE   |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  COM_initialized = true;

  if ((*iterator).second.builder)
  {
    reference_count = (*iterator).second.builder->AddRef ();
    graphBuilder_ = (*iterator).second.builder;

    // *NOTE*: Stream_Module_Device_Tools::loadRendererGraph() resets the graph
    //         (see below)
    if (!Stream_Module_Device_DirectShow_Tools::resetGraph (graphBuilder_,
                                                            CLSID_VideoInputDeviceCategory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::resetGraph(): \"%s\", aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF

    if (!Stream_Module_Device_DirectShow_Tools::getBufferNegotiation (graphBuilder_,
                                                                      MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                                                      buffer_negotiation_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::getBufferNegotiation(), aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (buffer_negotiation_p);

    goto continue_;
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::loadDeviceGraph ((*iterator).second.device,
                                                               CLSID_VideoInputDeviceCategory,
                                                               graphBuilder_,
                                                               buffer_negotiation_p,
                                                               stream_config_p,
                                                               graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT ((*iterator).second.device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graphBuilder_);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (stream_config_p);

  // clean up
  stream_config_p->Release ();
  stream_config_p = NULL;

  reference_count = graphBuilder_->AddRef ();
  (*iterator).second.builder = graphBuilder_;
  release_builder = true;

continue_:
  if (!Stream_Module_Device_DirectShow_Tools::setCaptureFormat (graphBuilder_,
                                                                CLSID_VideoInputDeviceCategory,
                                                                *(*iterator).second.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
#if defined (_DEBUG)
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("capture format: \"%s\"...\n"),
  //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*configuration_in.moduleHandlerConfiguration->format).c_str ())));

  log_file_name = Common_File_Tools::getLogDirectory (std::string (),
                                                      0);
  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
  Stream_Module_Device_DirectShow_Tools::debug (graphBuilder_,
                                                log_file_name);
#endif

  // sanity check(s)
  ACE_ASSERT (!session_data_r.direct3DDevice);

  if (!Stream_Module_Device_Tools::getDirect3DDevice ((*iterator).second.window,
                                                      *(*iterator).second.format,
                                                      session_data_r.direct3DDevice,
                                                      d3d_presentation_parameters,
                                                      direct3D_manager_p,
                                                      session_data_r.resetToken))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_Tools::getDirect3DDevice(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (direct3D_manager_p);

  if (!Stream_Module_Device_DirectShow_Tools::loadVideoRendererGraph (CLSID_VideoInputDeviceCategory,
                                                                      *(*iterator).second.format,
                                                                      (*iterator).second.window,
                                                                      graphBuilder_,
                                                                      graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::loadVideoRendererGraph(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF

  result =
    (*iterator).second.builder->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB,
                                                   &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result = filter_p->QueryInterface (IID_ISampleGrabber,
                                     (void**)&isample_grabber_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_ISampleGrabber): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (isample_grabber_p);
  filter_p->Release ();
  filter_p = NULL;

  result = isample_grabber_p->SetBufferSamples (false);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetBufferSamples(false): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = isample_grabber_p->SetCallback (source_impl_p, 0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISampleGrabber::SetCallback(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  isample_grabber_p->Release ();
  isample_grabber_p = NULL;

  ACE_ASSERT (buffer_negotiation_p);
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  allocator_properties.cbAlign = 1;
  allocator_properties.cbBuffer =
    configuration_in.allocatorConfiguration_.defaultBufferSize;
  allocator_properties.cbPrefix = -1; // <-- use default
  allocator_properties.cBuffers =
    MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
  result =
      buffer_negotiation_p->SuggestAllocatorProperties (&allocator_properties);
  if (FAILED (result)) // E_UNEXPECTED: 0x8000FFFF --> graph already connected
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IAMBufferNegotiation::SuggestAllocatorProperties(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF

  if (!Stream_Module_Device_DirectShow_Tools::connect (graphBuilder_,
                                                       graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_Tools::connect(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
  // *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //         null renderer 'breaks' the connection between the AVI decompressor
  //         and the sample grabber (go ahead, try it in with graphedit.exe)
  //         --> reconnect the AVI decompressor to the (connected) sample
  //             grabber; this seems to work
  if (!Stream_Module_Device_DirectShow_Tools::connected (graphBuilder_,
                                                         MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reconnecting...\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));

    if (!Stream_Module_Device_DirectShow_Tools::connectFirst (graphBuilder_,
                                                              MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::connectFirst(), aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (Stream_Module_Device_DirectShow_Tools::connected (graphBuilder_,
                                                                MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO));

  // debug info
  // *TODO*: find out why this fails
  ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
  result =
      buffer_negotiation_p->GetAllocatorProperties (&allocator_properties);
  if (FAILED (result)) // E_FAIL (0x80004005)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s/%s: failed to IAMBufferNegotiation::GetAllocatorProperties(): \"%s\", continuing\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Tools::errorToString (result, true).c_str ())));
    //goto error;
  } // end IF
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: negotiated allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                allocator_properties.cBuffers,
                allocator_properties.cbBuffer,
                allocator_properties.cbAlign,
                allocator_properties.cbPrefix));
  buffer_negotiation_p->Release ();
  buffer_negotiation_p = NULL;

  result = graphBuilder_->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release ();
  media_filter_p = NULL;

  if (session_data_r.format)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_r.format);
  ACE_ASSERT (!session_data_r.format);
  if (!Stream_Module_Device_DirectShow_Tools::getOutputFormat (graphBuilder_,
                                                               MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB,
                                                               session_data_r.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::getOutputFormat(\"%s\"), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB)));
    goto error;
  } // end IF
  ACE_ASSERT (session_data_r.format);

  // ---------------------------------------------------------------------------

  source_impl_p->set (&(inherited::state_));
  //fileReader_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF

  // ---------------------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_p =
  //    &const_cast<Test_I_Source_Stream_SessionData&> (inherited::sessionData_->get ());
  //session_data_p->fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_p->size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  if (release_builder)
  {
    (*iterator).second.builder->Release ();
    (*iterator).second.builder = NULL;
  } // end IF
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release ();
    session_data_r.direct3DDevice = NULL;
  } // end IF
  if (session_data_r.format)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_r.format);
  session_data_r.resetToken = 0;
  if (graphBuilder_)
  {
    graphBuilder_->Release ();
    graphBuilder_ = NULL;
  } // end IF

  if (COM_initialized)
    CoUninitialize ();

  return false;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                  ConfigurationType,
                                  HandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  MessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::collect (struct Test_I_Source_Stream_StatisticData& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_DirectShow_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  struct Test_I_Source_DirectShow_SessionData& session_data_r =
      const_cast<struct Test_I_Source_DirectShow_SessionData&> (inherited::sessionData_->get ());
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("RuntimeStatistic")));
    return false;
  } // end IF
  Test_I_Source_DirectShow_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
    dynamic_cast<Test_I_Source_DirectShow_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_DirectShow_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
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
    result_2 = runtimeStatistic_impl_p->collect (data_out);
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

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                  ConfigurationType,
                                  HandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  MessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_DirectShow_Stream_T::report"));

//   Net_Module_Statistic_ReaderTask_t* runtimeStatistic_impl =
//     dynamic_cast<Net_Module_Statistic_ReaderTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Net_Module_Statistic_ReaderTask_t> failed, returning\n")));
//     return;
//   } // end IF
//
//   // delegate to this module
//   return (runtimeStatistic_impl->report ());

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_DirectShow_Stream_T<StreamStateType,
                                  ConfigurationType,
                                  HandlerConfigurationType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ControlMessageType,
                                  MessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_DirectShow_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::Test_I_Source_MediaFoundation_Stream_T ()
 : inherited ()
 , inherited2 ()
 , mediaSession_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::Test_I_Source_MediaFoundation_Stream_T"));

}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::~Test_I_Source_MediaFoundation_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::~Test_I_Source_MediaFoundation_Stream_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif

  // *NOTE*: implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::start"));

  inherited::start ();

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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    // clean up
    PropVariantClear (&property_s);

    return;
  } // end IF
  PropVariantClear (&property_s);

  IMFAsyncCallback* callback_p = this;
  result = mediaSession_->BeginGetEvent (callback_p, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::BeginGetEvent(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
}
template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::stop (bool waitForCompletion_in,
                                                             bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::stop"));

  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Stop ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  } // end IF

  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::load (Stream_ModuleList_t& modules_out,
                                                             bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::load"));

  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  Stream_Module_t* module_p = NULL;
  if ((*iterator).second.window)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_Source_MediaFoundation_Display_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR ("Display"),
                                                                  NULL,
                                                                  false),
                    false);
    modules_out.push_back (module_p);
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  //else
//  //{
//  //  ACE_NEW_RETURN (module_p,
//  //                  Test_I_Source_MediaFoundation_DisplayNull_Module (ACE_TEXT_ALWAYS_CHAR ("DisplayNull"),
//  //                                                                    NULL,
//  //                                                                    false),
//  //                  false);
//  //  modules_out.push_back (module_p);
//  //} // end ELSE
//#endif
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR ("NetTarget"),
                                   NULL,
                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Source_MediaFoundation_StatisticReport_Module (this,
                                                                        ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                                        NULL,
                                                                        false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_MediaFoundation_CamSource_Module (this,
                                                                  ACE_TEXT_ALWAYS_CHAR ("CamSource"),
                                                                  NULL,
                                                                  false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                       ConnectorType>::initialize (const CONFIGURATION_T& configuration_in)
#else
                                       ConnectorType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::initialize"));

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());

  // ---------------------------------------------------------------------------
  // sanity check(s)
  //ACE_ASSERT (configuration_in.moduleConfiguration);

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  Test_I_Stream_MediaFoundation_CamSource* source_impl_p = NULL;
  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());

  // ******************* Camera Source ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("CamSource")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT ("CamSource")));
    return false;
  } // end IF
  source_impl_p =
    dynamic_cast<Test_I_Stream_MediaFoundation_CamSource*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_I_Stream_MediaFoundation_CamSource> failed, aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    return false;
  } // end IF

  bool graph_loaded = false;
  bool COM_initialized = false;
  HRESULT result = E_FAIL;
  IMFTopology* topology_p = NULL;

  result = CoInitializeEx (NULL,
                           (COINIT_MULTITHREADED     |
                            COINIT_DISABLE_OLE1DDE   |
                            COINIT_SPEED_OVER_MEMORY));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  COM_initialized = true;

  UINT32 item_count = 0;
  ULONG reference_count = 0;

  IMFMediaType* media_type_p = NULL;
  if (!(*iterator).second.format)
  {
    result = MFCreateMediaType (&(*iterator).second.format);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to MFCreateMediaType(): \"%s\", aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ()),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF
  if (!Stream_Module_Device_MediaFoundation_Tools::copyMediaType ((*iterator).second.format,
                                                                  media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::copyMediaType(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF

  if (!Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology ((*iterator).second.device,
                                                                              media_type_p,
                                                                              source_impl_p,
                                                                              NULL,
                                                                              //(*iterator).second.window,
                                                                              (*iterator).second.sampleGrabberNodeId,
                                                                              session_data_r.rendererNodeId,
                                                                              topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::loadVideoRendererTopology(\"%s\"), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT ((*iterator).second.device.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);
  graph_loaded = true;

  // set default capture media type ?
  result = media_type_p->GetCount (&item_count);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  if (!item_count)
  {
    IMFMediaSource* media_source_p = NULL;
    if (!Stream_Module_Device_MediaFoundation_Tools::getMediaSource (topology_p,
                                                                     media_source_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_Tools::getMediaSource(), aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF
    if (!Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                                       media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));

      // clean up
      media_source_p->Release ();

      goto error;
    } // end IF
    media_source_p->Release ();

    if (!Stream_Module_Device_MediaFoundation_Tools::copyMediaType (media_type_p,
                                                                    (*iterator).second.format))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::copyMediaType(), aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF
  } // end IF
  else if (!Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat (topology_p,
                                                                          media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: capture format: \"%s\"...\n"),
              ACE_TEXT (inherited::configuration_->name_.c_str ()),
              ACE_TEXT (Stream_Module_Device_MediaFoundation_Tools::mediaTypeToString (media_type_p).c_str ())));
#endif
  media_type_p->Release ();
  media_type_p = NULL;

  if (session_data_r.format)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_r.format);
  ACE_ASSERT (!session_data_r.format);
  session_data_r.format =
    static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
  if (!session_data_r.format)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, continuing\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
  ACE_OS::memset (session_data_r.format, 0, sizeof (struct _AMMediaType));
  ACE_ASSERT (!session_data_r.format->pbFormat);
  if (!Stream_Module_Device_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                    (*iterator).second.sampleGrabberNodeId,
                                                                    media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::getOutputFormat(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  result = MFInitAMMediaTypeFromMFMediaType (media_type_p,
                                             GUID_NULL,
                                             session_data_r.format);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFInitAMMediaTypeFromMFMediaType(): \"%m\", aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ()),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_type_p->Release ();
  media_type_p = NULL;
  ACE_ASSERT (session_data_r.format);

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
  if ((*iterator).second.session)
  {
    reference_count = (*iterator).second.session->AddRef ();
    mediaSession_ = (*iterator).second.session;

    if (!Stream_Module_Device_MediaFoundation_Tools::clear (mediaSession_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::clear(), aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF
  } // end IF

  if (!Stream_Module_Device_MediaFoundation_Tools::setTopology (topology_p,
                                                                mediaSession_,
                                                                true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_MediaFoundation_Tools::setTopology(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF
  topology_p->Release ();
  topology_p = NULL;
  ACE_ASSERT (mediaSession_);

  if (!(*iterator).second.session)
  {
    reference_count = mediaSession_->AddRef ();
    (*iterator).second.session = mediaSession_;
  } // end IF

  // -------------------------------------------------------------

  source_impl_p->set (&(inherited::state_));
  //fileReader_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_p =
  //    &const_cast<Test_I_Source_Stream_SessionData&> (inherited::sessionData_->get ());
  //session_data_p->fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_p->size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // *TODO*: remove type inferences
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.mediaFoundationConfiguration->mediaSession =
    mediaSession_;
  if (!inherited2::initialize (*configuration_in.configuration_.mediaFoundationConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Misc_MediaFoundation_Callback_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    goto error;
  } // end IF

  inherited::isInitialized_ = true;

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
  {
#if defined (_DEBUG)
    Stream_Module_Device_MediaFoundation_Tools::dump (topology_p);
#endif
    topology_p->Release ();
  } // end IF
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release ();
    session_data_r.direct3DDevice = NULL;
  } // end IF
  if (session_data_r.format)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (session_data_r.format);
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

  return false;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::collect (struct Test_I_Source_Stream_StatisticData& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  struct Test_I_Source_MediaFoundation_SessionData& session_data_r =
      const_cast<struct Test_I_Source_MediaFoundation_SessionData&> (inherited::sessionData_->get ());
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("RuntimeStatistic")));
    return false;
  } // end IF
  Test_I_Source_MediaFoundation_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
    dynamic_cast<Test_I_Source_MediaFoundation_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_MediaFoundation_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
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
    result_2 = runtimeStatistic_impl_p->collect (data_out);
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

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::report"));

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

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_MediaFoundation_Stream_T<StreamStateType,
                                       ConfigurationType,
                                       HandlerConfigurationType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       ControlMessageType,
                                       MessageType,
                                       SessionMessageType,
                                       ConnectionManagerType,
                                       ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_MediaFoundation_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
#else
template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_V4L2_Stream_T<StreamStateType,
                            ConfigurationType,
                            HandlerConfigurationType,
                            SessionDataType,
                            SessionDataContainerType,
                            ControlMessageType,
                            MessageType,
                            SessionMessageType,
                            ConnectionManagerType,
                            ConnectorType>::Test_I_Source_V4L2_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_V4L2_Stream_T::Test_I_Source_V4L2_Stream_T"));

}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_V4L2_Stream_T<StreamStateType,
                            ConfigurationType,
                            HandlerConfigurationType,
                            SessionDataType,
                            SessionDataContainerType,
                            ControlMessageType,
                            MessageType,
                            SessionMessageType,
                            ConnectionManagerType,
                            ConnectorType>::~Test_I_Source_V4L2_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_V4L2_Stream_T::~Test_I_Source_V4L2_Stream_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result = E_FAIL;
  if (mediaSession_)
  {
    result = mediaSession_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
#endif

  // *NOTE*: implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_V4L2_Stream_T<StreamStateType,
                            ConfigurationType,
                            HandlerConfigurationType,
                            SessionDataType,
                            SessionDataContainerType,
                            ControlMessageType,
                            MessageType,
                            SessionMessageType,
                            ConnectionManagerType,
                            ConnectorType>::load (Stream_ModuleList_t& modules_out,
                                                  bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_V4L2_Stream_T::load"));

  Stream_Module_t* module_p = NULL;
  // *TODO*: remove type inference
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
      inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  if ((*iterator).second.window)
  {
    ACE_NEW_RETURN (module_p,
                    Test_I_Source_V4L2_Display_Module (this,
                                                       ACE_TEXT_ALWAYS_CHAR ("Display"),
                                                       NULL,
                                                       false),
                    false);
    modules_out.push_back (module_p);
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  //else
//  //{
//  //  ACE_NEW_RETURN (module_p,
//  //                  Test_I_Source_DisplayNull_Module (ACE_TEXT_ALWAYS_CHAR ("DisplayNull"),
//  //                                                    NULL,
//  //                                                    false),
//  //                  false);
//  //  modules_out.push_back (module_p);
//  //} // end ELSE
//#endif
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR ("NetTarget"),
                                   NULL,
                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Source_V4L2_StatisticReport_Module (this,
                                                             ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                             NULL,
                                                             false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Source_V4L2_CamSource_Module (this,
                                                       ACE_TEXT_ALWAYS_CHAR ("CamSource"),
                                                       NULL,
                                                       false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_V4L2_Stream_T<StreamStateType,
                            ConfigurationType,
                            HandlerConfigurationType,
                            SessionDataType,
                            SessionDataContainerType,
                            ControlMessageType,
                            MessageType,
                            SessionMessageType,
                            ConnectionManagerType,
                            ConnectorType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_V4L2_Stream_T::initialize"));

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<ConfigurationType&> (configuration_in).configuration_.setupPipeline =
      false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::configuration_->name_.c_str ())));
    return false;
  } // end IF
  const_cast<ConfigurationType&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *TODO*: remove type inferences
  typename ConfigurationType::ITERATOR_T iterator =
      const_cast<ConfigurationType&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  session_data_r.v4l2Format = (*iterator).second.v4l2Format;
  session_data_r.v4l2FrameRate = (*iterator).second.v4l2FrameRate;
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
  session_data_r.format = (*iterator).second.format;
  session_data_r.height = session_data_r.v4l2Format.fmt.pix.height;
  session_data_r.width = session_data_r.v4l2Format.fmt.pix.width;
#endif

  // ---------------------------------------------------------------------------
  // sanity check(s)
//  ACE_ASSERT (configuration_in.moduleConfiguration);

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  Test_I_Source_V4L2_CamSource* source_impl_p = NULL;

  // ******************* Camera Source ************************
  typename inherited::MODULE_T* module_p =
    const_cast<typename inherited::MODULE_T*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("CamSource")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("CamSource")));
    return false;
  } // end IF
  source_impl_p =
    dynamic_cast<Test_I_Source_V4L2_CamSource*> (module_p->writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_V4L2_CamSource> failed, aborting\n")));
    return false;
  } // end IF
  source_impl_p->set (&(inherited::state_));
  //fileReader_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::configuration_->name_.c_str ())));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  //// *TODO*: remove type inferences
  //session_data_p =
  //    &const_cast<Test_I_Source_Stream_SessionData&> (inherited::sessionData_->get ());
  //session_data_p->fileName =
  //  configuration_in.moduleHandlerConfiguration->fileName;
  //session_data_p->size =
  //  Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

  // OK: all went well
  inherited::isInitialized_ = true;

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<ConfigurationType&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;

  return false;
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_V4L2_Stream_T<StreamStateType,
                            ConfigurationType,
                            HandlerConfigurationType,
                            SessionDataType,
                            SessionDataContainerType,
                            ControlMessageType,
                            MessageType,
                            SessionMessageType,
                            ConnectionManagerType,
                            ConnectorType>::collect (struct Test_I_Source_Stream_StatisticData& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_V4L2_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Source_V4L2_SessionData& session_data_r =
      const_cast<Test_I_Source_V4L2_SessionData&> (inherited::sessionData_->get ());
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("RuntimeStatistic")));
    return false;
  } // end IF
  Test_I_Source_Statistic_WriterTask_t* statistic_impl_p =
    dynamic_cast<Test_I_Source_Statistic_WriterTask_t*> (module_p->writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
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
    result_2 = statistic_impl_p->collect (data_out);
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

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_V4L2_Stream_T<StreamStateType,
                            ConfigurationType,
                            HandlerConfigurationType,
                            SessionDataType,
                            SessionDataContainerType,
                            ControlMessageType,
                            MessageType,
                            SessionMessageType,
                            ConnectionManagerType,
                            ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_V4L2_Stream_T::report"));

//   Net_Module_Statistic_ReaderTask_t* statistic_impl =
//     dynamic_cast<Net_Module_Statistic_ReaderTask_t*> (//runtimeStatistic_.writer ());
//   if (!statistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Net_Module_Statistic_ReaderTask_t> failed, returning\n")));
//     return;
//   } // end IF
//
//   // delegate to this module...
//   return (statistic_impl->report ());

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename StreamStateType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Test_I_Source_V4L2_Stream_T<StreamStateType,
                            ConfigurationType,
                            HandlerConfigurationType,
                            SessionDataType,
                            SessionDataContainerType,
                            ControlMessageType,
                            MessageType,
                            SessionMessageType,
                            ConnectionManagerType,
                            ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_V4L2_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
#endif
