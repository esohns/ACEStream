#ifndef STREAM_MODULE_DEV_COMMON_H
#define STREAM_MODULE_DEV_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <map>

extern "C"
{
#include "alsa/asoundlib.h"
}
#include "linux/videodev2.h"

// forward declarations
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_IAllocator;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef std::map<__u32, ACE_Message_Block*> Stream_Module_Device_BufferMap_t;
typedef Stream_Module_Device_BufferMap_t::const_iterator Stream_Module_Device_BufferMapIterator_t;
#endif

enum Stream_Module_Device_Mode
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_MODULE_DEV_MODE_DIRECTSHOW = 0,
  STREAM_MODULE_DEV_MODE_MEDIAFOUNDATION,
#else
  // *** audio ONLY (!) ***
  STREAM_MODULE_DEV_MODE_ALSA = 0,
  // *** video ONLY (!) ***
  STREAM_MODULE_DEV_MODE_V4L2,
#endif
  ////////////////////////////////////////
  STREAM_MODULE_DEV_MODE_MAX,
  STREAM_MODULE_DEV_MODE_INVALID
};

//struct Stream_Module_Device_CamOptions
//{
//  inline Stream_Module_Device_CamOptions ()
//  {};

//};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Stream_Module_Device_ALSA_Capture_AsynchCBData
{
  Stream_IAllocator*            allocator;
//  struct _snd_pcm_channel_area* areas;
  unsigned int                  bufferSize;
  ACE_Message_Queue_Base*       queue;
  unsigned int                  sampleSize;
};

struct Stream_Module_Device_ALSA_Playback_AsynchCBData
{
//  struct _snd_pcm_channel_area* areas;
  ACE_Message_Block*      currentBuffer;
  ACE_Message_Queue_Base* queue;
  unsigned int            sampleSize;
};
#endif

#endif
