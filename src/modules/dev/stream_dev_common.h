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

#include "stream_dev_defines.h"

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
struct Stream_Module_Device_ALSAConfiguration
{
  inline Stream_Module_Device_ALSAConfiguration ()
   : access (MODULE_DEV_MIC_ALSA_DEFAULT_ACCESS)
   , format (MODULE_DEV_MIC_ALSA_DEFAULT_FORMAT)
   , subFormat (SND_PCM_SUBFORMAT_STD)
   , channels (MODULE_DEV_MIC_ALSA_DEFAULT_CHANNELS)
   , rate (MODULE_DEV_MIC_ALSA_DEFAULT_SAMPLE_RATE)
   , periodTime (MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_TIME)
   , periodSize (MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_SIZE)
   , periods (MODULE_DEV_MIC_ALSA_DEFAULT_PERIODS)
   , bufferTime (MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_TIME)
   , bufferSize (MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_SIZE)
  {};

  enum _snd_pcm_access    access;
  enum _snd_pcm_format    format;
  enum _snd_pcm_subformat subFormat;
  unsigned int            channels;
  unsigned int            rate;
  unsigned int            periodTime;
  snd_pcm_uframes_t       periodSize;
  unsigned int            periods;
  unsigned int            bufferTime;
  snd_pcm_uframes_t       bufferSize;
};

struct Stream_Module_Device_ALSA_Capture_AsynchCBData
{
  Stream_IAllocator*            allocator;
//  struct _snd_pcm_channel_area* areas;
  unsigned int                  bufferSize;
  unsigned int                  channels;
  enum _snd_pcm_format          format;
  ACE_Message_Queue_Base*       queue;
  unsigned int                  sampleRate;
  unsigned int                  sampleSize;

  double                        frequency;
  bool                          sinus;
  double                        phase;
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
