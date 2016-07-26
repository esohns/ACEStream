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

#include "test_i_target_message.h"
#include "test_i_target_session_message.h"

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

  // ************************ Splitter *****************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("Splitter")));
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
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
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
