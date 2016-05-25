#ifndef STREAM_MODULE_DEV_DEFINES_H
#define STREAM_MODULE_DEV_DEFINES_H

#include "ace/config-lite.h"

#define MODULE_DEV_CAM_STATISTIC_COLLECTION_INTERVAL        STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL // ms

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// DirectShow
// *TODO*: move these somewhere else
#define MODULE_DEV_DIRECTSHOW_LOGFILE_NAME                  "directshow.log"
#define MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS    60 // ==> max. #frames(/sec)
#define MODULE_DEV_CAM_MEDIAFOUNDATION_DEFAULT_BACK_BUFFERS 2
//#define MODULE_DEV_DIRECTSHOW_FILTER_SOURCE_FRAME_INTERVAL 20 // ms

#define MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE_VIDEO      L"Capture Video"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_RGB        L"Color Space Converter"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_CONVERT_YUV        L"AVI Decoder"
// *NOTE*: the 'AVI decompressor' (CLSID_AVIDec) supports conversions of YUV
//         to RGB formats via the MSYUV Color Space Converter Codec
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_DECOMPRESS_AVI     L"AVI Decompressor"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_DECOMPRESS_MJPG    L"MJPG Decompressor"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_GRAB               L"Sample Grabber"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_NULL        L"Null Renderer"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_RENDER_VIDEO       L"Video Renderer"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_SPLIT_AVI          L"AVI Splitter"

// user-defined message to notify applications of filtergraph events
#define MODULE_DEV_CAM_UI_WIN32_WM_GRAPHNOTIFY              WM_APP + 1
#else
#define MODULE_DEV_DEVICE_DIRECTORY                         "/dev"
#define MODULE_DEV_DEFAULT_VIDEO_DEVICE                     "video0"

// *NOTE*: (on Linux,) my laptop camera buffers 32 frames...
#define MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS           60
#define MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD                V4L2_MEMORY_USERPTR
#endif

#endif
