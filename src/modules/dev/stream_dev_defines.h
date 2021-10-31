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
#define STREAM_DEV_MIC_SOURCE_WASAPI_DEFAULT_NAME_STRING          "WASAPIMicSource"
#define STREAM_DEV_MIC_SOURCE_WAVEIN_DEFAULT_NAME_STRING          "WaveInMicSource"

#define STREAM_DEV_TARGET_WAVOUT_DEFAULT_NAME_STRING              "WavOutPlayback"
#else
#define STREAM_DEV_CAM_SOURCE_LIBCAMERA_DEFAULT_NAME_STRING       "libCameraCamSource"
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
#else
// ALSA
#define STREAM_DEV_MIC_ALSA_DEFAULT_MODE                          SND_PCM_NO_SOFTVOL

// general
#define STREAM_DEV_DEVICE_DIRECTORY                               "/dev"

#define STREAM_DEV_DEFAULT_AUDIO_DEVICE                           "dsp"
#define STREAM_DEV_DEFAULT_VIDEO_DEVICE                           "video0"
#endif // ACE_WIN32 || ACE_WIN64

// general
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT                480
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH                 640
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_RATE                       30 // fps

#define STREAM_DEV_MIC_DEFAULT_BITS_PER_SAMPLE                    16
#define STREAM_DEV_MIC_DEFAULT_CHANNELS                           2 // i.e. stereo
#define STREAM_DEV_MIC_DEFAULT_SAMPLE_RATE                        44100 // Hz

#endif
