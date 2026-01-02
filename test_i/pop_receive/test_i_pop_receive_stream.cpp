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

#include "test_i_pop_receive_stream.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_net_defines.h"

#include "stream_stat_defines.h"

#include "pop_defines.h"

Test_I_POPReceive_Stream::Test_I_POPReceive_Stream ()
 : inherited ()
 , source_ (this,
            ACE_TEXT_ALWAYS_CHAR (MODULE_NET_SOURCE_DEFAULT_NAME_STRING))
 , asynchSource_ (this,
                  ACE_TEXT_ALWAYS_CHAR (MODULE_NET_SOURCE_DEFAULT_NAME_STRING))
#if defined (SSL_SUPPORT)
 , SSLSource_ (this,
               ACE_TEXT_ALWAYS_CHAR (MODULE_NET_SOURCE_DEFAULT_NAME_STRING))
#endif // SSL_SUPPORT
 , marshal_ (this,
             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MARSHAL_DEFAULT_NAME_STRING))
 //, parser_ (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_PARSER_DEFAULT_NAME_STRING),
 //           NULL,
 //           false)
 , statistic_ (this,
               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING))
 , protocolHandler_ (this,
                     ACE_TEXT_ALWAYS_CHAR (POP_DEFAULT_MODULE_SEND_NAME_STRING))
{
  STREAM_TRACE (ACE_TEXT ("Test_I_POPReceive_Stream::Test_I_POPReceive_Stream"));

}

bool
Test_I_POPReceive_Stream::load (Stream_ILayout* layout_inout,
                              bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_POPReceive_Stream::load"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_);
  ACE_ASSERT (inherited::configuration_->configuration_->dispatchConfiguration);

  deleteModules_out = false;

  // update configuration
  inherited::CONFIGURATION_T::ITERATOR_T iterator =
    inherited::configuration_->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != inherited::configuration_->end ());
  ACE_ASSERT ((*iterator).second.second->protocolConfiguration);
  ACE_ASSERT (!(*iterator).second.second->protocolConfiguration->parser);
  (*iterator).second.second->protocolConfiguration->parser =
    dynamic_cast<POP_IParser*> (marshal_.writer ());
  ACE_ASSERT ((*iterator).second.second->protocolConfiguration->parser);

  Net_ConnectionConfigurationsIterator_t iterator_2 =
    (*iterator).second.second->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != (*iterator).second.second->connectionConfigurations->end ());
  bool use_SSL_b =
    (NET_CONFIGURATION_TCP_CAST ((*iterator_2).second)->socketConfiguration.address.get_port_number () == POP_DEFAULT_TLS_SERVER_PORT);
  if (!use_SSL_b &&
      (NET_CONFIGURATION_TCP_CAST ((*iterator_2).second)->socketConfiguration.address.get_port_number () != POP_DEFAULT_SERVER_PORT))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("non-standard server port (was: %u), trying SSL connection\n"),
                NET_CONFIGURATION_TCP_CAST ((*iterator_2).second)->socketConfiguration.address.get_port_number ()));
    use_SSL_b = true;
  } // end IF
  if (inherited::configuration_->configuration_->dispatchConfiguration->dispatch == COMMON_EVENT_DISPATCH_REACTOR)
  {
    if (use_SSL_b)
      layout_inout->append (&SSLSource_, NULL, 0);
    else
      layout_inout->append (&source_, NULL, 0);
  } // end IF
  else
  {
    if (use_SSL_b)
    {
      ACE_ASSERT (false); // *TODO*
    } // end IF
    else
      layout_inout->append (&asynchSource_, NULL, 0);
  } // end ELSE
  layout_inout->append (&marshal_, NULL, 0);
  layout_inout->append (&statistic_, NULL, 0);
  layout_inout->append (&protocolHandler_, NULL, 0);

  return true;
}

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_I_POPReceive_Stream::initialize (const CONFIGURATION_T& configuration_in)
#else
Test_I_POPReceive_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Test_I_POPReceive_Stream::initialize"));

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

  inherited::isInitialized_ = true;

  return true;
}

bool
Test_I_POPReceive_Stream::collect (POP_Statistic_t& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_POPReceive_Stream::collect"));

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
Test_I_POPReceive_Stream::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_POPReceive_Stream::report"));

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
