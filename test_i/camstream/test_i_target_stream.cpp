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

#include "test_i_target_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "test_i_common_modules.h"
#include "test_i_source_stream.h"

Test_I_Target_Stream::Test_I_Target_Stream (const std::string& name_in)
 : inherited (name_in)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaSession_ (NULL)
 , referenceCount_ (1)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::Test_I_Target_Stream"));

}

Test_I_Target_Stream::~Test_I_Target_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::~Test_I_Target_Stream"));

  inherited::shutdown ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //if (_DEBUG && graphBuilder_)
  //  Stream_Module_Device_Tools::debug (graphBuilder_,
  //                                     std::string ());

  //if (graphBuilder_)
  //  graphBuilder_->Release ();
#endif
}

bool
Test_I_Target_Stream::load (Stream_ModuleList_t& modules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::load"));

//  // initialize return value(s)
//  for (Stream_ModuleListIterator_t iterator = modules_out.begin ();
//       iterator != modules_out.end ();
//       iterator++)
//    delete *iterator;
//  modules_out.clear ();

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->moduleHandlerConfiguration);

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Target_Stream_Module_Display_Module (ACE_TEXT_ALWAYS_CHAR ("Display"),
                                                              NULL,
                                                              false),
                  false);
  modules_out.push_back (module_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_NEW_RETURN (module_p,
//                  Test_I_Target_Stream_Module_DisplayNull_Module (ACE_TEXT_ALWAYS_CHAR ("DisplayNull"),
//                                                                  NULL,
//                                                                  false),
//                  false);
//  modules_out.push_back (module_p);
#endif
  module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //Test_I_Target_Stream_Module_DirectShowSource_Module directShowSource_;
  //Test_I_Target_Stream_Module_MediaFoundationSource_Module mediaFoundationSource_;
#endif
  ACE_NEW_RETURN (module_p,
                  Test_I_Target_Stream_Module_RuntimeStatistic_Module (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                                                                       NULL,
                                                                       false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
 ACE_NEW_RETURN (module_p,
                  Test_I_Target_Stream_Module_Splitter_Module (ACE_TEXT_ALWAYS_CHAR ("Splitter"),
                                                               NULL,
                                                               false),
                  false);
  modules_out.push_back (module_p);
  //Test_I_Target_Stream_Module_AVIDecoder_Module            decoder_;
  //Test_I_Target_Stream_Module_Net_IO_Module                source_;

  if (!inherited::load (modules_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::load(), aborting\n")));
    return false;
  } // end IF

  return true;
}

bool
Test_I_Target_Stream::initialize (const Test_I_Target_StreamConfiguration& configuration_in,
                                  bool setupPipeline_in,
                                  bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  // allocate a new session state, reset stream
  // sanity check(s)
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
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
  Test_I_Target_Stream_SessionData& session_data_r =
    const_cast<Test_I_Target_Stream_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.lock = &(inherited::lock_);
  inherited::state_.currentSessionData = &session_data_r;
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  session_data_r.format = configuration_in.moduleHandlerConfiguration->format;
#else
  session_data_r.format = configuration_in.moduleHandlerConfiguration->format;
#endif
  session_data_r.sessionID = configuration_in.sessionID;
  session_data_r.targetFileName =
    configuration_in.moduleHandlerConfiguration->targetFileName;

  ACE_ASSERT (configuration_in.moduleConfiguration);
  //  configuration_in.moduleConfiguration.streamState = &state_;
  Test_I_Target_StreamConfiguration& configuration_r =
      const_cast<Test_I_Target_StreamConfiguration&> (configuration_in);
  configuration_r.moduleHandlerConfiguration->stateMachineLock =
    &inherited::state_.stateMachineLock;

  // ---------------------------------------------------------------------------

  //Test_I_Target_Stream_Module_Display* display_impl_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //Test_I_Target_Stream_Module_DisplayNull* displayNull_impl_p = NULL;
#endif
  //Test_I_Target_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
//    NULL;
  Test_I_Target_Stream_Module_Splitter* splitter_impl_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //bool graph_loaded = false;
  //bool COM_initialized = false;
  //HRESULT result = E_FAIL;
  //IMFTopology* topology_p = NULL;
#endif

  // ******************* Display ************************
  // *TODO*: remove type inference
  if (configuration_in.moduleHandlerConfiguration->window)
  {
    //display_.initialize (*configuration_in.moduleConfiguration);
    //display_impl_p =
    //  dynamic_cast<Test_I_Target_Stream_Module_Display*> (display_.writer ());
    //if (!display_impl_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Display> failed, aborting\n")));
    //  goto error;
    //} // end IF
    //if (!display_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
    //              display_.name ()));
    //  goto error;
    //} // end IF
  } // end IF
  else
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //displayNull_.initialize (*configuration_in.moduleConfiguration);
    //displayNull_impl_p =
    //  dynamic_cast<Test_I_Target_Stream_Module_DisplayNull*> (displayNull_.writer ());
    //if (!displayNull_impl_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_DisplayNull> failed, aborting\n")));
    //  goto error;
    //} // end IF
    //if (!displayNull_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
    //              displayNull_.name ()));
    //  goto error;
    //} // end IF
#endif
  } // end ELSE

  // ******************* Runtime Statistic *************************
  //runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  //runtimeStatistic_impl_p =
  //    dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  //if (!runtimeStatistic_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_T> failed, aborting\n")));
  //  goto error;
  //} // end IF
  //if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval, // reporting interval
  //                                          true,                                        // push statistic messages
  //                                          configuration_in.printFinalReport,           // print final report ?
  //                                          configuration_in.messageAllocator))          // message allocator handle
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
  //              runtimeStatistic_.name ()));
  //  goto error;
  //} // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // ******************* DirectShow Source ************************
  //Test_I_Target_Stream_Module_DirectShowSource* directShowSource_impl_p =
  //  dynamic_cast<Test_I_Target_Stream_Module_DirectShowSource*> (directShowSource_.writer ());
  //if (!directShowSource_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_DirectShowSource>(%@) failed, aborting\n"),
  //              directShowSource_.writer ()));
  //  goto error;
  //} // end IF
  //Test_I_Target_Stream_Module_MediaFoundationSource* mediaFoundationSource_impl_p =
  //  dynamic_cast<Test_I_Target_Stream_Module_MediaFoundationSource*> (mediaFoundationSource_.writer ());
  //if (!mediaFoundationSource_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_MediaFoundationSource>(%@) failed, aborting\n"),
  //              mediaFoundationSource_.writer ()));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (configuration_r.moduleHandlerConfiguration);
  ////ACE_ASSERT (configuration_r.moduleHandlerConfiguration->filterConfiguration);
  ////ACE_ASSERT (configuration_r.moduleHandlerConfiguration->filterConfiguration->pinConfiguration);
  //configuration_r.moduleHandlerConfiguration->queue =
  //  mediaFoundationSource_impl_p->msg_queue ();

  //std::wstring filter_name;
  //std::list<std::wstring> filter_pipeline;
  //bool release_builder = false;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //bool graph_loaded = false;
  //bool COM_initialized = false;
  //HRESULT result = E_FAIL;
  //IMFTopology* topology_p = NULL;

  //result = CoInitializeEx (NULL,
  //  (COINIT_MULTITHREADED |
  //    COINIT_DISABLE_OLE1DDE |
  //    COINIT_SPEED_OVER_MEMORY));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
  //    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return false;
  //} // end IF
  //COM_initialized = true;

  //if (!Stream_Module_Device_Tools::loadRendererTopology (configuration_in.moduleHandlerConfiguration->device,
  //  configuration_in.moduleHandlerConfiguration->format,
  //  source_impl_p,
  //  NULL,
  //  //configuration_in.moduleHandlerConfiguration->window,
  //  configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
  //  session_data_r.rendererNodeId,
  //  topology_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //    ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererTopology(\"%s\"), aborting\n"),
  //    ACE_TEXT (configuration_in.moduleHandlerConfiguration->device.c_str ())));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (topology_p);
  //graph_loaded = true;

  //if (!Stream_Module_Device_Tools::setCaptureFormat (topology_p,
  //  configuration_in.moduleHandlerConfiguration->format))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //    ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //if (_DEBUG)
  //  ACE_DEBUG ((LM_DEBUG,
  //    ACE_TEXT ("capture format: \"%s\"...\n"),
  //    ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (configuration_in.moduleHandlerConfiguration->format).c_str ())));

  //if (session_data_r.format)
  //{
  //  session_data_r.format->Release ();
  //  session_data_r.format = NULL;
  //} // end IF
  //ACE_ASSERT (!session_data_r.format);
  //if (!Stream_Module_Device_Tools::getOutputFormat (topology_p,
  //  configuration_in.moduleHandlerConfiguration->sampleGrabberNodeId,
  //  session_data_r.format))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //    ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (session_data_r.format);

  //if (mediaSession_)
  //{
  //  // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
  //  //result = mediaSession_->Shutdown ();
  //  //if (FAILED (result))
  //  //  ACE_DEBUG ((LM_ERROR,
  //  //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
  //  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  mediaSession_->Release ();
  //  mediaSession_ = NULL;
  //} // end IF
  //ULONG reference_count = 0;
  //if (configuration_in.moduleHandlerConfiguration->session)
  //{
  //  reference_count =
  //    configuration_in.moduleHandlerConfiguration->session->AddRef ();
  //  mediaSession_ = configuration_in.moduleHandlerConfiguration->session;

  //  if (!Stream_Module_Device_Tools::clear (mediaSession_))
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //      ACE_TEXT ("failed to Stream_Module_Device_Tools::clear(), aborting\n")));
  //    goto error;
  //  } // end IF
  //} // end IF

  //if (!Stream_Module_Device_Tools::setTopology (topology_p,
  //  mediaSession_,
  //  true))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //    ACE_TEXT ("failed to Stream_Module_Device_Tools::setTopology(), aborting\n")));
  //  goto error;
  //} // end IF
  //topology_p->Release ();

  //if (!configuration_in.moduleHandlerConfiguration->session)
  //{
  //  reference_count = mediaSession_->AddRef ();
  //  configuration_in.moduleHandlerConfiguration->session = mediaSession_;
  //} // end IF
#endif

  //if (_DEBUG)
  //{
  //  std::string log_file_name =
  //    Common_File_Tools::getLogDirectory (std::string (),
  //                                        0);
  //  log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
  //  log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
  //  Stream_Module_Device_Tools::debug (configuration_r.moduleHandlerConfiguration->builder,
  //                                     log_file_name);
  //} // end IF

  //IBaseFilter* ibase_filter_p = NULL;
  //result_2 =
  //  CoCreateInstance (configuration_in.moduleHandlerConfiguration->filterCLSID, NULL,
  //                    CLSCTX_INPROC_SERVER, IID_IBaseFilter,
  //                    (void**)&ibase_filter_p);
  //if (FAILED (result_2))
  //{
  //  OLECHAR GUID_string[CHARS_IN_GUID];
  //  ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
  //  int nCount =
  //    StringFromGUID2 (configuration_in.moduleHandlerConfiguration->filterCLSID,
  //                     GUID_string, sizeof (GUID_string));
  //  ACE_ASSERT (nCount == CHARS_IN_GUID);
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
  //              ACE_TEXT_WCHAR_TO_TCHAR (GUID_string),
  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (ibase_filter_p);
  //// *TODO*: this should read
  ////         'Test_I_Target_Stream_Module_DirectShowSource::[ASYNCH_]FILTER_T::IINITIALIZE_T*'
  //Test_I_Target_Stream_Module_DirectShowSource::IINITIALIZE_T* iinitialize_p =
  //  dynamic_cast<Test_I_Target_Stream_Module_DirectShowSource::IINITIALIZE_T*> (ibase_filter_p);
  //if (!iinitialize_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to dynamic_cast<Common_IInitialize_T*>(%@), aborting\n"),
  //              ibase_filter_p));

  //  // clean up
  //  ibase_filter_p->Release ();

  //  goto error;
  //} // end IF
  //// sanity check(s)
  //// *TODO*: remove type inferences
  //ACE_ASSERT (configuration_r.moduleHandlerConfiguration->filterConfiguration);
  //if (!iinitialize_p->initialize (*configuration_in.moduleHandlerConfiguration->filterConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Common_IInitialize_T::initialize(), aborting\n")));

  //  // clean up
  //  ibase_filter_p->Release ();

  //  goto error;
  //} // end IF

  //filter_name =
  //  (configuration_r.moduleHandlerConfiguration->push ? TEST_I_STREAM_MODULE_DIRECTSHOW_SOURCE_FILTER_NAME
  //                                                    : TEST_I_STREAM_MODULE_DIRECTSHOW_ASYNCH_SOURCE_FILTER_NAME);
  ////ACE_TEXT_ALWAYS_WCHAR (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ());
  //result_2 =
  //  configuration_r.moduleHandlerConfiguration->builder->AddFilter (ibase_filter_p,
  //                                                                  filter_name.c_str ());
  //if (FAILED (result_2))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::AddFilter(\"%s\"): \"%s\", aborting\n"),
  //              ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  //  goto error;
  //} // end IF
  //filter_pipeline.push_front (filter_name);
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added \"%s\"...\n"),
  //            ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ())));

  //ACE_ASSERT (!filter_pipeline.empty ());
  //if (!Stream_Module_Device_Tools::connect (configuration_r.moduleHandlerConfiguration->builder,
  //                                          filter_pipeline))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::connect(), aborting\n")));
  //  goto error;
  //} // end IF
  ////// *NOTE*: for some (unknown) reason, connect()ing the sample grabber to the
  //////         null renderer 'breaks' the connection between the AVI decompressor
  //////         and the sample grabber (go ahead, try it in with graphedit.exe)
  //////         --> reconnect the AVI decompressor to the (connected) sample
  //////             grabber; this seems to work
  ////if (!Stream_Module_Device_Tools::connected (configuration_r.moduleHandlerConfiguration->builder))
  ////{
  ////  ACE_DEBUG ((LM_DEBUG,
  ////              ACE_TEXT ("reconnecting...\n")));

  ////  if (!Stream_Module_Device_Tools::connectFirst (configuration_r.moduleHandlerConfiguration->builder))
  ////  {
  ////    ACE_DEBUG ((LM_ERROR,
  ////                ACE_TEXT ("failed to Stream_Module_Device_Tools::connectFirst(), aborting\n")));
  ////    goto error;
  ////  } // end IF
  ////} // end IF
  ////ACE_ASSERT (Stream_Module_Device_Tools::connected (configuration_r.moduleHandlerConfiguration->builder));

  //IMediaFilter* media_filter_p = NULL;
  //result_2 =
  //  configuration_r.moduleHandlerConfiguration->builder->QueryInterface (IID_IMediaFilter,
  //                                                                       (void**)&media_filter_p);
  //if (FAILED (result_2))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (media_filter_p);
  //result_2 = media_filter_p->SetSyncSource (NULL);
  //if (FAILED (result_2))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
  //  goto error;
  //} // end IF
  //media_filter_p->Release ();

  //directShowSource_.initialize (*configuration_in.moduleConfiguration);
  //if (!directShowSource_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
  //              directShowSource_.name ()));
  //  return false;
  //} // end IF
  //mediaFoundationSource_.initialize (*configuration_in.moduleConfiguration);
  //if (!mediaFoundationSource_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
  //              mediaFoundationSource_.name ()));
  //  goto error;
  //} // end IF
#endif

  // ************************ Splitter *****************************
  Stream_Module_t* module_p =
      inherited::find (ACE_TEXT_ALWAYS_CHAR ("Splitter"));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("Splitter")));
    goto error;
  } // end IF
  splitter_impl_p =
    dynamic_cast<Test_I_Target_Stream_Module_Splitter*> (module_p->writer ());
  if (!splitter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Splitter> failed, aborting\n")));
    goto error;
  } // end IF
  if (!splitter_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
                module_p->name ()));
    goto error;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

//  // ************************ AVI Decoder *****************************
//  decoder_.initialize (*configuration_in.moduleConfiguration);
//  Test_I_Target_Stream_Module_AVIDecoder* decoder_impl_p =
//    dynamic_cast<Test_I_Target_Stream_Module_AVIDecoder*> (decoder_.writer ());
//  if (!decoder_impl_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_AVIDecoder> failed, aborting\n")));
//    goto error;
//  } // end IF
//  if (!decoder_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
//                decoder_.name ()));
//    goto error;
//  } // end IF
//  result_2 = inherited::push (&decoder_);
//  if (result_2 == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                decoder_.name ()));
//    goto error;
//  } // end IF

//  // ******************* Net Reader ***********************
//  source_.initialize (*configuration_in.moduleConfiguration);
//  Test_I_Target_Stream_Module_Net_Writer_t* source_impl_p =
//    dynamic_cast<Test_I_Target_Stream_Module_Net_Writer_t*> (source_.writer ());
//  if (!source_impl_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Net_Writer_T> failed, aborting\n")));
//    goto error;
//  } // end IF
//  if (!source_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
//                source_.name ()));
//    goto error;
//  } // end IF
//  if (!source_impl_p->initialize (inherited::state_))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize writer, aborting\n"),
//                source_.name ()));
//    goto error;
//  } // end IF
////  netReader_impl_p->reset ();
//  // *NOTE*: push()ing the module will open() it
//  //         --> set the argument that is passed along (head module expects a
//  //             handle to the session data)
//  source_.arg (inherited::sessionData_);
//  result_2 = inherited::push (&source_);
//  if (result_2 == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
//                source_.name ()));
//    goto error;
//  } // end IF

  // -------------------------------------------------------------

  if (setupPipeline_in)
    if (!inherited::setup (configuration_in.notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      goto error;
    } // end IF

  return true;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //if (release_builder)
  //{
  //  configuration_r.moduleHandlerConfiguration->builder->Release ();
  //  configuration_r.moduleHandlerConfiguration->builder = NULL;
  //} // end IF
  //if (direct3D_manager_p)
  //  direct3D_manager_p->Release ();
  //if (graph_loaded)
  //{
  //  //configuration_in.moduleHandlerConfiguration->builder->Release ();
  //  //configuration_in.moduleHandlerConfiguration->builder = NULL;
  //  //configuration_in.moduleHandlerConfiguration->sourceReader->Release ();
  //  //configuration_in.moduleHandlerConfiguration->sourceReader = NULL;
  //  configuration_in.moduleHandlerConfiguration->session->Release ();
  //  configuration_in.moduleHandlerConfiguration->session = NULL;
  //} // end IF
  if (session_data_r.direct3DDevice)
  {
    session_data_r.direct3DDevice->Release ();
    session_data_r.direct3DDevice = NULL;
  } // end IF
  if (session_data_r.format)
  {
    //Stream_Module_Device_Tools::deleteMediaType (session_data_r.format);
    session_data_r.format->Release ();
    session_data_r.format = NULL;
  } // end IF
  session_data_r.resetToken = 0;

  //if (COM_initialized)
  //  CoUninitialize ();
#endif

  return false;
}

void
Test_I_Target_Stream::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

bool
Test_I_Target_Stream::collect (Test_I_RuntimeStatistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Target_Stream_SessionData& session_data_r =
        const_cast<Test_I_Target_Stream_SessionData&> (inherited::sessionData_->get ());
  Stream_Module_t* module_p =
    inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("RuntimeStatistic")));
    return false;
  } // end IF
  Test_I_Target_Stream_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
    dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Target_Stream_Module_Statistic_WriterTask_t> failed, aborting\n")));
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

  // delegate to the statistics module...
  bool result_2 = false;
  try
  {
    result_2 = runtimeStatistic_impl_p->collect (data_out);
  }
  catch (...)
  {
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
Test_I_Target_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_Stream::report"));

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
