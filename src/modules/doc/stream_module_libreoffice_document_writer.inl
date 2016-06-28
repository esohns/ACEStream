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

#include "cppuhelper/bootstrap.hxx"

#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/process.h"

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/frame/Desktop.hpp"
#include "com/sun/star/frame/XComponentLoader.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"

#include "stream_macros.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename DocumentType>
Stream_Module_LibreOffice_Document_Writer_T<SessionMessageType,
                                               MessageType,
                                               ModuleHandlerConfigurationType,
                                               SessionDataType,
                                               DocumentType>::Stream_Module_LibreOffice_Document_Writer_T ()
 : inherited ()
 , component_ ()
 , context_ ()
 , configuration_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::Stream_Module_LibreOffice_Document_Writer_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename DocumentType>
Stream_Module_LibreOffice_Document_Writer_T<SessionMessageType,
                                            MessageType,
                                            ModuleHandlerConfigurationType,
                                            SessionDataType,
                                            DocumentType>::~Stream_Module_LibreOffice_Document_Writer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::~Stream_Module_LibreOffice_Document_Writer_T"));

  // *TODO*: ::lang::XComponent::dispose crashes the application
  //if (component_.is ())
  //  component_->dispose ();
  //if (context_.is ())
  //  uno::Reference<lang::XComponent>::query (context_)->dispose ();
}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType>
//void
//Stream_Module_LibreOffice_Document_Writer_T<SessionMessageType,
//                            MessageType,
//                            ModuleHandlerConfigurationType,
//                            SessionDataType>::handleDataMessage (MessageType*& message_inout,
//                                                                 bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::handleDataMessage"));
//
//  ssize_t bytes_written = -1;
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  // sanity check(s)
//  if (!connection_)
//  {
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to open db connection, returning\n")));
//    return;
//  } // end IF
//}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename DocumentType>
void
Stream_Module_LibreOffice_Document_Writer_T<SessionMessageType,
                                            MessageType,
                                            ModuleHandlerConfigurationType,
                                            SessionDataType,
                                            DocumentType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (configuration_);

  const typename SessionMessageType::DATA_T& session_data_container_r =
      message_inout->get ();
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_->socketConfiguration);

      uno::Reference<uno::XInterface> interface_p;
      uno::Reference<lang::XMultiComponentFactory> multi_component_factory_p;
      std::string connection_string = ACE_TEXT_ALWAYS_CHAR ("uno:socket,host=");
      std::ostringstream converter;
      ::rtl::OUString connection_string_2;
      uno::Reference<bridge::XUnoUrlResolver> resolver_p;
      uno::Reference<frame::XDesktop2> component_loader_p;
      uno::Reference<lang::XMultiComponentFactory> multi_component_factory_2;
      uno::Reference<beans::XPropertySet> property_set_p;
      ::rtl::OUString filename, working_directory, filename_url, absolute_filename_url;
      oslProcessError result_2 = osl_Process_E_InvalidError;
      ::osl::FileBase::RC result_3 = ::osl::FileBase::RC::E_invalidError;
      bool result_4 = false;
      const char* result_p = NULL;

      // debug info
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result =
        configuration_->socketConfiguration->address.addr_to_string (buffer,
                                                                     sizeof (buffer));
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", aborting\n")));
        goto error;
      } // end IF

      ACE_TCHAR host_address[BUFSIZ];
      ACE_OS::memset (host_address, 0, sizeof (host_address));
      result_p =
        configuration_->socketConfiguration->address.get_host_addr (host_address,
                                                                    sizeof (host_address));
      if (!result_p || (result_p != host_address))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::get_host_addr(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (buffer)));
        goto error;
      } // end IF

      connection_string += ACE_TEXT_ALWAYS_CHAR (host_address);
      connection_string += ACE_TEXT_ALWAYS_CHAR (",port=");
      converter <<
        configuration_->socketConfiguration->address.get_port_number ();
      connection_string += converter.str ();
      connection_string += ACE_TEXT_ALWAYS_CHAR (";urp;StarOffice.ServiceManager");
      connection_string_2 =
        ::rtl::OUString::createFromAscii (ACE_TEXT_ALWAYS_CHAR (connection_string.c_str ()));

      result_4 =
        context_.set (::cppu::defaultBootstrap_InitialComponentContext ());
      ACE_ASSERT (result_4);
      result_4 = multi_component_factory_p.set (context_->getServiceManager ());
      ACE_ASSERT (result_4);
      result_4 =
        interface_p.set (multi_component_factory_p->createInstanceWithContext (ACE_TEXT_ALWAYS_CHAR ("com.sun.star.bridge.UnoUrlResolver"),
                                                                               context_));
      ACE_ASSERT (result_4);
      uno::Reference<lang::XComponent>::query (multi_component_factory_p)->dispose ();
      result_4 = resolver_p.set (interface_p,
                                 uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      try
      {
        result_4 = interface_p.set (resolver_p->resolve (connection_string_2),
                                    uno::UNO_QUERY);
      }
      catch (uno::Exception& exception_in)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to establish a connection (was: \"%s\"): \"%s\"\n"),
                    ACE_TEXT (::rtl::OUStringToOString (connection_string_2,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ()),
                    ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
        goto error;
      }
      ACE_ASSERT (result_4);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("opened LibreOffice connection (was: \"%s\")...\n"),
                  ACE_TEXT (::rtl::OUStringToOString (connection_string_2,
                                                      RTL_TEXTENCODING_ASCII_US,
                                                      OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));

      result_4 = property_set_p.set (interface_p,
                                     uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      property_set_p->getPropertyValue ("DefaultContext") >>= context_;
      //result_4 = server_p.set (component_context_p->getServiceManager ());
      //ACE_ASSERT (result_4);

      result_4 = component_loader_p.set (frame::Desktop::create (context_));
      ACE_ASSERT (result_4);

      // generate document filename URL
      filename =
        ::rtl::OUString::createFromAscii (configuration_->targetFileName.c_str ());
      result_2 = osl_getProcessWorkingDir (&working_directory.pData);
      ACE_ASSERT (result_2 == osl_Process_E_None);
      result_3 = ::osl::FileBase::getFileURLFromSystemPath (filename,
                                                            filename_url);
      ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);
      result_3 = ::osl::FileBase::getAbsoluteFileURL (working_directory,
                                                      filename_url,
                                                      absolute_filename_url);
      ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);
      result_4 =
        component_.set (component_loader_p->loadComponentFromURL (absolute_filename_url,
                                                                  ::rtl::OUString ("_blank"),
                                                                  0,
                                                                  uno::Sequence<beans::PropertyValue> ()));
      ACE_ASSERT (result_4);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("loaded LibreOffice document (was: \"%s\")...\n"),
                  ACE_TEXT (::rtl::OUStringToOString (absolute_filename_url,
                                                      RTL_TEXTENCODING_ASCII_US,
                                                      OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));

      break;

error:
      session_data_r.aborted = true;

      return;
    }
    case STREAM_SESSION_END:
    {
      // *TODO*: ::lang::XComponent::dispose crashes the application
      if (component_.is ())
        component_->dispose ();
      if (context_.is ())
        uno::Reference<lang::XComponent>::query (context_)->dispose ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename DocumentType>
bool
Stream_Module_LibreOffice_Document_Writer_T<SessionMessageType,
                                            MessageType,
                                            ModuleHandlerConfigurationType,
                                            SessionDataType,
                                            DocumentType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::initialize"));

  if (isInitialized_)
  {
    // *TODO*: ::lang::XComponent::dispose crashes the application
    if (component_.is ())
      component_->dispose ();
    if (context_.is ())
      uno::Reference<lang::XComponent>::query (context_)->dispose ();

    isInitialized_ = false;
  } // end IF

  configuration_ =
    &const_cast<ModuleHandlerConfigurationType&> (configuration_in);

  isInitialized_ = true;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename DocumentType>
const ModuleHandlerConfigurationType&
Stream_Module_LibreOffice_Document_Writer_T<SessionMessageType,
                                            MessageType,
                                            ModuleHandlerConfigurationType,
                                            SessionDataType,
                                            DocumentType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
