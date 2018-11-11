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

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/document/MacroExecMode.hpp"
#include "com/sun/star/frame/FrameSearchFlag.hpp"
#include "com/sun/star/frame/XComponentLoader.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"

#include "net_common_tools.h"
//#include "net_configuration.h"

#include "stream_macros.h"

#include "stream_document_defines.h"
#include "stream_module_libreoffice_document_handler.h"

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionConfigurationIteratorType,
          typename SessionDataType,
          typename DocumentType>
Stream_Module_LibreOffice_Document_Writer_T<SynchStrategyType,
                                            TimePolicyType,
                                            ConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            ConnectionConfigurationIteratorType,
                                            SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                            DocumentType>::Stream_Module_LibreOffice_Document_Writer_T (ISTREAM_T* stream_in)
#else
                                            DocumentType>::Stream_Module_LibreOffice_Document_Writer_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , component_ ()
 , componentContext_ ()
 , interactionHandler_ ()
 , releaseHandler_ (false)
 /////////////////////////////////////////
 , handler_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::Stream_Module_LibreOffice_Document_Writer_T"));

  // *TODO*: for some reason, this doesn't work
  //ACE_NEW_NORETURN (handler_,
  //                  Stream_Module_LibreOffice_Document_Handler ());
  handler_ = new Stream_Module_LibreOffice_Document_Handler ();
  if (!handler_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  handler_->acquire ();
  bool result = interactionHandler_.set (*handler_,
                                         uno::UNO_QUERY);
  ACE_ASSERT (interactionHandler_.is ());
  ACE_ASSERT (result);

  releaseHandler_ = true;
}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionConfigurationIteratorType,
          typename SessionDataType,
          typename DocumentType>
Stream_Module_LibreOffice_Document_Writer_T<SynchStrategyType,
                                            TimePolicyType,
                                            ConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            ConnectionConfigurationIteratorType,
                                            SessionDataType,
                                            DocumentType>::~Stream_Module_LibreOffice_Document_Writer_T () throw ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::~Stream_Module_LibreOffice_Document_Writer_T"));

  // *TODO*: ::lang::XComponent::dispose crashes the application
  if (component_.is ())
    component_->dispose ();
  if (componentContext_.is ())
  {
    uno::Reference<lang::XComponent> component_p =
        uno::Reference<lang::XComponent>::query (componentContext_);
    if (component_p.is ())
      component_p->dispose ();
    componentContext_.clear ();
  } // end IF
  interactionHandler_.clear ();

  if (releaseHandler_)
    handler_->release ();

  if (handler_)
    delete handler_;
}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConnectionConfigurationIteratorType,
//          typename SessionDataType>
//void
//Stream_Module_LibreOffice_Document_Writer_T<SessionMessageType,
//                            MessageType,
//                            ConnectionConfigurationIteratorType,
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

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionConfigurationIteratorType,
          typename SessionDataType,
          typename DocumentType>
void
Stream_Module_LibreOffice_Document_Writer_T<SynchStrategyType,
                                            TimePolicyType,
                                            ConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            ConnectionConfigurationIteratorType,
                                            SessionDataType,
                                            DocumentType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::handleSessionMessage"));

//  int result = -1;
  oslProcessError result_2 = osl_Process_E_InvalidError;
  enum ::osl::FileBase::RC result_3 = ::osl::FileBase::RC::E_invalidError;
  bool result_4 = false;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ::rtl::OUString filename, working_directory, filename_url;
  ::rtl::OUString absolute_filename_url;
  result_2 = osl_getProcessWorkingDir (&working_directory.pData);
  ACE_ASSERT (result_2 == osl_Process_E_None);
  uno::Sequence<beans::PropertyValue> document_properties;

//  const typename SessionMessageType::DATA_T& session_data_container_r =
//      message_inout->getR ();
//  SessionDataType& session_data_r =
//      const_cast<SessionDataType&> (session_data_container_r.getR ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      uno::Reference<uno::XInterface> interface_p;
      uno::Reference<lang::XMultiComponentFactory> multi_component_factory_p;
      std::string connection_string = ACE_TEXT_ALWAYS_CHAR ("uno:socket,host=");
      std::ostringstream converter;
      ::rtl::OUString connection_string_2;
      uno::Reference<bridge::XUnoUrlResolver> resolver_p;
      uno::Reference<frame::XComponentLoader> component_loader_p;
      uno::Reference<lang::XMultiComponentFactory> multi_component_factory_2;
      uno::Reference<beans::XPropertySet> property_set_p;

      // --> create new frame (see below)
      ::rtl::OUString target_frame_name (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_FRAME_BLANK)));
      const char* result_p = NULL;
      sal_Int32 search_flags = frame::FrameSearchFlag::AUTO;
      document_properties.realloc (3);
      document_properties[0].Name =
        ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_HIDDEN)));
      document_properties[0].Value <<= true;
      document_properties[1].Name =
        ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_INTERACTIONHANDLER)));
      document_properties[1].Value = makeAny (interactionHandler_);
      document_properties[2].Name =
        ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_MACROEXECCUTIONMODE)));
      document_properties[2].Value <<=
        document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->connectionConfigurations);
      ACE_ASSERT (!inherited::configuration_->connectionConfigurations->empty ());

      ConnectionConfigurationIteratorType iterator =
        inherited::configuration_->connectionConfigurations->find (inherited::mod_->name ());
      if (iterator == inherited::configuration_->connectionConfigurations->end ())
        iterator =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: applying connection configuration\n"),
                    inherited::mod_->name ()));
      ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());
      ACE_TCHAR host_address[BUFSIZ];
      ACE_OS::memset (host_address, 0, sizeof (host_address));
      // *TODO*: remove type inferences
      result_p =
        (*iterator).second.socketHandlerConfiguration.socketConfiguration_2.address.get_host_addr (host_address,
                                                                                                   sizeof (host_address));
      if (!result_p || (result_p != host_address))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_INET_Addr::get_host_addr(%s): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString ((*iterator).second.socketHandlerConfiguration.socketConfiguration_2.address).c_str ())));
        goto error;
      } // end IF

      connection_string += ACE_TEXT_ALWAYS_CHAR (host_address);
      connection_string += ACE_TEXT_ALWAYS_CHAR (",port=");
      converter <<
        (*iterator).second.socketHandlerConfiguration.socketConfiguration_2.address.get_port_number ();
      connection_string += converter.str ();
      connection_string += ACE_TEXT_ALWAYS_CHAR (";urp;StarOffice.ServiceManager");
      connection_string_2 =
        ::rtl::OUString::createFromAscii (connection_string.c_str ());

      result_4 =
        componentContext_.set (::cppu::defaultBootstrap_InitialComponentContext ());
      ACE_ASSERT (result_4);
      result_4 = multi_component_factory_p.set (componentContext_->getServiceManager ());
      ACE_ASSERT (result_4);
      result_4 =
        interface_p.set (multi_component_factory_p->createInstanceWithContext (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR ("com.sun.star.bridge.UnoUrlResolver"))),
                                                                               componentContext_));
      ACE_ASSERT (result_4);
      uno::Reference<lang::XComponent>::query (multi_component_factory_p)->dispose ();
      result_4 = resolver_p.set (interface_p,
                                 uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      try {
        result_4 = interface_p.set (resolver_p->resolve (connection_string_2),
                                    uno::UNO_QUERY);
      } catch (uno::Exception& exception_in) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to establish a LibreOffice connection (was: \"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
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
                  ACE_TEXT ("%s: opened LibreOffice connection (was: \"%s\")\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (::rtl::OUStringToOString (connection_string_2,
                                                      RTL_TEXTENCODING_ASCII_US,
                                                      OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));

      result_4 = property_set_p.set (interface_p,
                                     uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      property_set_p->getPropertyValue (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_DEFAULT_CONTEXT)))) >>= componentContext_;
      //result_4 = server_p.set (component_componentContext_p->getServiceManager ());
      //ACE_ASSERT (result_4);

      result_4 =
        component_loader_p.set (multi_component_factory_p->createInstanceWithContext (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR ("com.sun.star.frame.Desktop"))),
                                                                                      componentContext_),
                                uno::UNO_QUERY);
      ACE_ASSERT (component_loader_p.is ());
      ACE_ASSERT (result_4);

      // generate document filename URL
      filename =
        ::rtl::OUString::createFromAscii (inherited::configuration_->targetFileName.c_str ());
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
        component_.set (component_loader_p->loadComponentFromURL (absolute_filename_url, // URL
                                                                  target_frame_name,     // target frame name
                                                                  search_flags,          // search flags
                                                                  document_properties),  // properties
                        uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: loaded LibreOffice document (was: \"%s\")\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (::rtl::OUStringToOString (absolute_filename_url,
                                                      RTL_TEXTENCODING_ASCII_US,
                                                      OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *TODO*: ::lang::XComponent::dispose crashes the application
      if (component_.is ())
        component_->dispose ();
      if (componentContext_.is ())
        uno::Reference<lang::XComponent>::query (componentContext_)->dispose ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionConfigurationIteratorType,
          typename SessionDataType,
          typename DocumentType>
bool
Stream_Module_LibreOffice_Document_Writer_T<SynchStrategyType,
                                            TimePolicyType,
                                            ConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            ConnectionConfigurationIteratorType,
                                            SessionDataType,
                                            DocumentType>::initialize (const ConfigurationType& configuration_in,
                                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Writer_T::initialize"));

  if (inherited::isInitialized_)
  {
    // *TODO*: ::lang::XComponent::dispose crashes the application
    if (component_.is ())
      component_->dispose ();
    if (componentContext_.is ())
      uno::Reference<lang::XComponent>::query (componentContext_)->dispose ();
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}
