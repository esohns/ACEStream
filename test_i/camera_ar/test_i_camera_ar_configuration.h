#ifndef TEST_I_CAMERA_AR_CONFIGURATION_H
#define TEST_I_CAMERA_AR_CONFIGURATION_H

#include "ace/Synch_Traits.h"

#include "common_configuration.h"
#include "common_isubscribe.h"

#include "stream_messageallocatorheap_base.h"

#include "test_i_configuration.h"

#include "test_i_camera_ar_stream_common.h"

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_CameraAR_EventHandler_T;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraAR_DirectShow_Configuration
 : Test_I_Configuration
{
  Stream_CameraAR_DirectShow_Configuration ()
   : Test_I_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration  direct3DConfiguration;
  Stream_CameraAR_DirectShow_StreamConfiguration_t streamConfiguration;
};

struct Stream_CameraAR_MediaFoundation_Configuration
 : Test_I_Configuration
{
  Stream_CameraAR_MediaFoundation_Configuration ()
   : Test_I_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration       direct3DConfiguration;
  Stream_CameraAR_MediaFoundation_StreamConfiguration_t streamConfiguration;
};
#else
struct Stream_CameraAR_Configuration
 : Test_I_Configuration
{
  Stream_CameraAR_Configuration ()
   : Test_I_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Stream_CameraAR_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Stream_CameraAR_Message,
//                                         Stream_CameraAR_SessionMessage> Stream_CameraAR_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraAR_DirectShow_Message_t,
                                          Stream_CameraAR_DirectShow_SessionMessage_t> Stream_CameraAR_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraAR_MediaFoundation_Message_t,
                                          Stream_CameraAR_MediaFoundation_SessionMessage_t> Stream_CameraAR_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraAR_Message_t,
                                          Stream_CameraAR_SessionMessage_t> Stream_CameraAR_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Stream_CameraAR_DirectShow_ISessionNotify_t> Stream_CameraAR_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Stream_CameraAR_MediaFoundation_ISessionNotify_t> Stream_CameraAR_MediaFoundation_ISubscribe_t;

typedef Stream_CameraAR_EventHandler_T<Stream_CameraAR_DirectShow_ISessionNotify_t,
                                       Stream_CameraAR_DirectShow_Message_t,
                                       Stream_CameraAR_DirectShow_SessionMessage_t> Stream_CameraAR_DirectShow_EventHandler_t;
typedef Stream_CameraAR_EventHandler_T<Stream_CameraAR_MediaFoundation_ISessionNotify_t,
                                       Stream_CameraAR_MediaFoundation_Message_t,
                                       Stream_CameraAR_MediaFoundation_SessionMessage_t> Stream_CameraAR_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Stream_CameraAR_ISessionNotify_t> Stream_CameraAR_ISubscribe_t;

typedef Stream_CameraAR_EventHandler_T<Stream_CameraAR_ISessionNotify_t,
                                       Stream_CameraAR_Message_t,
                                       Stream_CameraAR_SessionMessage_t> Stream_CameraAR_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif // TEST_I_CAMERA_AR_CONFIGURATION_H
