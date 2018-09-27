#ifndef STREAM_MODULE_DEV_DEFINES_H
#define STREAM_MODULE_DEV_DEFINES_H

#include "ace/config-lite.h"

#define MODULE_DEV_CAM_STATISTIC_COLLECTION_INTERVAL              STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL // ms

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING      "DirectShowCamSource"
#define MODULE_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING "MediaFoundationCamSource"

#define MODULE_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING      "DirectShowMicSource"
#define MODULE_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING "MediaFoundationMicSource"
#else
#define MODULE_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING             "V4LCamSource"

#define MODULE_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING            "ALSAMicSource"
#define MODULE_DEV_TARGET_ALSA_DEFAULT_NAME_STRING                "ALSAPlayback"
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// DirectShow
// *TODO*: move these somewhere else
#define MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS          60 // ==> max. #frames(/sec)
//#define MODULE_DEV_DIRECTSHOW_FILTER_SOURCE_FRAME_INTERVAL 20 // ms

#define MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO       L"Capture Video"

#define MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO       L"Capture Audio"

// properties
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING       L"Description"
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING              L"DevicePath"
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING              L"FriendlyName"
#define MODULE_DEV_DIRECTSHOW_PROPERTIES_ID_STRING                L"WaveInID"
#else
// ALSA
#define MODULE_DEV_ALSA_TARGET_DEFAULT_NAME_STRING                "ALSATarget"

#define MODULE_DEV_ALSA_DEVICE_CAPTURE_PREFIX                     "hw"
//#define MODULE_DEV_ALSA_DEVICE_CAPTURE_PREFIX               "dsnoop"
#define MODULE_DEV_ALSA_DEVICE_PLAYBACK_PREFIX                    "plughw"
//#define MODULE_DEV_ALSA_DEVICE_PLAYBACK_PREFIX              "dmix"
#define MODULE_DEV_ALSA_PCM_INTERFACE_NAME                        "pcm"

#define MODULE_DEV_ALSA_DEFAULT_LOG_FILE                          "alsa.log"

#define MODULE_DEV_MIC_ALSA_DEFAULT_ACCESS                        SND_PCM_ACCESS_RW_INTERLEAVED
#define MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_SIZE                   128 // frames
#define MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_TIME                   999 // us
#define MODULE_DEV_MIC_ALSA_DEFAULT_CHANNELS                      2
//#define MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME             "default"
#define MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME                   "hw:0,0"
#define MODULE_DEV_MIC_ALSA_DEFAULT_FORMAT                        SND_PCM_FORMAT_S16
#define MODULE_DEV_MIC_ALSA_DEFAULT_MODE                          SND_PCM_ASYNC
// *TODO*: number of frames between each interrupt
#define MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_SIZE                   32 // frames
#define MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_TIME                   333 // us
#define MODULE_DEV_MIC_ALSA_DEFAULT_PERIODS                       32
#define MODULE_DEV_MIC_ALSA_DEFAULT_SAMPLE_RATE                   44100

// general
#define MODULE_DEV_DEVICE_DIRECTORY                               "/dev"

#define MODULE_DEV_DEFAULT_AUDIO_DEVICE                           "dsp"
#define MODULE_DEV_DEFAULT_VIDEO_DEVICE                           "video0"

// V4L
// *NOTE*: (on Linux,) a Lenovo (TM) ThinkPad T410 integrated camera buffers 32
//         frames
#define MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS                 60
#define MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD                      V4L2_MEMORY_USERPTR

#endif

#endif
