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
HTTPGet_Stream_T<ConnectorType>::HTTPGet_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::HTTPGet_Stream_T"));

}

template <typename ConnectorType>
HTTPGet_Stream_T<ConnectorType>::~HTTPGet_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::~HTTPGet_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename ConnectorType>
bool
HTTPGet_Stream_T<ConnectorType>::load (Stream_ModuleList_t& modules_out,
                                       bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  HTTPGet_FileWriter_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                                             NULL,
                                             false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  HTTPGet_HTTPGet_Module (this,
                                          ACE_TEXT_ALWAYS_CHAR ("HTTPGet"),
                                          NULL,
                                          false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  SOURCE_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR ("NetSource"),
                                   NULL,
                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  HTTPGet_StatisticReport_Module (this,
                                                  ACE_TEXT_ALWAYS_CHAR ("StatisticReport"),
                                                  NULL,
                                                  false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  HTTPGet_HTTPMarshal_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                              NULL,
                                              false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

template <typename ConnectorType>
bool
HTTPGet_Stream_T<ConnectorType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

  HTTPGet_StreamConfiguration_t::ITERATOR_T iterator;
  struct HTTPGet_SessionData* session_data_p = NULL;
  struct HTTPGet_ModuleHandlerConfiguration* configuration_p = NULL;
  Stream_Module_t* module_p = NULL;
  HTTPGet_HTTPParser* HTTPParser_impl_p = NULL;

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ())));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
      &const_cast<struct HTTPGet_SessionData&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  configuration_p =
    dynamic_cast<struct HTTPGet_ModuleHandlerConfiguration*> (&(*iterator).second);
  ACE_ASSERT (configuration_p);
  session_data_p->targetFileName = configuration_p->targetFileName;

  // ---------------------------------------------------------------------------

  // ******************* HTTP Marshal ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("Marshal")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ()),
                ACE_TEXT ("Marshal")));
    goto failed;
  } // end IF

  HTTPParser_impl_p =
    dynamic_cast<HTTPGet_HTTPParser*> (module_p->writer ());
  if (!HTTPParser_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<HTTPGet_HTTPParser> failed, aborting\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ())));
    goto failed;
  } // end IF
  HTTPParser_impl_p->set (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (inherited::configuration_.name_.c_str ())));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ())));

  return false;
}

template <typename ConnectorType>
bool
HTTPGet_Stream_T<ConnectorType>::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  struct HTTPGet_SessionData& session_data_r =
      const_cast<struct HTTPGet_SessionData&> (inherited::sessionData_->get ());
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("StatisticReport")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ()),
                ACE_TEXT ("StatisticReport")));
    return false;
  } // end IF
  HTTPGet_StatisticReport_WriterTask_t* statistic_impl_p =
    dynamic_cast<HTTPGet_StatisticReport_WriterTask_t*> (module_p->writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Module_StatisticReport_WriterTask_T> failed, aborting\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ())));
    return false;
  } // end IF

  // synch access
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n"),
                  ACE_TEXT (inherited::configuration_.name_.c_str ())));
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
                ACE_TEXT ("%s: caught exception in Common_IStatistic_T::collect(), continuing\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ())));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IStatistic_T::collect(), aborting\n"),
                ACE_TEXT (inherited::configuration_.name_.c_str ())));
  else
    session_data_r.currentStatistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  ACE_TEXT (inherited::configuration_.name_.c_str ())));
  } // end IF

  return result_2;
}

template <typename ConnectorType>
void
HTTPGet_Stream_T<ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::report"));

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
HTTPGet_Stream_T<ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
