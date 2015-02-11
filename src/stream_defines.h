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

// queue / task / buffers
#define STREAM_TASK_GROUP_ID               1000
// *IMPORTANT NOTE*: any of these COULD seriously affect performance
#define STREAM_MAX_QUEUE_SLOTS             10000
// *IMPORTANT NOTE*: static heap memory consumption can probably be approximated
// as STREAM_DEF_MAX_MESSAGES * sizeof(stream-message-type) bytes
#define STREAM_MAX_MESSAGES                1000

#define STREAM_BUFFER_SIZE                 1024 // 1 kB

#define STREAM_DEF_NUM_STREAM_HEAD_THREADS 1
#define STREAM_DEF_HANDLER_THREAD_NAME     "stream dispatch"

#endif
