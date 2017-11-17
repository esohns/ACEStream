#ifndef STREAM_MODULE_DEV_DEFINES_H
#define STREAM_MODULE_DEV_DEFINES_H

#include "ace/config-lite.h"

#define MODULE_DEV_CAM_STATISTIC_COLLECTION_INTERVAL             STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL // ms

/////////////////////////////////////////

#define MODULE_DEV_CAM_SOURCE_DEFAULT_NAME_STRING                "CamSource"
#define MODULE_DEV_MIC_SOURCE_DEFAULT_NAME_STRING                "MicSource"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// DirectShow
// *TODO*: move these somewhere else
#define MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS         60 // ==> max. #frames(/sec)
//#define MODULE_DEV_DIRECTSHOW_FILTER_SOURCE_FRAME_INTERVAL 20 // ms

#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO      L"Capture Audio"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_PCM        L"WAV Converter"

#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO      L"Capture Video"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_RGB        L"Color Space Converter"
//#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_YUV     L"AVI Decoder"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_YUV        L"Color Converter DSP DMO"
// *NOTE*: the 'AVI decompressor' (CLSID_AVIDec) supports conversions of YUV
//         to RGB formats via the MSYUV Color Space Converter Codec
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI     L"AVI Decompressor"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264    L"H264 Decompressor"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG    L"MJPG Decompressor"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_GRAB               L"Sample Grabber"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO       L"Video Renderer"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RESIZER_VIDEO      L"Video Resizer DSP DMO"
#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_SPLIT_AVI          L"AVI Splitter"

#define MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO       L"Audio Effect"
#define MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO       L"Audio Renderer"

#define MODULE_DEV_DIRECTSHOW_FILTER_NAME_RENDER_NULL            L"Null Renderer"

#define MODULE_DEV_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT       MEDIASUBTYPE_RGB32
// *IMPORTANT NOTE*: "...When the Video Renderer draws to a DirectDraw overlay
//                   surface, it allocates a single buffer for its input pin. If
//                   the upstream filter attempts to force a connection using
//                   multiple buffers, the Video Renderer will be unable to use
//                   the overlay surface. ..."
#define MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER          CLSID_VideoRenderer
//#define MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX7      CLSID_VideoMixingRenderer
#define MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX7      CLSID_VideoRendererDefault
#define MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX9      CLSID_VideoMixingRenderer9
#define MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_ENHANCED CLSID_EnhancedVideoRenderer
#if (_WIN32_WINNT < _WIN32_WINNT_WINXP)
#define MODULE_DEV_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER  MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER
#elif (_WIN32_WINNT < _WIN32_WINNT_VISTA)
#if defined (VMR9_SUPPORT)
#define MODULE_DEV_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER  MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX9
#else
#define MODULE_DEV_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER  MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX7
#endif
#else
#define MODULE_DEV_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER  MODULE_DEV_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_ENHANCED
#endif

// properties
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING      L"Description"
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING             L"DevicePath"
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING             L"FriendlyName"
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_ID_STRING               L"WaveInID"

// Direct3D
#define MODULE_DEV_CAM_DIRECT3D_DEFAULT_BACK_BUFFERS             2

#else
// ALSA
#define MODULE_DEV_ALSA_TARGET_DEFAULT_NAME_STRING               "ALSATarget"

#define MODULE_DEV_ALSA_DEVICE_CAPTURE_PREFIX                    "hw"
//#define MODULE_DEV_ALSA_DEVICE_CAPTURE_PREFIX               "dsnoop"
#define MODULE_DEV_ALSA_DEVICE_PLAYBACK_PREFIX                   "plughw"
//#define MODULE_DEV_ALSA_DEVICE_PLAYBACK_PREFIX              "dmix"
#define MODULE_DEV_ALSA_PCM_INTERFACE_NAME                       "pcm"

#define MODULE_DEV_ALSA_DEFAULT_LOG_FILE                         "alsa.log"

#define MODULE_DEV_MIC_ALSA_DEFAULT_ACCESS                       SND_PCM_ACCESS_RW_INTERLEAVED
#define MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_SIZE                  128 // frames
#define MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_TIME                  999 // us
#define MODULE_DEV_MIC_ALSA_DEFAULT_CHANNELS                     2
//#define MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME             "default"
#define MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME                  "hw:0,0"
#define MODULE_DEV_MIC_ALSA_DEFAULT_FORMAT                       SND_PCM_FORMAT_S16
#define MODULE_DEV_MIC_ALSA_DEFAULT_MODE                         SND_PCM_ASYNC
// *TODO*: number of frames between each interrupt
#define MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_SIZE                  32 // frames
#define MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_TIME                  333 // us
#define MODULE_DEV_MIC_ALSA_DEFAULT_PERIODS                      32
#define MODULE_DEV_MIC_ALSA_DEFAULT_SAMPLE_RATE                  44100

// general
#define MODULE_DEV_DEVICE_DIRECTORY                              "/dev"

#define MODULE_DEV_DEFAULT_AUDIO_DEVICE                          "dsp"
#define MODULE_DEV_DEFAULT_VIDEO_DEVICE                          "video0"

// V4L
// *NOTE*: (on Linux,) a Lenovo (TM) ThinkPad T410 integrated camera buffers 32
//         frames
#define MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS                60
#define MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD                     V4L2_MEMORY_USERPTR

#endif

#endif
