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

#ifndef TEST_I_TARGET_COMMON_H
#define TEST_I_TARGET_COMMON_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/os_include/sys/os_socket.h"
#include "ace/Time_Value.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "d3d9.h"
#include "evr.h"
#include "mfapi.h"
//#include "mfidl.h"
//#include "mmeapi.h"
//#include "mtype.h"
#include "strmif.h"
#else
//#include "linux/videodev2.h"

#include "gtk/gtk.h"
#endif

#include "stream_dec_defines.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "stream_misc_defines.h"

#include "net_defines.h"
#include "net_ilistener.h"

#include "test_i_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct v4l2_window;
#endif
class Test_I_Target_Stream_Message;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_MessageData
{
  inline Test_I_Target_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMFMediaBuffer* sample;
  LONGLONG        sampleTime;
};
#endif

struct Test_I_Target_Configuration;
struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_UserData
 : Stream_UserData
{
  inline Test_I_Target_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_Target_Configuration*       configuration;
  Test_I_Target_StreamConfiguration* streamConfiguration;
};

struct Test_I_Target_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Target_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ////////////////////////////////////
   , userData (NULL)
  {};

  Test_I_Target_UserData* userData;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_PinConfiguration
{
  inline Test_I_Target_DirectShow_PinConfiguration ()
   : format (NULL)
   , queue (NULL)
  {};

  struct _AMMediaType*    format; // (preferred) media type handle
  ACE_Message_Queue_Base* queue;  // (inbound) buffer queue handle
};
struct Test_I_Target_DirectShow_FilterConfiguration
{
  inline Test_I_Target_DirectShow_FilterConfiguration ()
   : format (NULL)
   , module (NULL)
   , pinConfiguration (NULL)
  {};

  // *TODO*: specify this as part of the network protocol header/handshake
  struct _AMMediaType*                       format; // handle
  Stream_Module_t*                           module; // handle
  Test_I_Target_DirectShow_PinConfiguration* pinConfiguration; // handle
};
#endif

struct Test_I_Target_Stream_ModuleHandlerConfiguration
 : Test_I_Stream_ModuleHandlerConfiguration
{
  inline Test_I_Target_Stream_ModuleHandlerConfiguration ()
   : Test_I_Stream_ModuleHandlerConfiguration ()
   , area ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , device ()
   //, filterCLSID ()
   //, filterConfiguration (NULL)
   , format (NULL)
   , mediaSource (NULL)
   //, push (MODULE_MISC_DS_WIN32_FILTER_SOURCE_DEFAULT_PUSH)
   , queue (NULL)
#else
   , format ()
#endif
   , connection (NULL)
   , connectionManager (NULL)
   , printProgressDot (false)
   , targetFileName ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , v4l2Window (NULL)
#endif
   , window (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , windowController (NULL)
#endif
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //format =
    //  static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    //if (!format)
    //{
    //  ACE_DEBUG ((LM_CRITICAL,
    //              ACE_TEXT ("failed to allocate memory, continuing\n")));
    //} // end IF
    //else
    //  ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
    HRESULT result = MFCreateMediaType (&format);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#endif
  };

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct tagRECT                                  area;
  std::string                                     device; // FriendlyName
  //Test_I_Target_DirectShow_FilterConfiguration*   filterConfiguration;
  //struct _GUID                                    filterCLSID;
  //struct _AMMediaType*                            format; // splitter module
  IMFMediaType*                                   format;
  IMFMediaSource*                                 mediaSource;
  //bool                                          push; // media sample passing strategy
  ACE_Message_Queue_Base*                         queue;  // (inbound) buffer queue handle
#else
  GdkRectangle                                    area;
  struct v4l2_format                              format; // splitter module
#endif
  Test_I_Target_IConnection_t*                    connection; // Net source/IO module
  Test_I_Target_InetConnectionManager_t*          connectionManager; // Net IO module
  bool                                            printProgressDot;
  std::string                                     targetFileName; // file writer module
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                                            window;
  //IVideoWindow*                                   windowController;
  IMFVideoDisplayControl*                         windowController;
#else
  struct v4l2_window*                             v4l2Window;
  GdkWindow*                                      window;
#endif
};

struct Test_I_Target_ListenerConfiguration
{
  inline Test_I_Target_ListenerConfiguration ()
   : address (TEST_I_DEFAULT_PORT, static_cast<ACE_UINT32> (INADDR_ANY))
   , addressFamily (ACE_ADDRESS_FAMILY_INET)
   , connectionManager (NULL)
   , messageAllocator (NULL)
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
   , useLoopBackDevice (false)
  {};

  ACE_INET_Addr                             address;
  int                                       addressFamily;
  Test_I_Target_IInetConnectionManager_t*   connectionManager;
  Stream_IAllocator*                        messageAllocator;
  Test_I_Target_SocketHandlerConfiguration* socketHandlerConfiguration;
  ACE_Time_Value                            statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                                      useLoopBackDevice;
};

typedef Net_IListener_T<Test_I_Target_ListenerConfiguration,
                        Test_I_Target_SocketHandlerConfiguration> Test_I_Target_IListener_t;

struct Test_I_Target_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_I_Target_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerID (-1)
  {};

  Test_I_Target_IListener_t*          listener;
  Test_I_StatisticReportingHandler_t* statisticReportingHandler;
  long                                statisticReportingTimerID;
};

struct Test_I_Target_Stream_SessionData
 : Test_I_Stream_SessionData
{
  inline Test_I_Target_Stream_SessionData ()
   : Test_I_Stream_SessionData ()
   , connectionState (NULL)
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//   , direct3DDevice (NULL)
//   , format (NULL)
//   , resetToken (0)
//#else
//   , format ()
//#endif
   , targetFileName ()
   , userData (NULL)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //format =
    //  static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    //if (!format)
    //{
    //  ACE_DEBUG ((LM_CRITICAL,
    //              ACE_TEXT ("failed to allocate memory, continuing\n")));
    //} // end IF
    //else
    //  ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
    //HRESULT result = MFCreateMediaType (&format);
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#endif
  };

  inline Test_I_Target_Stream_SessionData& operator+= (const Test_I_Target_Stream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_Stream_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState : rhs_in.connectionState);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    // sanity check(s)
//    ACE_ASSERT (rhs_in.format);
//
//    if (format)
//    {
//      format->Release ();
//      format = NULL;
//    } // end IF
//
//    //if (!Stream_Module_Device_Tools::copyMediaType (*rhs_in.format,
//    //                                                format))
//    //  ACE_DEBUG ((LM_ERROR,
//    //              ACE_TEXT ("failed to Stream_Module_Device_Tools::copyMediaType(), continuing\n")));
//    struct _AMMediaType media_type;
//    ACE_OS::memset (&media_type, 0, sizeof (media_type));
//    HRESULT result = MFInitAMMediaTypeFromMFMediaType (rhs_in.format,
//                                                       GUID_NULL,
//                                                       &media_type);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFInitAMMediaTypeFromMFMediaType(): \"%s\", continuing\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//      goto continue_;
//    } // end IF
//
//    result = MFInitMediaTypeFromAMMediaType (format,
//                                             &media_type);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFInitMediaTypeFromAMMediaType(): \"%s\", continuing\n"),
//                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//
//      // clean up
//      Stream_Module_Device_Tools::freeMediaType (media_type);
//
//      goto continue_;
//    } // end IF
//
//    // clean up
//    Stream_Module_Device_Tools::freeMediaType (media_type);
//continue_:
//#else
//    format = rhs_in.format;
//#endif
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_I_Target_ConnectionState* connectionState;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  //struct _AMMediaType*           format;
//  IDirect3DDevice9Ex*            direct3DDevice;
//  IMFMediaType*                  format;
//  UINT                           resetToken; // direct 3D manager 'id'
//#else
//  struct v4l2_format             format;
//#endif
  std::string                    targetFileName;
  Test_I_Target_UserData*        userData;
};
typedef Stream_SessionData_T<Test_I_Target_Stream_SessionData> Test_I_Target_Stream_SessionData_t;

struct Test_I_Target_StreamConfiguration
 : Stream_Configuration
{
  inline Test_I_Target_StreamConfiguration ()
   : Stream_Configuration ()
   //, graphBuilder (NULL)
   , moduleHandlerConfiguration (NULL)
   //, window (NULL)
  {};

  //IGraphBuilder*                                   graphBuilder;
  Test_I_Target_Stream_ModuleHandlerConfiguration* moduleHandlerConfiguration;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  HWND                                             window;
//#else
//  GdkWindow*                                       window;
//#endif
};

struct Test_I_Target_StreamState
 : Stream_State
{
  inline Test_I_Target_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_I_Target_Stream_SessionData* currentSessionData;
  Test_I_Target_UserData*           userData;
};

struct Test_I_Target_Configuration
 : Test_I_Configuration
{
  inline Test_I_Target_Configuration ()
   : Test_I_Configuration ()
   , socketHandlerConfiguration ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   //, pinConfiguration ()
   //, filterConfiguration ()
#endif
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** socket data **********************************
  Test_I_Target_SocketHandlerConfiguration        socketHandlerConfiguration;
  // **************************** listener data ********************************
  ACE_HANDLE                                      handle;
  //Test_I_Target_IListener_t*               listener;
  Test_I_Target_ListenerConfiguration             listenerConfiguration;
  // **************************** signal data **********************************
  Test_I_Target_SignalHandlerConfiguration        signalHandlerConfiguration;
  // **************************** stream data **********************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //Test_I_Target_DirectShow_PinConfiguration       pinConfiguration;
  //Test_I_Target_DirectShow_FilterConfiguration    filterConfiguration;
#endif
  Test_I_Target_Stream_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_I_Target_StreamConfiguration               streamConfiguration;

  Test_I_Target_UserData                          userData;
};

struct Test_I_Target_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_I_Target_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    buffer = STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};
typedef Stream_MessageAllocatorHeapBase_T<Test_I_Target_AllocatorConfiguration,

                                          Test_I_Target_Stream_Message,
                                          Test_I_Target_Stream_SessionMessage> Test_I_Target_MessageAllocator_t;

typedef Common_INotify_T<unsigned int,
                         Test_I_Target_Stream_SessionData,
                         Test_I_Target_Stream_Message,
                         Test_I_Target_Stream_SessionMessage> Test_I_Target_IStreamNotify_t;
typedef std::list<Test_I_Target_IStreamNotify_t*> Test_I_Target_Subscribers_t;
typedef Test_I_Target_Subscribers_t::iterator Test_I_Target_SubscribersIterator_t;

typedef Common_ISubscribe_T<Test_I_Target_IStreamNotify_t> Test_I_Target_ISubscribe_t;

struct Test_I_Target_GTK_CBData
 : Test_I_GTK_CBData
{
  inline Test_I_Target_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , configuration (NULL)
   , progressEventSourceID (0)
   , subscribers ()
  {};

  Test_I_Target_Configuration* configuration;
  guint                        progressEventSourceID;
  Test_I_Target_Subscribers_t  subscribers;
};

#endif
