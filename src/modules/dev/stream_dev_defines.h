#ifndef STREAM_DEV_DEFINES_H
#define STREAM_DEV_DEFINES_H

#include "ace/config-lite.h"

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// Video
#define STREAM_DEV_CAM_SOURCE_VIDEOFORWINDOW_DEFAULT_NAME_STRING  "VfW_CamSource"
#define STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING      "DirectShow_CamSource"
#define STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING "MediaFoundation_CamSource"

// Audio
#define STREAM_DEV_MIC_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING      "DirectShow_MicSource"
#define STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING "MediaFoundation_MicSource"

#define STREAM_DEV_WASAPI_CAPTURE_DEFAULT_NAME_STRING             "WASAPICapture"
#define STREAM_DEV_WASAPI_RENDER_DEFAULT_NAME_STRING              "WASAPIRender"

#define STREAM_DEV_WAVEIN_CAPTURE_DEFAULT_NAME_STRING             "WaveInCapture"
#define STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING             "WaveOutRender"
#else
#define STREAM_DEV_CAM_SOURCE_LIBCAMERA_DEFAULT_NAME_STRING       "libCameraCamSource"
#define STREAM_DEV_CAM_SOURCE_PIPEWIRE_DEFAULT_NAME_STRING        "PipewireCamSource"
#define STREAM_DEV_CAM_SOURCE_V4L_DEFAULT_NAME_STRING             "V4LCamSource"

#define STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING            "ALSAMicSource"
#define STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING                "ALSAPlayback"
#define STREAM_DEV_MIC_SOURCE_PIPEWIRE_DEFAULT_NAME_STRING        "PipewireMicSource"
#define STREAM_DEV_TARGET_PIPEWIRE_DEFAULT_NAME_STRING            "PipewirePlayback"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// VfW
#define STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_DEVICE_BUFFERS      30
#define STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_WINDOW_NAME_STRING  "ACEStream VfW Window"
// capture in preview mode ? : 'overlay'
#define STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_PREVIEW_MODE        false

// DirectShow
// *TODO*: move these somewhere else
#define STREAM_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS          30
#define STREAM_DEV_MIC_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS          30
//#define STREAM_DEV_DIRECTSHOW_FILTER_SOURCE_FRAME_INTERVAL 20 // ms

// WaveIn/Out
#define STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS                  15
#define STREAM_DEV_WAVEOUT_DEFAULT_DEVICE_BUFFERS                 5

#define STREAM_DEV_AUDIO_DEFAULT_RENDERER                         STREAM_DEVICE_RENDERER_WASAPI
#else
// general
#define STREAM_DEV_DEVICE_DIRECTORY                               "/dev"

#define STREAM_DEV_DEFAULT_AUDIO_DEVICE                           "dsp"
#define STREAM_DEV_DEFAULT_VIDEO_DEVICE                           "video0"

#define STREAM_DEV_PIPEWIRE_DEFAULT_POD_BUFFER_SIZE               4096 // bytes

#define STREAM_DEV_AUDIO_DEFAULT_RENDERER                         STREAM_DEVICE_RENDERER_ALSA
#endif // ACE_WIN32 || ACE_WIN64

// *TODO*: move all of this somewhere else ASAP
// general
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT                480
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH                 640
#define STREAM_DEV_CAM_DEFAULT_CAPTURE_FRAME_RATE                 30 // fps

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_DEV_MIC_DEFAULT_FORMAT                             WAVE_FORMAT_PCM
#else
#endif // ACE_WIN32 || ACE_WIN64
#define STREAM_DEV_MIC_DEFAULT_BITS_PER_SAMPLE                    16
#define STREAM_DEV_MIC_DEFAULT_CHANNELS                           2 // i.e. stereo
#define STREAM_DEV_MIC_DEFAULT_SAMPLE_RATE                        48000 // Hz

#endif
