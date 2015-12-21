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
// user-defined message to notify applications of filtergraph events 
#define MODULE_DEV_CAM_UI_WIN32_WM_GRAPHNOTIFY   WM_APP + 1
#endif

#endif
