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

#include "stream_file_defines.h"

#include "stream_net_defines.h"
#include "stream_net_http_defines.h"

#include "stream_stat_defines.h"

#include "http_tools.h"

template <typename TCPConnectorType,
          typename SSLConnectorType>
HTTPGet_Stream_T<TCPConnectorType,
                 SSLConnectorType>::HTTPGet_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::HTTPGet_Stream_T"));

}

template <typename TCPConnectorType,
          typename SSLConnectorType>
bool
HTTPGet_Stream_T<TCPConnectorType,
                 SSLConnectorType>::load (Stream_ILayout* layout_inout,
                                          bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  HTTPGet_HTTPMarshal_Module (this,
                                              ACE_TEXT_ALWAYS_CHAR ("Marshal")),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  HTTPGet_StatisticReport_Module (this,
//                                                  ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
//                  false);
//  ACE_ASSERT (module_p);
//  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  TCP_SOURCE_MODULE_T (this,
                                       ACE_TEXT_ALWAYS_CHAR (MODULE_NET_SOURCE_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  HTTPGet_HTTPGet_Module (this,
                                          ACE_TEXT_ALWAYS_CHAR (MODULE_NET_HTTP_GET_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  HTTPGet_FileWriter_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

template <typename TCPConnectorType,
          typename SSLConnectorType>
bool
HTTPGet_Stream_T<TCPConnectorType,
                 SSLConnectorType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Stream_T::initialize"));

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  struct HTTPGet_SessionData* session_data_p = NULL;
  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  HTTPGet_SessionManager_t* session_manager_p =
    HTTPGet_SessionManager_t::SINGLETON_T::instance ();

  // sanity check(s)
  ACE_ASSERT (iterator != configuration_in.end ());
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  session_data_p =
    &const_cast<struct HTTPGet_SessionData&> (session_manager_p->getR ());
  // *TODO*: remove type inferences
  session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  if (HTTP_Tools::URLRequiresSSL ((*iterator).second.second->URL))
  {
    Stream_Module_t* module_p = NULL;
    ACE_NEW_RETURN (module_p,
                    SSL_SOURCE_MODULE_T (this,
                                         ACE_TEXT_ALWAYS_CHAR (MODULE_NET_SOURCE_DEFAULT_NAME_STRING)),
                    false);
    ACE_ASSERT (module_p);
    SSL_SOURCE_MODULE_T* module_2 =
        dynamic_cast<SSL_SOURCE_MODULE_T*> (module_p);
    ACE_ASSERT (module_2);
    module_2->initialize (*(*iterator).second.first);
    module_2->reset ();
    SSL_SOURCE_WRITER_T* task_p =
        dynamic_cast<SSL_SOURCE_WRITER_T*> (module_2->writer ());
    ACE_ASSERT (task_p);
//    Common_IInitialize_T<struct HTTPGet_ModuleHandlerConfiguration>* iinitialize_p =
//        dynamic_cast<Common_IInitialize_T<struct HTTPGet_ModuleHandlerConfiguration>*> (module_p->writer ());
//    ACE_ASSERT (iinitialize_p);
//    if (!iinitialize_p->initialize (*(*iterator).second.second))
    if (!task_p->initialize (*(*iterator).second.second))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Stream::replace(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (MODULE_NET_SOURCE_DEFAULT_NAME_STRING)));
      delete module_p; module_p = NULL;
      goto failed;
    } // end IF
    int result = inherited::replace (ACE_TEXT_ALWAYS_CHAR (MODULE_NET_SOURCE_DEFAULT_NAME_STRING),
                                     module_p,
                                     M_DELETE);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Stream::replace(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (stream_name_string_),
                  ACE_TEXT (MODULE_NET_SOURCE_DEFAULT_NAME_STRING)));
      delete module_p; module_p = NULL;
      goto failed;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: HTTPS URL; replaced module \"%s\", continuing\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (MODULE_NET_SOURCE_DEFAULT_NAME_STRING)));
  } // end IF

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}
