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

#include <ace/Date_Time.h>

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "stream_macros.h"

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
                           SessionDataType>::Stream_Module_XPathQuery_T ()
 : inherited ()
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
Stream_Module_XPathQuery_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType>::~Stream_Module_XPathQuery_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_XPathQuery_T::~Stream_Module_XPathQuery_T"));

}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType>
//void
//Stream_Module_XPathQuery_T<SessionMessageType,
//                           MessageType,
//                           ModuleHandlerConfigurationType,
//                           SessionDataType>::handleDataMessage (MessageType*& message_inout,
//                                                                bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_XPathQuery_T::handleDataMessage"));

//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//  ACE_UNUSED_ARG (message_inout);
//}

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

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // *TODO*: remove type inferences
  const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
      message_inout->get ();
  const SessionDataType& session_data_r =
      session_data_container_r.get ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_END:
    {
      if (!session_data_r.parserContext)
        return;

      // sanity check(s)
      ACE_ASSERT (session_data_r.parserContext);
      ACE_ASSERT (session_data_r.parserContext->parserContext);

      if (session_data_r.parserContext->parserContext->myDoc)
      {
        result =
            htmlParseChunk (session_data_r.parserContext->parserContext, // context
                            ACE_TEXT_ALWAYS_CHAR (""),                   // chunk
                            0,                                           // size
                            1);                                          // terminate ?
        if (result)
        {
          xmlParserErrors error = static_cast<xmlParserErrors> (result);
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("failed to htmlParseChunk() (error was: %d), continuing\n"),
                      error));
        } // end IF

        if (!session_data_r.parserContext->parserContext->wellFormed)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("HTML document not well formed, continuing\n")));
        if (session_data_r.parserContext->parserContext->errNo)
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("HTML document had errors ((last) error was: %d), continuing\n"),
                      session_data_r.parserContext->parserContext->errNo));

        // extract headlines from HTML
//        xmlXPathContextPtr xpath_context_p = xmlXPathNewContext (document_);
        xmlXPathContextPtr xpath_context_p =
            xmlXPathNewContext (session_data_r.parserContext->parserContext->myDoc);
        if (!xpath_context_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to xmlXPathNewContext(); \"%m\", returning\n")));
          return;
        } // end IF
        xmlChar* query_string_p =
            BAD_CAST (ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_XPATHQUERY_QUERY_STRING));
        xmlXPathObjectPtr xpath_object_p =
            xmlXPathEvalExpression (query_string_p,
                                    xpath_context_p);
        if (!xpath_object_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to xmlXPathEvalExpression(\"%s\"); \"%m\", returning\n"),
                      ACE_TEXT (query_string_p)));

          // clean up
          xmlXPathFreeContext (xpath_context_p);

          return;
        } // end IF
        ACE_ASSERT (xpath_object_p->nodesetval);

        for (int i = 0;
             i < xpath_object_p->nodesetval->nodeNr;
             i++)
        {
          ACE_ASSERT (xpath_object_p->nodesetval->nodeTab);
          ACE_ASSERT (xpath_object_p->nodesetval->nodeTab[i]);
          ACE_ASSERT (xpath_object_p->nodesetval->nodeTab[i]->children);
          std::cout << ACE_TEXT_ALWAYS_CHAR (xpath_object_p->nodesetval->nodeTab[i]->children[0].content)
                    << std::endl;
        } // end FOR

        // clean up
        xmlXPathFreeObject (xpath_object_p);
        xmlXPathFreeContext (xpath_context_p);
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType>
//const ModuleHandlerConfigurationType&
//Stream_Module_XPathQuery_T<SessionMessageType,
//                           MessageType,
//                           ModuleHandlerConfigurationType,
//                           SessionDataType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_XPathQuery_T::get"));
//
//  return configuration_;
//}
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataContainerType,
//          typename SessionDataType>
//bool
//Stream_Module_XPathQuery_T<SessionMessageType,
//                           MessageType,
//                           ModuleHandlerConfigurationType,
//                           SessionDataType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_XPathQuery_T::initialize"));
//
//  configuration_ = configuration_in;
//
//  return true;
//}
