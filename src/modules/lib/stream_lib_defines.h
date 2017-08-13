/***************************************************************************
*   Copyright (C) 2009 by Erik Sohns   *
*   erik.sohns@web.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef STREAM_LIB_DEFINES_H
#define STREAM_LIB_DEFINES_H

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_LIB_DEFAULT_MEDIAFRAMEWORK                          STREAM_MEDIAFRAMEWORK_DIRECTSHOW

// DirectShow
#define MODULE_LIB_DIRECTSHOW_FILTER_SOURCE_BUFFERS                30 // ==> max. #frames(/sec)

#define MODULE_LIB_DIRECTSHOW_FILTER_SOURCE_FRAME_INTERVAL         20 // ms
// *NOTE*: if the graph (i.e. usually the renderers'-) (default) allocator
//         supplies the sample buffers (instead of the (source) filter), and the
//         stream message type does not implement IMediaSample, the 'push'
//         strategy (FillBuffer/Receive) involves a(n additional) memcpy of
//         (inbound) frame data
// *NOTE*: the 'pull' strategy is implemented via IAsynchReader (Request/
//         WaitForNext)
#define MODULE_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH           false

#define MODULE_LIB_DIRECTSHOW_ALLOCATOR_NAME                       "ACEStream DirectShow Allocator"

#define MODULE_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE            "ACEStream DirectShow Asynch Source"
#define MODULE_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L          L"ACEStream DirectShow Asynch Source"
#define MODULE_LIB_DIRECTSHOW_FILTER_NAME_SOURCE                   "ACEStream DirectShow Source"
#define MODULE_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L                 L"ACEStream DirectShow Source"
#define MODULE_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME               L"Output"

#define MODULE_LIB_DIRECTSHOW_LOGFILE_NAME                         "ACEStream_DirectShow.log"

// user-defined message to notify applications of filtergraph events
#define MODULE_LIB_DIRECTSHOW_WM_GRAPHNOTIFY_EVENT                 WM_APP + 1

// MediaFoundation
// *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
//         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
//         --> (try to) wait for the next MESessionTopologySet event
#define MODULE_LIB_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT            10 // seconds

// *NOTE*: #samples each stream tries to hold in its queue
#define MODULE_LIB_MEDIAFOUNDATION_MEDIASOURCE_SAMPLE_QUEUE_SIZE   2;

#define MODULE_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_DESCRIPTION   "ACEStream Source ByteStreamHandler"
#define MODULE_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_ROOTKEY       "Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers"

#endif

#endif
