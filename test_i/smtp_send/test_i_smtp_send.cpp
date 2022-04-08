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

#include <iostream>
#include <string>

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gdk/gdkwin32.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "gtk/gtk.h"
#elif defined (WXWIDGETS_USE)
#endif
#endif // GUI_SUPPORT

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_defines.h"
#include "common_ui_tools.h"
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#elif defined (WXWIDGETS_USE)
#include "common_ui_wxwidgets_application.h"
#include "common_ui_wxwidgets_tools.h"
#endif
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "smtp_stream_common.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_smtp_send_defines.h"
#include "test_i_smtp_send_eventhandler.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_smtp_send_gtk_callbacks.h"
#elif defined (WXWIDGETS_USE)
#include "test_u_smtp_send_ui.h"
#endif
#endif // GUI_SUPPORT
#include "test_i_smtp_send_common_modules.h"
#include "test_i_smtp_send_signalhandler.h"
#include "test_i_smtp_send_stream.h"
#include "test_i_smtp_send_stream_common.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("SMTPSend_Stream");
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
const char toplevel_widget_classname_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME);
const char toplevel_widget_name_string_[] =
  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME);
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : destination address")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : source address")
            << std::endl;
#if defined (GUI_SUPPORT)
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
#endif // GUI_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-h [STRING] : server host")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [VALUE]  : server port")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m [STRING] : message")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [STRING] : password")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (_DEBUG)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // _DEBUG
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [STRING] : user name")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (_DEBUG)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-z          : debug parser")
            << std::endl;
#endif // _DEBUG
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     std::string& destinationAddress_out,
                     std::string& sourceAddress_out,
#if defined (GUI_SUPPORT)
                     std::string& UIFile_out,
#endif // GUI_SUPPORT
                     ACE_INET_Addr& address_out,
                     bool& logToFile_out,
                     std::string& message_out,
                     std::string& password_out,
                     bool& useReactor_out,
#if defined (_DEBUG)
                     bool& traceInformation_out,
#endif // _DEBUG
                     std::string& userName_out,
                     enum Stream_SMTPSend_ProgramMode& mode_out
#if defined (_DEBUG)
                     ,bool& debugParser_out
#endif // _DEBUG
                     )
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  int result = -1;
  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  destinationAddress_out.clear ();
  sourceAddress_out.clear ();
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT
  address_out.set (0,
                   ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
                   AF_INET);
  logToFile_out = false;
  message_out.clear ();
  password_out.clear ();
  useReactor_out =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
#if defined (_DEBUG)
  traceInformation_out = false;
#endif // _DEBUG
  userName_out.clear ();
  mode_out = STREAM_SMTPSEND_PROGRAMMODE_NORMAL;
#if defined (_DEBUG)
  debugParser_out = false;
#endif // _DEBUG

  std::string option_string = ACE_TEXT_ALWAYS_CHAR ("d:f:h:i:lm:p:rs:u:v");
#if defined (GUI_SUPPORT)
  option_string += ACE_TEXT_ALWAYS_CHAR ("g::");
#endif // GUI_SUPPORT
#if defined (_DEBUG)
  option_string += ACE_TEXT_ALWAYS_CHAR ("tz");
#endif // _DEBUG
  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT (option_string.c_str ()),
                              1,                          // skip command name
                              1,                          // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS,  // ordering
                              0);                         // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'd':
      {
        destinationAddress_out =
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'f':
      {
        sourceAddress_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
#if defined (GUI_SUPPORT)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
#endif // GUI_SUPPORT
      case 'h':
      {
        result = address_out.set (static_cast<u_short> (SMTP_DEFAULT_SERVER_PORT),
                                  ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()),
                                  AF_INET);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                      argumentParser.opt_arg ()));
          return false;
        } // end IF
        break;
      }
      case 'i':
      {
        ACE_UINT16 port_i = 0;
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> port_i;
        address_out.set_port_number (port_i, 1);
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'm':
      {
        message_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'p':
      {
        password_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
#if defined (_DEBUG)
      case 't':
      {
        traceInformation_out = true;
        break;
      }
#endif // _DEBUG
      case 'u':
      {
        userName_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'v':
      {
        mode_out = STREAM_SMTPSEND_PROGRAMMODE_PRINT_VERSION;
        break;
      }
#if defined (_DEBUG)
      case 'z':
      {
        debugParser_out = true;
        break;
      }
#endif // _DEBUG
      // error handling
      case ':':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("option \"%c\" requires an argument, aborting\n"),
                    argumentParser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    argumentParser.last_option ()));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    argumentParser.long_option ()));
        return false;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("parse error, aborting\n")));
        return false;
      }
    } // end SWITCH
  } // end WHILE

  return true;
}

void
do_initializeSignals (ACE_Sig_Set& signals_out,
                      ACE_Sig_Set& ignoredSignals_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initializeSignals"));

  int result = -1;

  // initialize return value(s)
  result = signals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF
  result = ignoredSignals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF

  // *PORTABILITY*: on Windows(TM) platforms most signals are not defined, and
  //                ACE_Sig_Set::fill_set() doesn't really work as specified
  // --> add valid signals (see <signal.h>)...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signals_out.sig_add (SIGINT);          // 2  /* interrupt */
  signals_out.sig_add (SIGILL);          // 4  /* illegal instruction - invalid function image */
  signals_out.sig_add (SIGFPE);          // 8  /* floating point exception */
  //  signals_out.sig_add (SIGSEGV);     // 11 /* segment violation */
  signals_out.sig_add (SIGTERM);         // 15 /* Software termination signal from kill */
  signals_out.sig_add (SIGBREAK);        // 21 /* Ctrl-Break sequence */
  signals_out.sig_add (SIGABRT);         // 22 /* abnormal termination triggered by abort call */
  signals_out.sig_add (SIGABRT_COMPAT);  // 6  /* SIGABRT compatible with other platforms, same as SIGABRT */
#else
  result = signals_out.fill_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::fill_set(): \"%m\", returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (
#if defined (GUI_SUPPORT)
         const std::string& UIDefinitionFilename_in,
         struct Stream_SMTPSend_UI_CBData& CBData_in,
#if defined (WXWIDGETS_USE)
         Common_UI_wxWidgets_IApplicationBase_t* iapplication_in,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         struct Stream_SMTPSend_Configuration& configuration_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_SMTPSend_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    //CBData_in.UIState->gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE
#endif // GUI_SUPPORT

  // ********************** module configuration data **************************
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct SMTP_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (configuration_in.allocatorConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF

  struct SMTP_Request request_s;
  struct Stream_ModuleConfiguration module_configuration;
  struct Stream_SMTPSend_ModuleHandlerConfiguration modulehandler_configuration;
  Stream_SMTPSend_EventHandler_t ui_event_handler (
#if defined (GUI_SUPPORT)
                                                   &CBData_in
#if defined (GTK_USE)
#elif defined (WXWIDGETS_USE)
                                                   ,iapplication_in
#endif
#endif // GUI_SUPPORT
                                                   );

  Stream_SMTPSend_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                        &heap_allocator,     // heap allocator handle
                                                        true);               // block ?
  Stream_SMTPSend_StreamConfiguration_t stream_configuration_connection;
  Test_I_SMTPSend_ConnectionConfiguration_t connection_configuration;
  connection_configuration.allocatorConfiguration =
    &configuration_in.allocatorConfiguration;
  connection_configuration.delayRead = true;
  connection_configuration.socketConfiguration.address =
    configuration_in.address;
  connection_configuration.streamConfiguration =
    &stream_configuration_connection;
  Net_ConnectionConfigurations_t connection_configurations;
  connection_configurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                    &connection_configuration));

  modulehandler_configuration.allocatorConfiguration =
    &configuration_in.allocatorConfiguration;
  modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration.connectionConfigurations =
    &connection_configurations;
  modulehandler_configuration.messageAllocator = &message_allocator;
  modulehandler_configuration.parserConfiguration =
    &configuration_in.parserConfiguration;
  modulehandler_configuration.protocolConfiguration =
    &configuration_in.protocolConfiguration;
  modulehandler_configuration.request = &request_s;
  if (statisticReportingInterval_in)
    modulehandler_configuration.statisticCollectionInterval.set (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
#if defined (GUI_SUPPORT)
  modulehandler_configuration.subscriber = &ui_event_handler;
#endif // GUI_SUPPORT

  Stream_SMTPSend_MessageHandler_Module message_handler (NULL,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_SMTPSend_Stream stream;
  struct SMTP_StreamConfiguration stream_configuration;
  struct SMTP_StreamConfiguration stream_configuration_2; // connection stream-
  stream_configuration.allocatorConfiguration =
    &configuration_in.allocatorConfiguration;
  stream_configuration.dispatchConfiguration =
    &configuration_in.dispatchConfiguration;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &message_handler;
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  stream_configuration_2 = stream_configuration;
  stream_configuration_2.module = NULL;
  stream_configuration_connection.initialize (module_configuration,
                                              modulehandler_configuration,
                                              stream_configuration_2);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  int result = -1;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // retrieve external address
  ACE_INET_Addr interface_address, gateway_address;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier =
    Net_Common_Tools::getDefaultInterface_2 (NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP);
#else
  std::string interface_identifier =
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP);;
#endif // _WIN32_WINNT_VISTA
#else
  std::string interface_identifier =
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP);;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (InlineIsEqualGUID (interface_identifier, GUID_NULL))
#else
  if (interface_identifier.empty ())
#endif // _WIN32_WINNT_VISTA
#else
  if (interface_identifier.empty ())
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::getDefaultInterface(), aborting\n")));
    return;
  } // end IF
  if (!Net_Common_Tools::interfaceToExternalIPAddress (interface_identifier,
                                                       configuration_in.domain))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // _WIN32_WINNT_VISTA
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
    return;
  } // end IF

  // initialize signal handling
  signalHandler_in.initialize (configuration_in.signalHandlerConfiguration);
  if (!Common_Signal_Tools::initialize ((useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                       : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    return;
  } // end IF

  // intialize timers
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (configuration_in.timerConfiguration);
  timer_manager_p->start (NULL);

  struct Net_UserData user_data_s;
  Test_I_SMTPSend_Connection_Manager_t* connection_manager_p =
    TEST_I_SMTPSEND_CONNECTIONMANAGER_SINGLETON::instance ();
  struct Common_EventDispatchState dispatch_state;
  dispatch_state.configuration =
    &configuration_in.dispatchConfiguration;

  // initialize event dispatch
  configuration_in.dispatchConfiguration.dispatch =
    (useReactor_in ? COMMON_EVENT_DISPATCH_REACTOR : COMMON_EVENT_DEFAULT_DISPATCH);
  configuration_in.dispatchConfiguration.numberOfProactorThreads =
          (!useReactor_in ? TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS : 0);
  configuration_in.dispatchConfiguration.numberOfReactorThreads =
          (useReactor_in ? TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS : 0);
  if (!Common_Tools::initializeEventDispatch (configuration_in.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    goto clean;
    return;
  } // end IF

  // initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (connection_configuration,
                             &user_data_s);

  // start event loop(s)
  if (!Common_Tools::startEventDispatch (dispatch_state))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
    goto clean;
  } // end IF

#if defined (GUI_SUPPORT)
  CBData_in.configuration = &configuration_in;
  // step1a: start UI event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
    CBData_in.stream = &stream;
#if defined (GTK_USE)
    CBData_in.UIState = &state_r;
    CBData_in.progressData.state = &state_r;
#elif defined (WXWIDGETS_USE)
    struct Common_UI_wxWidgets_State& state_r =
      const_cast<struct Common_UI_wxWidgets_State&> (iapplication_in->getR ());
    state_r.resources[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<wxObject*> (NULL));
#endif // GTK_USE

#if defined (GTK_USE)
    ACE_ASSERT (gtk_manager_p);
    gtk_manager_p->start (NULL);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
    result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      goto clean;
    } // end IF
    gtk_manager_p->wait (false);
#elif (WXWIDGETS_USE)
    if (unlikely (!iapplication_in->run ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_wxWidgets_IApplicationBase_T::run(), returning\n")));
      goto clean;
    } // end IF
#endif // GTK_USE || WXWIDGETS_USE
  } // end IF
  else
  {
#endif // GUI_SUPPORT
    Stream_IStreamControlBase* stream_p = NULL;
    if (!stream.initialize (configuration_in.streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, returning\n")));
      goto clean;
    } // end IF
    stream_p = &stream;
    stream_p->start ();
//    if (!stream_p->isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start stream, aborting\n")));
//      //timer_manager_p->stop ();
//      return;
//    } // end IF
    stream_p->wait (true, false, false);
#if defined (GUI_SUPPORT)
  } // end ELSE
#endif // GUI_SUPPORT

  // step3: clean up
clean:
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  gtk_manager_p->stop (true, true);
#endif // GTK_USE
#endif // GUI_SUPPORT
  connection_manager_p->stop (false, true);
  connection_manager_p->abort ();
  connection_manager_p->wait ();

  Common_Tools::finalizeEventDispatch (dispatch_state,
                                       true);

  timer_manager_p->stop ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

COMMON_DEFINE_PRINTVERSION_FUNCTION(do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result = -1;
#endif // ACE_WIN32 || ACE_WIN64

  // step0: initialize
  // *PORTABILITY*: on Windows, initialize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::init ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize framework(s)
  Common_Tools::initialize (false); // initialize random number generator ?
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  if (!Common_UI_GTK_Tools::initialize (argc_in,
                                        argv_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::initialize(), aborting\n")));
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
#elif defined (WXWIDGETS_USE)
  if (!Common_UI_WxWidgets_Tools::initialize (argc_in,
                                              argv_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_WxWidgets_Tools::initialize(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

  // step1a set defaults
  struct Stream_SMTPSend_Configuration configuration;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  std::string UI_definition_filename = path;
  UI_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_definition_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_DEFINITION_FILE);
#endif // GUI_SUPPORT
  bool log_to_file = false;
  bool use_reactor =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
#if defined (_DEBUG)
  bool trace_information = false;
#endif // _DEBUG
  enum Stream_SMTPSend_ProgramMode program_mode_e =
      STREAM_SMTPSEND_PROGRAMMODE_NORMAL;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  bool result_2 = false;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            configuration.to,
                            configuration.from,
#if defined (GUI_SUPPORT)
                            UI_definition_filename,
#endif // GUI_SUPPORT
                            configuration.address,
                            log_to_file,
                            configuration.message,
                            configuration.password,
                            use_reactor,
#if defined (_DEBUG)
                            trace_information,
#endif // _DEBUG
                            configuration.username,
                            program_mode_e
#if defined (_DEBUG)
                            ,configuration.parserConfiguration.debugParser
#endif // _DEBUG
                           ))
  {
    do_printUsage (ACE::basename (argv_in[0]));
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
#if defined (_DEBUG)
  if (configuration.parserConfiguration.debugParser)
    configuration.parserConfiguration.debugScanner = true;
#endif // _DEBUG

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_I_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to a deadlock --> ensure the streaming elements are sufficiently efficient in this regard\n")));
  if (
#if defined (GUI_SUPPORT)
      (!UI_definition_filename.empty () &&
       !Common_File_Tools::isReadable (UI_definition_filename)) ||
#else
      configuration.to.empty () ||
      configuration.from.empty () ||
      configuration.password.empty () ||
      configuration.username.empty () ||
#endif // GUI_SUPPORT
     false)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1d: initialize logging and/or tracing
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_MessageStack_t* logstack_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  logstack_p = &state_r.logStack;
  lock_p = &state_r.logStackLock;

  Common_Logger_t logger (logstack_p,
                          lock_p);
#endif // GTK_USE
#endif // GUI_SUPPORT
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME),
                                          ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
  if (!Common_Log_Tools::initializeLogging (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])), // program name
                                            log_file_name,                                // log file name
                                            false,                                        // log to syslog ?
                                            false,                                        // trace messages ?
#if defined (_DEBUG)
                                            trace_information,                            // debug messages ?
#else
                                            false,                                        // debug messages ?
#endif // _DEBUG
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                            (UI_definition_filename.empty () ? NULL
                                                                             : &logger))) // (ui) logger ?
#elif defined (QT_USE)
                                            NULL))                                        // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                            NULL))                                        // (ui) logger ?
#else
                                            NULL))                                        // (ui) logger ?
#endif
#else
                                            NULL))                                        // (ui) logger ?
#endif // GUI_SUPPORT
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1f: handle specific program modes
  switch (program_mode_e)
  {
    case STREAM_SMTPSEND_PROGRAMMODE_PRINT_VERSION:
    {
      do_printVersion (ACE::basename (argv_in[0]));

      Common_Log_Tools::finalizeLogging ();
      // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_SUCCESS;
    }
    case STREAM_SMTPSEND_PROGRAMMODE_NORMAL:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown program mode (was: %d), aborting\n"),
                  program_mode_e));

      // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_FAILURE;
    }
  } // end SWITCH

#if defined (GUI_SUPPORT)
  struct Stream_SMTPSend_UI_CBData* ui_cb_data_p = NULL;
  struct Stream_SMTPSend_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
  ui_cb_data_p = &ui_cb_data;

#if defined (GTK_USE)
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
#endif // GTK_USE
  ACE_ASSERT (ui_cb_data_p);
#endif // GUI_SUPPORT

  // step1h: initialize UI framework
#if defined (GUI_SUPPORT)
  struct Common_UI_State* ui_state_p = NULL;
#if defined (GTK_USE)
  ui_state_p = &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#elif defined (WXWIDGETS_USE)
  Common_UI_wxWidgets_IApplicationBase_t* iapplication_p = NULL;
//  // *WORKAROUND*: this prevents crashing the wxGTK3 application in Fedora 29
//  GtkCssProvider* css_provider_p = gtk_css_provider_new ();
//  if (!css_provider_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to gtk_css_provider_new(), returning\n")));

//    Common_Log_Tools::finalizeLogging ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // *PORTABILITY*: on Windows, finalize ACE...
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
//    return EXIT_FAILURE;
//  } // end IF
//  GError* error_p = NULL;
  std::string css_profile_path = Common_File_Tools::getWorkingDirectory ();
  css_profile_path += ACE_DIRECTORY_SEPARATOR_STR;
  css_profile_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  css_profile_path += ACE_DIRECTORY_SEPARATOR_STR;
  css_profile_path +=ACE_TEXT_ALWAYS_CHAR (TEST_U_Stream_smtp_send_UI_CSS_FILE);
//  if (!gtk_css_provider_load_from_path (css_provider_p,
//                                        css_profile_path.c_str (),
//                                        &error_p))
//  { ACE_ASSERT (error_p);
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gtk_css_provider_load_from_path(\"%s\"): \"%s\", returning\n"),
//                ACE_TEXT (css_profile_path.c_str ()),
//                ACE_TEXT (error_p->message)));
//    g_error_free (error_p); error_p = NULL;

//    Common_Log_Tools::finalizeLogging ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // *PORTABILITY*: on Windows, finalize ACE...
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
//    return EXIT_FAILURE;
//  } // end IF
//  GdkDisplay* display_p = gdk_display_open (ACE_TEXT_ALWAYS_CHAR (":0"));
//  ACE_ASSERT (display_p);
//  gdk_display_manager_set_default_display (gdk_display_manager_get (),
//                                           display_p);
//  GdkScreen* screen_p = gdk_screen_get_default ();
//  ACE_ASSERT (screen_p);
//  gtk_style_context_add_provider_for_screen (screen_p,
//                                             GTK_STYLE_PROVIDER (css_provider_p),
//                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  ACE_NEW_NORETURN (iapplication_p,
                    Stream_smtp_send_V4L_WxWidgetsApplication_t (toplevel_widget_name_string_,
                                                               argc_in,
                                                               Common_UI_WxWidgets_Tools::convertArgV (argc_in,
                                                                                                       argv_in),
                                                               COMMON_UI_WXWIDGETS_APP_CMDLINE_DEFAULT_PARSE));
  Stream_smtp_send_V4L_WxWidgetsApplication_t::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Stream_smtp_send_V4L_WxWidgetsApplication_t::IINITIALIZE_T*> (iapplication_p);
  // *NOTE*: this sets ui_cb_data.UIState
  iinitialize_p->initialize (ui_cb_data);
  Stream_smtp_send_V4L_WxWidgetsIApplication_t* iapplication_2 =
    dynamic_cast<Stream_smtp_send_V4L_WxWidgetsIApplication_t*> (iapplication_p);
  ACE_ASSERT (iapplication_2);
  const Stream_smtp_send_V4L_WxWidgetsApplication_t::CONFIGURATION_T& configuration_r =
    iapplication_2->getR_2 ();
  ACE_ASSERT (configuration_r.UIState);
  ui_state_p =
    const_cast<Stream_smtp_send_V4L_WxWidgetsApplication_t::CONFIGURATION_T&> (configuration_r).UIState;

  if (!iapplication_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: %m, aborting\n")));

    Common_Log_Tools::finalizeLogging ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
#endif
  ACE_ASSERT (ui_state_p);
#endif // GUI_SUPPORT

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                        : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                           true, // use networking
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
#endif // GUI_SUPPORT
  Stream_SMTPSend_SignalHandler signal_handler;

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false,  // file descriptors
                                        true,   // stack traces
                                        false)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

#if defined (GUI_SUPPORT)
  if (!UI_definition_filename.empty ())
  {
#if defined (GTK_USE)
    result_2 =
        gtk_manager_p->initialize (ui_cb_data.configuration->GTKConfiguration);
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), aborting\n")));

      Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                  : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                     previous_signal_actions,
                                     previous_signal_mask);
      Common_Log_Tools::finalizeLogging ();
      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_FAILURE;
    } // end IF
#endif // GTK_USE
  } // end IF
#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (
#if defined (GUI_SUPPORT)
           UI_definition_filename,
           ui_cb_data,
#if defined (WXWIDGETS_USE)
           iapplication_p,
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
           use_reactor,
           statistic_reporting_interval,
           configuration,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // stop profile timer...
  process_profile.stop ();

  // only process profile left to do...
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  if (process_profile.elapsed_time (elapsed_time) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
  user_time_string = Common_Timer_Tools::periodToString (user_time);
  system_time_string = Common_Timer_Tools::periodToString (system_time);

  // debug info
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\nmaximum resident set size = %d\nintegral shared memory size = %d\nintegral unshared data size = %d\nintegral unshared stack size = %d\npage reclaims = %d\npage faults = %d\nswaps = %d\nblock input operations = %d\nblock output operations = %d\nmessages sent = %d\nmessages received = %d\nsignals received = %d\nvoluntary context switches = %d\ninvoluntary context switches = %d\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ()),
              elapsed_rusage.ru_maxrss,
              elapsed_rusage.ru_ixrss,
              elapsed_rusage.ru_idrss,
              elapsed_rusage.ru_isrss,
              elapsed_rusage.ru_minflt,
              elapsed_rusage.ru_majflt,
              elapsed_rusage.ru_nswap,
              elapsed_rusage.ru_inblock,
              elapsed_rusage.ru_oublock,
              elapsed_rusage.ru_msgsnd,
              elapsed_rusage.ru_msgrcv,
              elapsed_rusage.ru_nsignals,
              elapsed_rusage.ru_nvcsw,
              elapsed_rusage.ru_nivcsw));
#endif // ACE_WIN32 || ACE_WIN64

  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
