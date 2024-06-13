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

#include <iostream>

#include "ace/Date_Time.h"

#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"

#include "stream_macros.h"

#include "stream_html_common.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Module_XPathQuery_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::Stream_Module_XPathQuery_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_XPathQuery_T::Stream_Module_XPathQuery_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Module_XPathQuery_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_XPathQuery_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (inherited::configuration_->xPathQueryString.empty ())
    return;
  const typename DataMessageType::DATA_T& data_container_r =
    message_inout->getR ();
  typename DataMessageType::DATA_T::DATA_T& data_r =
    const_cast<typename DataMessageType::DATA_T::DATA_T&> (data_container_r.getR ());
  if (!data_r.document)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no document, cannot proceed, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // step1: create a query context
  xmlXPathContextPtr xpath_context_p = xmlXPathNewContext (data_r.document);
  if (!xpath_context_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to xmlXPathNewContext(); \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // step2: register given namespaces
  int result = -1;
  for (Stream_HTML_XPathNameSpacesConstIterator_t iterator = inherited::configuration_->xPathNameSpaces.begin ();
       iterator != inherited::configuration_->xPathNameSpaces.end ();
       ++iterator)
  {
    result = xmlXPathRegisterNs (xpath_context_p,
                                 BAD_CAST ((*iterator).first.c_str ()),
                                 BAD_CAST ((*iterator).second.c_str ()));
    if (unlikely (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to xmlXPathRegisterNs(\"%s\" --> \"%s\"), continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT ((*iterator).first.c_str ()),
                  ACE_TEXT ((*iterator).second.c_str ())));
  } // end FOR

  // step3: perform query
  ACE_ASSERT (!data_r.xPathObject);
  data_r.xPathObject = 
    xmlXPathEvalExpression (BAD_CAST (inherited::configuration_->xPathQueryString.c_str ()),
                            xpath_context_p);
  if (!data_r.xPathObject)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to xmlXPathEvalExpression(\"%s\"); \"%m\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->xPathQueryString.c_str ())));
    xmlXPathFreeContext (xpath_context_p); xpath_context_p = NULL;
    return;
  } // end IF

  // step4: clean up
  xmlXPathFreeContext (xpath_context_p); xpath_context_p = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Module_XPathQuery_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_XPathQuery_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (message_inout);

  // *TODO*: remove type inferences
//  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
//    inherited::sessionData_->getR ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_STEP:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}
