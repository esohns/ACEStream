#ifndef STREAM_MODULE_DEV_DEFINES_H
#define STREAM_MODULE_DEV_DEFINES_H

//#define MODULE_DEV_CAM_DEFAULT_CHARACTER_SET   "utf8"
//#define MODULE_DEV_CAM_DEFAULT_PORT            3306
//#define MODULE_DEV_CAM_DEFAULT_TIMEOUT_CONNECT 3 // seconds
//#define MODULE_DEV_CAM_DEFAULT_TIMEOUT_READ    3 // seconds
//#define MODULE_DEV_CAM_DEFAULT_TIMEOUT_WRITE   3 // seconds
//#define MODULE_DEV_CAM_DEFAULT_RECONNECT       true
//#define MODULE_DEV_CAM_DEFAULT_USER            "root"

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_CAPTURE         L"Capture Filter"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_AVI_DECOMPRESS  L"AVI Decompressor"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_MJPG_DECOMPRESS L"MJPG Decompressor"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_GRABBER         L"Sample Grabber"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_NULL_RENDERER   L"Null Renderer"
#define MODULE_DEV_CAM_WIN32_FILTER_NAME_VIDEO_RENDERER  L"Video Renderer"

// user-defined message to notify applications of filtergraph events
#define MODULE_DEV_CAM_UI_WIN32_WM_GRAPHNOTIFY           WM_APP + 1
#else
#define MODULE_DEV_DEVICE_DIRECTORY                      "/dev"
#define MODULE_DEV_DEFAULT_VIDEO_DEVICE                  "video0"

// *NOTE*: (on Linux,) my laptop camera buffers 32 frames...
#define MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS        100
#define MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD             V4L2_MEMORY_USERPTR
#endif

#endif
