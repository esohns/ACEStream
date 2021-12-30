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

#include "test_i_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "test_i_modules.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_DirectShow_Stream::Test_I_DirectShow_Stream ()
 : inherited ()
 , statistic_ (this,
               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::Test_I_DirectShow_Stream"));

}

bool
Test_I_DirectShow_Stream::load (Stream_ILayout* layout_inout,
                                bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::load"));

  // initialize return value(s)
  deleteModules_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_->dispatchConfiguration);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  layout_inout->append (&statistic_, NULL, 0);

  return true;
}

bool
Test_I_DirectShow_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);

  inherited::isInitialized_ = true;

  return true;
}

bool
Test_I_DirectShow_Stream::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::collect"));

  STATISTIC_WRITER_T* statistic_report_impl_p =
    dynamic_cast<STATISTIC_WRITER_T*> (statistic_.writer ());
  if (!statistic_report_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // delegate to this module
  return statistic_report_impl_p->collect (data_out);
}

void
Test_I_DirectShow_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Stream::report"));

//   RPG_Net_Module_RuntimeStatistic* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<RPG_Net_Module_RuntimeStatistic*> (//                                            myRuntimeStatistic.writer());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic) failed> (aborting\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module
//   return (runtimeStatistic_impl->report());

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

//////////////////////////////////////////

Test_I_MediaFoundation_Stream::Test_I_MediaFoundation_Stream ()
 : inherited ()
 , statistic_ (this,
               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::Test_I_MediaFoundation_Stream"));

}

bool
Test_I_MediaFoundation_Stream::load (Stream_ILayout* layout_inout,
                                bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::load"));

  // initialize return value(s)
  deleteModules_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_->dispatchConfiguration);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  layout_inout->append (&statistic_, NULL, 0);

  return true;
}

bool
Test_I_MediaFoundation_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);

  inherited::isInitialized_ = true;

  return true;
}

bool
Test_I_MediaFoundation_Stream::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::collect"));

  STATISTIC_WRITER_T* statistic_report_impl_p =
    dynamic_cast<STATISTIC_WRITER_T*> (statistic_.writer ());
  if (!statistic_report_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // delegate to this module
  return statistic_report_impl_p->collect (data_out);
}

void
Test_I_MediaFoundation_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Stream::report"));

//   RPG_Net_Module_RuntimeStatistic* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<RPG_Net_Module_RuntimeStatistic*> (//                                            myRuntimeStatistic.writer());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic) failed> (aborting\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module
//   return (runtimeStatistic_impl->report());

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
#else
Test_I_ALSA_Stream::Test_I_ALSA_Stream ()
  : inherited ()
  , statistic_ (this,
                ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::Test_I_ALSA_Stream"));

}

bool
Test_I_ALSA_Stream::load (Stream_ILayout* layout_inout,
                                     bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::load"));

  // initialize return value(s)
  deleteModules_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_->dispatchConfiguration);
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());

  layout_inout->append (&statistic_, NULL, 0);

  return true;
}

bool
Test_I_ALSA_Stream::initialize (const CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!inherited::isRunning ());
  ACE_ASSERT (configuration_in.configuration_);

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::sessionData_);

  inherited::isInitialized_ = true;

  return true;
}

bool
Test_I_ALSA_Stream::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::collect"));

  STATISTIC_WRITER_T* statistic_report_impl_p =
    dynamic_cast<STATISTIC_WRITER_T*> (statistic_.writer ());
  if (!statistic_report_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Statistic_StatisticReport_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

  // delegate to this module
  return statistic_report_impl_p->collect (data_out);
}

void
Test_I_ALSA_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ALSA_Stream::report"));

  //   RPG_Net_Module_RuntimeStatistic* runtimeStatistic_impl = NULL;
  //   runtimeStatistic_impl = dynamic_cast<RPG_Net_Module_RuntimeStatistic*> (//                                            myRuntimeStatistic.writer());
  //   if (!runtimeStatistic_impl)
  //   {
  //     ACE_DEBUG((LM_ERROR,
  //                ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic) failed> (aborting\n")));
  //
  //     return;
  //   } // end IF
  //
  //   // delegate to this module
  //   return (runtimeStatistic_impl->report());

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
#endif // ACE_WIN32 || ACE_WIN64
