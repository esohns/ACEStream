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

//#include "ace/Synch.h"
#include "test_u_riffdecoder_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Test_U_RIFFDecoder_Stream::Test_U_RIFFDecoder_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR ("FileSource"))
 , decoder_ (this,
             ACE_TEXT_ALWAYS_CHAR ("Decoder"))
 , statistic_ (this,
               ACE_TEXT_ALWAYS_CHAR ("StatisticReport"))
{
  STREAM_TRACE (ACE_TEXT ("Test_U_RIFFDecoder_Stream::Test_U_RIFFDecoder_Stream"));

}

Test_U_RIFFDecoder_Stream::~Test_U_RIFFDecoder_Stream ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_RIFFDecoder_Stream::~Test_U_RIFFDecoder_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
Test_U_RIFFDecoder_Stream::load (Stream_ModuleList_t& modules_out,
                                 bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_RIFFDecoder_Stream::load"));

  // initialize return value(s)
  modules_out.clear ();
  delete_out = false;

  modules_out.push_back (&statistic_);
  modules_out.push_back (&decoder_);
  modules_out.push_back (&source_);

  return true;
}

bool
Test_U_RIFFDecoder_Stream::initialize (const inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_RIFFDecoder_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
//  struct Test_U_RIFFDecoder_SessionData* session_data_p = NULL;
  Test_U_RIFFDecoder_Module_Source* source_impl_p = NULL;

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
  ACE_ASSERT (inherited::sessionData_);
//  session_data_p =
//    &const_cast<struct Test_U_RIFFDecoder_SessionData&> (inherited::sessionData_->getR ());

  // things to be done here:
  // [- initialize base class]
  // ------------------------------------
  // - initialize notification strategy (if any)
  // ------------------------------------
  // - push the final module onto the stream (if any)
  // ------------------------------------
  // - initialize modules
  // - push them onto the stream (tail-first) !
  // ------------------------------------

//  int result = -1;
//  inherited::MODULE_T* module_p = NULL;
//  if (configuration_in.notificationStrategy)
//  {
//    module_p = inherited::head ();
//    if (!module_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("no head module found, aborting\n")));
//      return false;
//    } // end IF
//    inherited::TASK_T* task_p = module_p->reader ();
//    if (!task_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("no head module reader task found, aborting\n")));
//      return false;
//    } // end IF
//    inherited::QUEUE_T* queue_p = task_p->msg_queue ();
//    if (!queue_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("no head module reader task queue found, aborting\n")));
//      return false;
//    } // end IF
//    queue_p->notification_strategy (configuration_in.notificationStrategy);
//  } // end IF
//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------

  // ******************* Runtime Statistic ************************
  //runtimeStatistic_.initialize (*configuration_in.moduleConfiguration);
  //Test_U_RIFFDecoder_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
  //    dynamic_cast<Test_U_RIFFDecoder_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  //if (!runtimeStatistic_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Test_U_RIFFDecoder_Module_RuntimeStatistic> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval, // reporting interval (seconds)
  //                                          true,                                        // push 1-second interval statistic messages downstream ?
  //                                          configuration_in.printFinalReport,           // print final report ?
  //                                          configuration_in.messageAllocator))          // message allocator handle
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              runtimeStatistic_.name ()));
  //  return false;
  //} // end IF

  // ******************* Decoder ************************
  //decoder_.initialize (*configuration_in.moduleConfiguration);
  //Test_U_RIFFDecoder_Module_Decoder* decoder_impl_p =
  //  dynamic_cast<Test_U_RIFFDecoder_Module_Decoder*> (decoder_.writer ());
  //if (!decoder_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<Test_U_RIFFDecoder_Module_Decoder> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!decoder_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              decoder_.name ()));
  //  return false;
  //} // end IF

  // ******************* File Source ************************
  source_impl_p =
    dynamic_cast<Test_U_RIFFDecoder_Module_Source*> (source_.writer ());
  if (!source_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_U_RIFFDecoder_Module_CamSource> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
//  // *TODO*: remove type inference
//  if (!source_impl_p->initialize (*configuration_in.moduleHandlerConfiguration,
//                                  configuration_in.messageAllocator))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
//                source_.name ()));
//    return false;
//  } // end IF
  source_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  source_.arg (inherited::sessionData_);

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;

  return false;
}
