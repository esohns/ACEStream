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

#ifndef STREAM_DEFINES_H
#define STREAM_DEFINES_H

// message
#define STREAM_MESSAGE_DATA_BUFFER_SIZE                  1024 // 1 kB
// *IMPORTANT NOTE*: this is not what it seems; the value is currently used ONLY
//                   to distinguish between message types (see e.g.
//                   stream_cachedmessageallocator.cpp:172)
// *TODO*: find an alternative method and implement proper message priorization
#define STREAM_MESSAGE_CONTROL_PRIORITY                  100

// queue
// *IMPORTANT NOTE*: any of these can seriously affect application performance
#define STREAM_QUEUE_MAX_SLOTS                           10000
// *IMPORTANT NOTE*: #concurrent in-flight messages, i.e. static heap memory
//                   allocation can be approximated as STREAM_QUEUE_MAX_MESSAGES
//                   * sizeof(stream-message type) bytes
// *IMPORTANT NOTE*: when compressed data is being streamed, the decompressing
//                   module may need to cache messages
//                   --> applications must ensure that there are enough message
//                       buffers to contain at least one decompressed frame,
//                       otherwise the pipeline could lock without progressing
#define STREAM_QUEUE_MAX_MESSAGES                        1000
// *IMPORTANT NOTE*: pre-cached messages (cached allocators only)
#define STREAM_QUEUE_DEFAULT_CACHED_MESSAGES             1000

// module
#define STREAM_MODULE_TASK_GROUP_ID                      10
#define STREAM_MODULE_DEFAULT_HEAD_THREADS               1
#define STREAM_MODULE_THREAD_NAME                        "stream dispatch"

// modules (generic)
#define STREAM_MODULE_DEFAULT_CRUNCH_MESSAGES            false
#define STREAM_MODULE_HEAD_NAME                          "StreamHead"

// stream
#define STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL     500 // ms [0: off]
#define STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL      0 // second(s) [0: off]

#define STREAM_DEFAULT_MODULE_SOURCE_EVENT_POLL_INTERVAL 10 // ms

#endif
