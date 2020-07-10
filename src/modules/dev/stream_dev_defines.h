#ifndef STREAM_DEV_DEFINES_H
#define STREAM_DEV_DEFINES_H

#include "ace/config-lite.h"

#define STREAM_DEV_CAM_STATISTIC_COLLECTION_INTERVAL              STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL // ms

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING      "DirectShowCamSource"
#define STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING "MediaFoundationCamSource"

#define STREAM_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING      "DirectShowMicSource"
#define STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING "MediaFoundationMicSource"
#define STREAM_DEV_MIC_SOURCE_WAVEIN_DEFAULT_NAME_STRING          "WaveInMicSource"

#define STREAM_DEV_TARGET_WAVOUT_DEFAULT_NAME_STRING              "WavOutPlayback"
#else
#define STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING             "V4LCamSource"

#define STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING            "ALSAMicSource"
#define STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING                "ALSAPlayback"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// DirectShow
// *TODO*: move these somewhere else
#define STREAM_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS          60 // ==> max. #frames(/sec)
#define STREAM_DEV_MIC_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS          10
#define STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS              10
//#define STREAM_DEV_DIRECTSHOW_FILTER_SOURCE_FRAME_INTERVAL 20 // ms

#define STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO       L"Capture Video"

#define STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO       L"Capture Audio"

// properties
#define STREAM_DEV_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING       L"Description"
#define STREAM_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING              L"DevicePath"
#define STREAM_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING              L"FriendlyName"
#define STREAM_DEV_DIRECTSHOW_PROPERTIES_ID_STRING                L"WaveInID"
#else
// ALSA
#define STREAM_DEV_ALSA_TARGET_DEFAULT_NAME_STRING                "ALSATarget"

//#define STREAM_DEV_ALSA_DEVICE_CAPTURE_PREFIX                     "dsnoop"
//#define STREAM_DEV_ALSA_DEVICE_CAPTURE_PREFIX                     "hw"
#define STREAM_DEV_ALSA_DEVICE_CAPTURE_PREFIX                     "sysdefault"
//#define STREAM_DEV_ALSA_DEVICE_PLAYBACK_PREFIX                    "dmix"
//#define STREAM_DEV_ALSA_DEVICE_PLAYBACK_PREFIX                    "plughw"
#define STREAM_DEV_ALSA_DEVICE_PLAYBACK_PREFIX                    "default"
#define STREAM_DEV_ALSA_PCM_INTERFACE_NAME                        "pcm"

#define STREAM_DEV_ALSA_DEFAULT_LOG_FILE                          "alsa.log"

#define STREAM_DEV_MIC_ALSA_DEFAULT_ACCESS                        SND_PCM_ACCESS_RW_INTERLEAVED
#define STREAM_DEV_MIC_ALSA_DEFAULT_BUFFER_SIZE                   128 // frames
#define STREAM_DEV_MIC_ALSA_DEFAULT_BUFFER_TIME                   999 // us
//#define STREAM_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME             "default"
#define STREAM_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME                   "hw:0,0"
#define STREAM_DEV_MIC_ALSA_DEFAULT_FORMAT                        SND_PCM_FORMAT_S16
#define STREAM_DEV_MIC_ALSA_DEFAULT_MODE                          SND_PCM_ASYNC
// *TODO*: number of frames between each interrupt
#define STREAM_DEV_MIC_ALSA_DEFAULT_PERIOD_SIZE                   32 // frames
#define STREAM_DEV_MIC_ALSA_DEFAULT_PERIOD_TIME                   333 // us
#define STREAM_DEV_MIC_ALSA_DEFAULT_PERIODS                       32

// general
#define STREAM_DEV_DEVICE_DIRECTORY                               "/dev"

#define STREAM_DEV_DEFAULT_AUDIO_DEVICE                           "dsp"
#define STREAM_DEV_DEFAULT_VIDEO_DEVICE                           "video0"

// V4L
// *NOTE*: (on Linux,) a Lenovo (TM) ThinkPad T410 integrated camera buffers 32
//         frames
#define STREAM_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS                 32
//#define STREAM_DEV_CAM_V4L_DEFAULT_IO_METHOD                      V4L2_MEMORY_USERPTR
#define STREAM_DEV_CAM_V4L_DEFAULT_IO_METHOD                      V4L2_MEMORY_MMAP

#define STREAM_DEV_CAM_V4L_DEFAULT_PIXELFORMAT                    V4L2_PIX_FMT_RGB24
#endif // ACE_WIN32 || ACE_WIN64

#define STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT                480
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH                 640
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_RATE                       30 // fps

#define STREAM_DEV_MIC_DEFAULT_CHANNELS                           2 // i.e. stereo
#define STREAM_DEV_MIC_DEFAULT_SAMPLE_RATE                        44100 // Hz

#endif
