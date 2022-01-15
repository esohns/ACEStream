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
// *IMPORTANT NOTE*: this is not what it seems; the value is currently used ONLY
//                   to distinguish between message types (see e.g.
//                   stream_cachedmessageallocatorheap_base.inl:190)
// *TODO*: find an alternative method and implement proper message priorization
#define STREAM_MESSAGE_CONTROL_PRIORITY                     100
#define STREAM_MESSAGE_SESSION_TYPE                         ACE_Message_Block::MB_EVENT

#define STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE             16384 // 16 kB

// queue
// *IMPORTANT NOTE*: any of these can seriously affect application performance
#define STREAM_QUEUE_MAX_SLOTS                              10000
// *IMPORTANT NOTE*: #concurrent in-flight messages, i.e. static heap memory
//                   allocation can be approximated as STREAM_QUEUE_MAX_MESSAGES
//                   * sizeof(stream-message type) bytes
// *IMPORTANT NOTE*: when compressed data is being streamed, the decompressing
//                   module may need to cache messages
//                   --> applications must ensure that there are enough message
//                       buffers to contain at least one decompressed frame,
//                       otherwise the pipeline could lock without progressing
#define STREAM_QUEUE_MAX_MESSAGES                           1000
// *IMPORTANT NOTE*: pre-cached messages (cached allocators only)
#define STREAM_QUEUE_DEFAULT_CACHED_MESSAGES                ACE_DEFAULT_FREE_LIST_HWM

// module
#define STREAM_MODULE_TASK_GROUP_ID                         10
#define STREAM_MODULE_THREAD_NAME                           "stream dispatch"
#define STREAM_MODULE_DEFAULT_HEAD_THREADS                  1

// modules (generic)
#define STREAM_MODULE_HEAD_NAME                             "StreamHead"
#define STREAM_MODULE_TAIL_NAME                             "StreamTail"
#define STREAM_MODULE_DEFAULT_CRUNCH_MESSAGES               false

// state-machine
#define STREAM_STATEMACHINE_CONTROL_NAME                    "StreamStateMachineControl"

// stream
#define STREAM_DEFAULT_MODULE_SOURCE_EVENT_POLL_INTERVAL_MS 10 // ms
// *IMPORTANT NOTE*: this affects waiting on the 'stream head' reader only
//                   --> writer/outbound side for connections
//                   This means that during a stream stop(true), x seconds worth
//                   of outbound data may still be dispatched to the kernel
// *NOTE*: this may well depend on whether the event dispatch is multithreaded
#define STREAM_DEFAULT_STOP_WAIT_HEAD_READER_RETRIES        3 // --> * s

#define STREAM_SUBSTREAM_DECODE_NAME                        "decode"
#define STREAM_SUBSTREAM_DISPLAY_NAME                       "display"
#define STREAM_SUBSTREAM_NETWORK_NAME                       "network"
#define STREAM_SUBSTREAM_PLAYBACK_NAME                      "playback"
#define STREAM_SUBSTREAM_SAVE_NAME                          "save to file"

// statistic
#define STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS     500 // ms [0: off]
#define STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S       0 // second(s) [0: off]

#endif
