#ifndef TEST_U_CAMERA_FILTER_CONFIGURATION_H
#define TEST_U_CAMERA_FILTER_CONFIGURATION_H

#include "common_isubscribe.h"

#include "test_u_configuration.h"

#include "test_u_stream_common.h"

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
class Test_U_EventHandler_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_Configuration
 : Test_U_Configuration
{
  Test_U_DirectShow_Configuration ()
   : Test_U_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_U_DirectShow_StreamConfiguration_t             streamConfiguration;
};

struct Test_U_MediaFoundation_Configuration
 : Test_U_Configuration
{
  Test_U_MediaFoundation_Configuration ()
   : Test_U_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_U_MediaFoundation_StreamConfiguration_t        streamConfiguration;
};
#else
struct Test_U_V4L_Configuration
 : Test_U_Configuration
{
  Test_U_V4L_Configuration ()
   : Test_U_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Test_U_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Test_U_Message,
//                                         Test_U_SessionMessage> Test_U_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_DirectShow_Message_t,
                                          Test_U_DirectShow_SessionMessage_t> Test_U_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_MediaFoundation_Message_t,
                                          Test_U_MediaFoundation_SessionMessage_t> Test_U_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message_t,
                                          Test_U_SessionMessage_t> Test_U_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Test_U_DirectShow_ISessionNotify_t> Test_U_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Test_U_MediaFoundation_ISessionNotify_t> Test_U_MediaFoundation_ISubscribe_t;

typedef Test_U_EventHandler_T<Test_U_DirectShow_ISessionNotify_t,
                              Test_U_DirectShow_Message_t,
                              Test_U_DirectShow_SessionMessage_t> Test_U_DirectShow_EventHandler_t;
typedef Test_U_EventHandler_T<Test_U_MediaFoundation_ISessionNotify_t,
                              Test_U_MediaFoundation_Message_t,
                              Test_U_MediaFoundation_SessionMessage_t> Test_U_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Test_U_ISessionNotify_t> Test_U_ISubscribe_t;

typedef Test_U_EventHandler_T<Test_U_ISessionNotify_t,
                              Test_U_Message_t,
                              Test_U_SessionMessage_t> Test_U_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif // TEST_U_CAMERA_FILTER_CONFIGURATION_H
