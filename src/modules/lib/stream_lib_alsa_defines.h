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

#ifndef STREAM_LIB_ALSA_DEFINES_H
#define STREAM_LIB_ALSA_DEFINES_H

//#define STREAM_LIB_ALSA_DEVICE_CAPTURE_PREFIX                     "dsnoop"
//#define STREAM_LIB_ALSA_DEVICE_CAPTURE_PREFIX                     "hw"
#define STREAM_LIB_ALSA_DEVICE_CAPTURE_PREFIX                     "sysdefault"
//#define STREAM_LIB_ALSA_DEVICE_PLAYBACK_PREFIX                    "dmix"
//#define STREAM_LIB_ALSA_DEVICE_PLAYBACK_PREFIX                    "plughw"
#define STREAM_LIB_ALSA_DEVICE_PLAYBACK_PREFIX                    "default"
#define STREAM_LIB_ALSA_PCM_INTERFACE_NAME                        "pcm"

#define STREAM_LIB_ALSA_DEFAULT_LOG_FILE                          "alsa.log"

#define STREAM_LIB_MIC_ALSA_DEFAULT_ACCESS                        SND_PCM_ACCESS_RW_INTERLEAVED
#define STREAM_LIB_MIC_ALSA_DEFAULT_BUFFER_SIZE                   128 // frames
#define STREAM_LIB_MIC_ALSA_DEFAULT_BUFFER_TIME                   999 // us
//#define STREAM_LIB_MIC_ALSA_DEFAULT_DEVICE_NAME             "default"
#define STREAM_LIB_MIC_ALSA_DEFAULT_DEVICE_NAME                   "hw:0,0"
#define STREAM_LIB_MIC_ALSA_DEFAULT_FORMAT                        SND_PCM_FORMAT_S16
#define STREAM_LIB_MIC_ALSA_DEFAULT_MODE                          SND_PCM_ASYNC
// *TODO*: number of frames between each interrupt
#define STREAM_LIB_MIC_ALSA_DEFAULT_PERIOD_SIZE                   32 // frames
#define STREAM_LIB_MIC_ALSA_DEFAULT_PERIOD_TIME                   333 // us
#define STREAM_LIB_MIC_ALSA_DEFAULT_PERIODS                       32

#endif
