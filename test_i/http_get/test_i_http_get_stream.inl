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

#include <ace/Log_Msg.h>

#include "stream_macros.h"

template <typename ConnectorType>
Test_I_HTTPGet_Stream_T<ConnectorType>::Test_I_HTTPGet_Stream_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR ("SourceStream"))
 , HTTPMarshal_ (ACE_TEXT_ALWAYS_CHAR ("HTTPMarshal"),
                 NULL,
                 false)
 , statisticReport_ (ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                     NULL,
                     false)
 , HTMLParser_ (ACE_TEXT_ALWAYS_CHAR ("HTMLParser"),
                NULL,
                false)
//, HTMLWriter_ (ACE_TEXT_ALWAYS_CHAR ("HTMLWriter"),
//               NULL,
//               false)
 , netSource_ (ACE_TEXT_ALWAYS_CHAR ("NetSource"),
               NULL,
               false)
 , HTTPGet_ (ACE_TEXT_ALWAYS_CHAR ("HTTPGet"),
             NULL,
             false)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::Test_I_HTTPGet_Stream_T"));

}

template <typename ConnectorType>
Test_I_HTTPGet_Stream_T<ConnectorType>::~Test_I_HTTPGet_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::~Test_I_HTTPGet_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::load (Stream_ModuleList_t& modules_out,
                                              bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::load"));

  // initialize return value(s)
  modules_out.clear ();
  delete_out = false;

  modules_out.push_back (&HTTPGet_);
  modules_out.push_back (&netSource_);
  //modules_out.push_back (&HTMLWriter_);
  modules_out.push_back (&HTMLParser_);
  modules_out.push_back (&statisticReport_);
  modules_out.push_back (&HTTPMarshal_);

  return true;
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::initialize (const Test_I_StreamConfiguration& configuration_in,
                                                    bool setupPipeline_in,
                                                    bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

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
  Test_I_Stream_SessionData& session_data_r =
      const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);
  session_data_r.targetFileName =
      configuration_in.moduleHandlerConfiguration->targetFileName;
//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.moduleConfiguration);
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

  // ---------------------------------------------------------------------------

  Test_I_HTTPParser* HTTPParser_impl_p = NULL;

  // ******************* HTTP Marshal ************************
  //HTTPMarshal_.initialize (*configuration_in.moduleConfiguration);
  HTTPParser_impl_p =
    dynamic_cast<Test_I_HTTPParser*> (HTTPMarshal_.writer ());
  if (!HTTPParser_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_HTTPParser*> failed, aborting\n")));
    goto failed;
  } // end IF
  // *TODO*: remove type inferences
  //if (!HTTPParser_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              HTTPMarshal_.name ()));
  //  goto failed;
  //} // end IF
  if (!HTTPParser_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                HTTPMarshal_.name ()));
    goto failed;
  } // end IF

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  HTTPMarshal_.arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  // OK: all went well
  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

failed:
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::reset(): \"%m\", continuing\n")));

  return false;
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::collect (Test_I_RuntimeStatistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Stream_SessionData& session_data_r =
      const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());

  Test_I_Statistic_WriterTask_t* statistic_impl =
    dynamic_cast<Test_I_Statistic_WriterTask_t*> (statisticReport_.writer ());
  if (!statistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Statistic_WriterTask_t*> failed, aborting\n")));
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

template <typename ConnectorType>
void
Test_I_HTTPGet_Stream_T<ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::report"));

//   Test_I_Stream_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<Test_I_Stream_Statistic_WriterTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Test_I_Stream_Statistic_WriterTask_t*> failed, returning\n")));
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

template <typename ConnectorType>
void
Test_I_HTTPGet_Stream_T<ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
