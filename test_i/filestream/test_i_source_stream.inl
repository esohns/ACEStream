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

#include "stream_stat_defines.h"

#include "test_i_common_modules.h"

template <typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_Stream_T<ConnectionManagerType,
                       ConnectorType>::Test_I_Source_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::Test_I_Source_Stream_T"));

}

template <typename ConnectionManagerType,
          typename ConnectorType>
Test_I_Source_Stream_T<ConnectionManagerType,
                       ConnectorType>::~Test_I_Source_Stream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::~Test_I_Source_Stream_T"));

  // *NOTE*: implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectionManagerType,
                       ConnectorType>::load (Stream_ILayout* layout_in,
                                             bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::load"));

//  // initialize return value(s)
//  modules_out.clear ();
//  delete_out = false;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_FileReader_Module (this,
                                            ACE_TEXT_ALWAYS_CHAR ("FileReader")),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_Source_StatisticReport_Module (this,
  //                                                      ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TARGET_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR ("NetTarget")),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       ConnectorType>::initialize (const CONFIGURATION_T& configuration_in)
#else
                       ConnectorType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);


  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_Source_SessionData* session_data_p = NULL;
  Test_I_Source_StreamConfiguration_t::CONST_ITERATOR_T iterator =
    configuration_in.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  Test_I_SessionManager_t* session_manager_p =
    Test_I_SessionManager_t::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<Test_I_Source_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<Test_I_Source_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      return false;
    } // end IF

  // -------------------------------------------------------------

  session_data_p =
    &const_cast<struct Test_I_Source_SessionData&> (session_manager_p->getR (inherited::id_));
  // *TODO*: remove type inferences
  session_data_p->sourceFileName =
    (*iterator).second.second->fileIdentifier.identifier;
  session_data_p->size =
    Common_File_Tools::size ((*iterator).second.second->fileIdentifier.identifier);

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<Test_I_Source_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}

template <typename ConnectionManagerType,
          typename ConnectorType>
bool
Test_I_Source_Stream_T<ConnectionManagerType,
                       ConnectorType>::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_Stream_T::collect"));

  // sanity check(s)
  Test_I_SessionManager_t* session_manager_p =
    Test_I_SessionManager_t::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  int result = -1;
  struct Test_I_Source_SessionData& session_data_r =
    const_cast<struct Test_I_Source_SessionData&> (session_manager_p->getR (inherited::id_));

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)));
    return false;
  } // end IF
  Test_I_Source_Module_Statistic_WriterTask_t* statistic_report_impl_p =
    static_cast<Test_I_Source_Module_Statistic_WriterTask_t*> (module_p->writer ());
  ACE_ASSERT (statistic_report_impl_p);

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

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_report_impl_p->collect (data_out);
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
