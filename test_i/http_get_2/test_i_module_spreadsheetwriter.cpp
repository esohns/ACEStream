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

#include "test_i_module_spreadsheetwriter.h"

#include "stream_macros.h"

Test_I_Stream_SpreadsheetWriter::Test_I_Stream_SpreadsheetWriter ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_SpreadsheetWriter::Test_I_Stream_SpreadsheetWriter"));

}

Test_I_Stream_SpreadsheetWriter::~Test_I_Stream_SpreadsheetWriter ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_SpreadsheetWriter::~Test_I_Stream_SpreadsheetWriter"));

}

void
Test_I_Stream_SpreadsheetWriter::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_SpreadsheetWriter::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  const Test_I_Stream_SessionData_t& session_data_container_r =
    message_inout->get ();
  Test_I_Stream_SessionData& session_data_r =
    const_cast<Test_I_Stream_SessionData&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_->socketConfiguration);

      uno::Reference<uno::XInterface> interface_p;
      uno::Reference<uno::XComponentContext> component_context_p;
      uno::Reference<lang::XMultiComponentFactory> client_p;
      std::string connection_string = ACE_TEXT_ALWAYS_CHAR ("uno:socket,host=");
      std::ostringstream converter;
      ::rtl::OUString connection_string_2;
      uno::Reference<bridge::XUnoUrlResolver> resolver_p;
      uno::Reference<frame::XDesktop2> component_loader_p;
      uno::Reference<lang::XMultiComponentFactory> server_p;
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
        configuration_->libreOfficeHost.get_host_addr (host_address,
                                                       sizeof (host_address));
      if (!result_p || (result_p != host_address))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::get_host_addr(\"%s\"): \"%m\", aborting\n"),
                    buffer));
        goto error;
      } // end IF

      connection_string += ACE_TEXT_ALWAYS_CHAR (host_address);
      connection_string += ACE_TEXT_ALWAYS_CHAR (",port=");
      converter << configuration_->libreOfficeHost.get_port_number ();
      connection_string += converter.str ();
      connection_string +=
        ACE_TEXT_ALWAYS_CHAR (";urp;StarOffice.ServiceManager");
      connection_string_2 =
        ::rtl::OUString::createFromAscii (ACE_TEXT_ALWAYS_CHAR (connection_string.c_str ()));

      result_4 =
        component_context_p.set (::cppu::defaultBootstrap_InitialComponentContext ());
      ACE_ASSERT (result_4);
      result_4 = client_p.set (component_context_p->getServiceManager ());
      ACE_ASSERT (result_4);
      result_4 =
        interface_p.set (client_p->createInstanceWithContext (ACE_TEXT_ALWAYS_CHAR ("com.sun.star.bridge.UnoUrlResolver"),
                                                              component_context_p));
      ACE_ASSERT (result_4);
      uno::Reference<lang::XComponent>::query (client_p)->dispose ();
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
      property_set_p->getPropertyValue ("DefaultContext") >>=
        component_context_p;
      //result_4 = server_p.set (component_context_p->getServiceManager ());
      //ACE_ASSERT (result_4);

      result_4 =
        component_loader_p.set (frame::Desktop::create (component_context_p));
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
        inherited::document_.set (component_loader_p->loadComponentFromURL (absolute_filename_url,
                                  ::rtl::OUString ("_blank"),
                                  0,
                                  uno::Sequence<beans::PropertyValue> ()));
      ACE_ASSERT (result_4);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("loaded LibreOffice spreadsheet document (was: \"%s\")...\n"),
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

      break;
    }
    default:
      break;
  } // end SWITCH
}
