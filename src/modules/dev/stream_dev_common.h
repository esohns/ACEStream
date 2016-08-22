#ifndef STREAM_MODULE_DEV_COMMON_H
#define STREAM_MODULE_DEV_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <map>

#include "linux/videodev2.h"

// forward declarations
class ACE_Message_Block;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef std::map<__u32, ACE_Message_Block*> Stream_Module_Device_BufferMap_t;
typedef Stream_Module_Device_BufferMap_t::const_iterator Stream_Module_Device_BufferMapIterator_t;
#endif

enum Stream_Module_Device_Mode
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_MODULE_DEV_MODE_DIRECTSHOW,
  STREAM_MODULE_DEV_MODE_MEDIAFOUNDATION,
#else
  STREAM_MODULE_DEV_MODE_V4L2 = 0,
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

#endif
