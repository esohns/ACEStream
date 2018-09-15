#ifndef STREAM_MODULE_DEV_COMMON_H
#define STREAM_MODULE_DEV_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <list>
#include <string>

#include <d3d9.h>
#include <d3d9types.h>
#include <strmif.h>

#include "ace/OS.h"
#else
#include <map>

extern "C"
{
#include <alsa/asoundlib.h>
}
#include <linux/videodev2.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_defines.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_IAllocator;
struct Stream_Statistic;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef std::list<std::string> Stream_Module_Device_List_t;
typedef Stream_Module_Device_List_t::const_iterator Stream_Module_Device_ListIterator_t;
#else
typedef std::map<__u32, ACE_Message_Block*> Stream_Module_Device_BufferMap_t;
typedef Stream_Module_Device_BufferMap_t::const_iterator Stream_Module_Device_BufferMapIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

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
//  inline Stream_Module_Device_CamOptions () {}

//};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_Module_Device_Direct3DConfiguration
{
  Stream_Module_Device_Direct3DConfiguration ()
   : adapter (D3DADAPTER_DEFAULT)
   , behaviorFlags (//D3DCREATE_ADAPTERGROUP_DEVICE          |
                    //D3DCREATE_DISABLE_DRIVER_MANAGEMENT    |
                    //D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX |
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                    //D3DCREATE_DISABLE_PRINTSCREEN          |
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                    //D3DCREATE_DISABLE_PSGP_THREADING       |
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
#if defined (_DEBUG)
                    D3DCREATE_ENABLE_PRESENTSTATS          |
#endif // _DEBUG
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                    D3DCREATE_FPU_PRESERVE                 |
                    //D3DCREATE_HARDWARE_VERTEXPROCESSING    |
                    //D3DCREATE_MIXED_VERTEXPROCESSING       |
                    //D3DCREATE_SOFTWARE_VERTEXPROCESSING    |
                    D3DCREATE_MULTITHREADED)//                |
                    //D3DCREATE_NOWINDOWCHANGES              |
                    //D3DCREATE_PUREDEVICE                   |
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                    D3DCREATE_SCREENSAVER)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
   , deviceType (D3DDEVTYPE_HAL)
   , focusWindow (NULL)
   , presentationParameters ()
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
   , fullScreenDisplayMode ()
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
   //, usage (0)
  {
    ACE_OS::memset (&presentationParameters,
                    0,
                    sizeof (struct _D3DPRESENT_PARAMETERS_));
    //presentationParameters.BackBufferWidth = 0;
    //presentationParameters.BackBufferHeight = 0;
    presentationParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
    presentationParameters.BackBufferCount = D3DPRESENT_BACK_BUFFERS_MAX;
    presentationParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
    //presentationParameters.MultiSampleQuality = 0;
    presentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    //presentationParameters.hDeviceWindow = NULL;
    presentationParameters.Windowed = true;
    presentationParameters.EnableAutoDepthStencil = true;
    presentationParameters.AutoDepthStencilFormat = D3DFMT_D16;
    presentationParameters.Flags =
      (D3DPRESENTFLAG_DEVICECLIP           | // "not valid with D3DSWAPEFFECT_FLIPEX"
       D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL | // "illegal for all lockable formats"
       D3DPRESENTFLAG_LOCKABLE_BACKBUFFER  |
       //D3DPRESENTFLAG_NOAUTOROTATE         |
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
       D3DPRESENTFLAG_UNPRUNEDMODE         |
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
       D3DPRESENTFLAG_VIDEO);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
       //D3DPRESENTFLAG_OVERLAY_LIMITEDRGB   |
       //D3DPRESENTFLAG_OVERLAY_YCbCr_BT709  |
       //D3DPRESENTFLAG_OVERLAY_YCbCr_xvYCC  |
       //D3DPRESENTFLAG_RESTRICTED_CONTENT   |
       //D3DPRESENTFLAG_RESTRICT_SHARED_RESOURCE_DRIVER);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    //presentationParameters.FullScreen_RefreshRateInHz = 0;
    // *NOTE*: to prevent tearing: D3DPRESENT_INTERVAL_DEFAULT (i.e. 'vSync')
    presentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_OS::memset (&fullScreenDisplayMode,
                    0,
                    sizeof (struct D3DDISPLAYMODEEX));
    fullScreenDisplayMode.Size = sizeof (struct D3DDISPLAYMODEEX);
    //fullScreenDisplayMode.Width = 0;
    //fullScreenDisplayMode.height = 0;
    //fullScreenDisplayMode.RefreshRate = 0;
    fullScreenDisplayMode.Format = D3DFMT_UNKNOWN;
    fullScreenDisplayMode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

    //usage = (//D3DUSAGE_AUTOGENMIPMAP      |
    //         D3DUSAGE_DEPTHSTENCIL    |
    //         D3DUSAGE_DMAP            |
    //         //D3DUSAGE_DONOTCLIP          |
    //         D3DUSAGE_DYNAMIC         |
    //         D3DUSAGE_NONSECURE       |
    //         //D3DUSAGE_NPATCHES           |
    //         //D3DUSAGE_POINTS             |
    //         D3DUSAGE_RENDERTARGET    |
    //         //D3DUSAGE_RTPATCHES          |
    //         //D3DUSAGE_SOFTWAREPROCESSING |
    //         //D3DUSAGE_TEXTAPI            |
    //         D3DUSAGE_WRITEONLY);//       |
    //         //D3DUSAGE_RESTRICTED_CONTENT |
    //         //D3DUSAGE_RESTRICT_SHARED_RESOURCE |
    //         //D3DUSAGE_RESTRICT_SHARED_RESOURCE_DRIVER);
  }

  UINT                           adapter;
  DWORD                          behaviorFlags; // see also: D3DCREATE
  enum _D3DDEVTYPE               deviceType;
  HWND                           focusWindow;
  struct _D3DPRESENT_PARAMETERS_ presentationParameters;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct D3DDISPLAYMODEEX        fullScreenDisplayMode;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  //DWORD                          usage; // see also: D3DUSAGE
};
#else
struct Stream_Module_Device_ALSAConfiguration
{
  Stream_Module_Device_ALSAConfiguration ()
   : access (MODULE_DEV_MIC_ALSA_DEFAULT_ACCESS)
   , bufferSize (MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_SIZE)
   , bufferTime (MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_TIME)
   , format (MODULE_DEV_MIC_ALSA_DEFAULT_FORMAT)
   , subFormat (SND_PCM_SUBFORMAT_STD)
   , channels (MODULE_DEV_MIC_ALSA_DEFAULT_CHANNELS)
   , periods (MODULE_DEV_MIC_ALSA_DEFAULT_PERIODS)
   , periodSize (MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_SIZE)
   , periodTime (MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_TIME)
   , rate (MODULE_DEV_MIC_ALSA_DEFAULT_SAMPLE_RATE)
  {}

  enum _snd_pcm_access    access;
  snd_pcm_uframes_t       bufferSize;
  unsigned int            bufferTime;
  enum _snd_pcm_format    format;
  enum _snd_pcm_subformat subFormat;
  unsigned int            channels;
  unsigned int            periods;
  snd_pcm_uframes_t       periodSize;
  unsigned int            periodTime;
  unsigned int            rate;
};

struct Stream_Module_Device_ALSA_Capture_AsynchCBData
{
  Stream_IAllocator*            allocator;
  Stream_Statistic*             statistic;

  //  struct _snd_pcm_channel_area* areas;
  unsigned int                  bufferSize;
  unsigned int                  channels;
  enum _snd_pcm_format          format;
  ACE_Message_Queue_Base*       queue;
  unsigned int                  sampleRate;
  unsigned int                  sampleSize;

  double*                       frequency;
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
#endif // ACE_WIN32 || ACE_WIN64

#endif
