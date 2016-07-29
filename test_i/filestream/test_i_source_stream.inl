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

#include "test_i_common_modules.h"

template <typename ConnectorType>
Test_I_Source_Stream_T<ConnectorType>::Test_I_Source_Stream_T (const std::string& name_in)
 : inherited (name_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::Test_I_Source_Stream_T"));

}

template <typename ConnectorType>
Test_I_Source_Stream_T<ConnectorType>::~Test_I_Source_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::~Test_I_Source_Stream_T"));

  // *NOTE*: implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename ConnectorType>
void
Test_I_Source_Stream_T<ConnectorType>::ping ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::ping"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectorType>::load (Stream_ModuleList_t& modules_out,
                                             bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::load"));

  // initialize return value(s)
  modules_out.clear ();
  delete_out = false;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (ACE_TEXT_ALWAYS_CHAR ("NetTarget"),
                                   NULL,
                                   false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Source_Module_RuntimeStatistic_Module (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                                                                NULL,
                                                                false),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Module_FileReader_Module (ACE_TEXT_ALWAYS_CHAR ("FileReader"),
                                                   NULL,
                                                   false),
                  false);
  modules_out.push_back (module_p);

  delete_out = true;

  return true;
}

template <typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectorType>::initialize (const Test_I_Source_Stream_Configuration& configuration_in,
                                                   bool setupPipeline_in,
                                                   bool resetSessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

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

  // things to be done here:
  // [- initialize base class]
  // ------------------------------------
  // - initialize modules
  // - push them onto the stream (tail-first) !
  // ------------------------------------

  //  configuration_in.moduleConfiguration.streamState = &state_;

  // ---------------------------------------------------------------------------
  ACE_ASSERT (configuration_in.moduleConfiguration);
  ACE_ASSERT (configuration_in.moduleHandlerConfiguration);

  // ---------------------------------------------------------------------------

  Test_I_Module_FileReader* fileReader_impl_p = NULL;
  Test_I_Stream_SessionData* session_data_p = NULL;

  // ******************* File Reader ************************
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("FileReader")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("FileReader")));
    goto failed;
  } // end IF
  //fileReader_.initialize (*configuration_in.moduleConfiguration);
  fileReader_impl_p =
    dynamic_cast<Test_I_Module_FileReader*> (module_p->writer ());
  if (!fileReader_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Module_FileReader> failed, aborting\n")));
    goto failed;
  } // end IF
  //if (!fileReader_impl_p->initialize (*configuration_in.moduleHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              module_p->name ()));
  //  goto failed;
  //} // end IF
  if (!fileReader_impl_p->initialize (inherited::state_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                module_p->name ()));
    goto failed;
  } // end IF
  //fileReader_impl_p->reset ();
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (setupPipeline_in)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      return false;
    } // end IF

  // -------------------------------------------------------------

  // *TODO*: remove type inferences
  session_data_p =
      &const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());
  session_data_p->fileName =
    configuration_in.moduleHandlerConfiguration->fileName;
  session_data_p->size =
    Common_File_Tools::size (configuration_in.moduleHandlerConfiguration->fileName);

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
Test_I_Source_Stream_T<ConnectorType>::collect (Test_I_RuntimeStatistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Stream_SessionData& session_data_r =
      const_cast<Test_I_Stream_SessionData&> (inherited::sessionData_->get ());

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("RuntimeStatistic")));
    return false;
  } // end IF
  Test_I_Source_Module_Statistic_WriterTask_t* runtimeStatistic_impl =
    dynamic_cast<Test_I_Source_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Source_Module_Statistic_WriterTask_t> failed, aborting\n")));
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

  // delegate to the statistics module
  bool result_2 = false;
  try {
    result_2 = runtimeStatistic_impl->collect (data_out);
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
Test_I_Source_Stream_T<ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
