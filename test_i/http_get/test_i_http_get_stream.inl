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

#include "stream_macros.h"

template <typename ConnectorType>
Test_I_HTTPGet_Stream_T<ConnectorType>::Test_I_HTTPGet_Stream_T ()
 : inherited ()
 , HTTPMarshal_ (this,
                 ACE_TEXT_ALWAYS_CHAR ("HTTPMarshal"))
 //, statisticReport_ (this,
 //                    ACE_TEXT_ALWAYS_CHAR ("StatisticReport"))
 , HTMLParser_ (this,
                ACE_TEXT_ALWAYS_CHAR ("HTMLParser"))
//, HTMLWriter_ (ACE_TEXT_ALWAYS_CHAR ("HTMLWriter"))
 , netSource_ (this,
               ACE_TEXT_ALWAYS_CHAR ("NetSource"))
 , HTTPGet_ (this,
             ACE_TEXT_ALWAYS_CHAR ("HTTPGet"))
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::Test_I_HTTPGet_Stream_T"));

}

template <typename ConnectorType>
Test_I_HTTPGet_Stream_T<ConnectorType>::~Test_I_HTTPGet_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::~Test_I_HTTPGet_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::load (Stream_ILayout* layout_in,
                                              bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::load"));

  // initialize return value(s)
  delete_out = false;

  layout_in->append (&HTTPMarshal_, NULL, 0);
  //layout_in->append (&statisticReport_, NULL, 0);
  layout_in->append (&HTMLParser_, NULL, 0);
  layout_in->append (&netSource_, NULL, 0);
  layout_in->append (&HTTPGet_, NULL, 0);

  return true;
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::initialize (const Test_I_StreamConfiguration_t& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_Stream_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<Test_I_StreamConfiguration_t&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  Test_I_HTTPParser* HTTPParser_impl_p = NULL;
  Test_I_SessionManager_t* session_manager_p =
    Test_I_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<Test_I_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<Test_I_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<struct Test_I_Stream_SessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;
//  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------
  // *TODO*: remove type inferences
//  ACE_ASSERT (configuration_in.moduleConfiguration);

  // ---------------------------------------------------------------------------
  // ******************* HTTP Marshal ************************
  HTTPParser_impl_p =
    dynamic_cast<Test_I_HTTPParser*> (HTTPMarshal_.writer ());
  if (!HTTPParser_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_I_HTTPParser> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<Test_I_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}

template <typename ConnectorType>
bool
Test_I_HTTPGet_Stream_T<ConnectorType>::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_HTTPGet_Stream_T::collect"));

  Test_I_SessionManager_t* session_manager_p =
    Test_I_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (session_manager_p);

  int result = -1;
  struct Test_I_Stream_SessionData& session_data_r =
    const_cast<struct Test_I_Stream_SessionData&> (session_manager_p->getR (inherited::id_));

  //Test_I_Statistic_WriterTask_t* statistic_impl =
  //  dynamic_cast<Test_I_Statistic_WriterTask_t*> (statisticReport_.writer ());
  //if (!statistic_impl)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: dynamic_cast<Test_I_Statistic_WriterTask_t> failed, aborting\n"),
  //              ACE_TEXT (stream_name_string_)));
  //  return false;
  //} // end IF

  // synch access
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

  // delegate to the statistics module...
  bool result_2 = false;
  try {
    //result_2 = statistic_impl->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (stream_name_string_)));
  }
  if (!result_2)
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
