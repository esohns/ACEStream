#ifndef STREAM_MODULE_DEV_COMMON_H
#define STREAM_MODULE_DEV_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <map>

#include "ace/Message_Block.h"

#include "linux/videodev2.h"
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef std::map<__u32, ACE_Message_Block*> INDEX2BUFFER_MAP_T;
typedef INDEX2BUFFER_MAP_T::const_iterator INDEX2BUFFER_MAP_ITERATOR_T;
#endif

//struct Stream_Module_Device_CamOptions
//{
//  inline Stream_Module_Device_CamOptions ()
//  {};

//};

#endif
