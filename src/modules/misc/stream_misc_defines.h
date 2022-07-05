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

#ifndef STREAM_MISC_DEFINES_H
#define STREAM_MISC_DEFINES_H

#include "ace/config-lite.h"

// module
#define STREAM_MISC_AGGREGATOR_DEFAULT_NAME_STRING                  "Aggregator"
#define STREAM_MISC_DEFRAGMENT_DEFAULT_NAME_STRING                  "Defragment"
#define STREAM_MISC_DELAY_DEFAULT_NAME_STRING                       "Delay"
#define STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING                 "Distributor"
#define STREAM_MISC_DUMP_DEFAULT_NAME_STRING                        "Dump"
#define STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING              "MessageHandler"
// *NOTE*: duplex module consisting of 'parser' / 'streamer'
#define STREAM_MISC_MARSHAL_DEFAULT_NAME_STRING                     "Marshal"
#define STREAM_MISC_PARSER_DEFAULT_NAME_STRING                      "Parser"
#define STREAM_MISC_QUEUE_DEFAULT_NAME_STRING                       "Queue"
#define STREAM_MISC_SPLITTER_DEFAULT_NAME_STRING                    "Splitter"

// stream
#define STREAM_MISC_DEFAULT_INPUT_STREAM_NAME_STRING                "InputStream"

#define STREAM_MISC_DEFAULT_DELAY_AUDIO_INTERVAL_US                 10000 // us

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// DirectShow
#define STREAM_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS                  30 // ==> max. #frames(/sec)

#define STREAM_MISC_DS_WIN32_FILTER_SOURCE_FRAME_INTERVAL           20 // ms
// *NOTE*: if the graph (i.e. usually the renderers'-) (default) allocator
//         supplies the sample buffers (instead of the (source) filter), and the
//         stream message type does not implement IMediaSample, the 'push'
//         strategy (FillBuffer/Receive) involves a(n additional) memcpy of
//         (inbound) frame data
// *NOTE*: the 'pull' strategy is implemented via IAsynchReader (Request/
//         WaitForNext)
#define STREAM_MISC_DS_WIN32_FILTER_SOURCE_DEFAULT_PUSH             false

#define STREAM_MISC_DS_WIN32_ALLOCATOR_NAME                         "ACEStream DirectShow Allocator"
#define STREAM_MISC_DS_WIN32_FILTER_NAME_ASYNCH_SOURCE              "ACEStream DirectShow Asynch Source"
#define STREAM_MISC_DS_WIN32_FILTER_NAME_ASYNCH_SOURCE_L            L"ACEStream DirectShow Asynch Source"
#define STREAM_MISC_DS_WIN32_FILTER_NAME_SOURCE                     "ACEStream DirectShow Source"
#define STREAM_MISC_DS_WIN32_FILTER_NAME_SOURCE_L                   L"ACEStream DirectShow Source"
#define STREAM_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME                 L"Output"

// MediaFoundation
// *NOTE*: #samples each stream tries to hold in its queue
#define STREAM_MISC_MF_WIN32_MEDIA_SOURCE_SAMPLE_QUEUE_SIZE         2;

#define STREAM_MISC_MF_WIN32_BYTESTREAMHANDLER_DESCRIPTION          "ACEStream Source ByteStreamHandler"
#define STREAM_MISC_MF_WIN32_REG_BYTESTREAMHANDLERS_KEY             "Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers"
#endif // ACE_WIN32 || ACE_WIN64

#endif
