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

// device names
//#define STREAM_LIB_ALSA_DEVICE_CAPTURE_PREFIX                     "hw"
#define STREAM_LIB_ALSA_DEVICE_CAPTURE_PREFIX                     "sysdefault"
// *NOTE*: 'sysdefault' references the hardware device --> no simultaneous playback
#define STREAM_LIB_ALSA_DEVICE_PLAYBACK_PREFIX                    "sysdefault"
//#define STREAM_LIB_ALSA_DEVICE_DMIX                               "plug:dmix"
#define STREAM_LIB_ALSA_DEVICE_DEFAULT                            "default"
//#define STREAM_LIB_ALSA_DEVICE_PLUGHW                             "plug:hw"

#define STREAM_LIB_ALSA_PCM_INTERFACE_NAME                        "pcm"

#define STREAM_LIB_ALSA_DEFAULT_ACCESS                            SND_PCM_ACCESS_RW_INTERLEAVED
//#define STREAM_LIB_ALSA_DEFAULT_DEVICE_NAME                       "hw:0,0"
#define STREAM_LIB_ALSA_DEFAULT_FORMAT                            SND_PCM_FORMAT_S16
#define STREAM_LIB_ALSA_DEFAULT_LOG_FILE                          "alsa.log"
#define STREAM_LIB_ALSA_DEFAULT_WAIT_TIMEOUT_MS                   30 // ms

// capture stream
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_ASYNCH                    true
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_SIZE               2048 // bytes (== periodsize * #period)
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_TIME               100000 // us
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME               STREAM_LIB_ALSA_DEVICE_CAPTURE_PREFIX
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_MODE                      SND_PCM_NO_SOFTVOL
// *IMPORTANT NOTE*: "...Latency is directly proportional with the buffer size
//                    on playback devices or the period siz on capture devices..."
// *TODO*: number of frames between each interrupt
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_SIZE               1024 // frames
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_TIME               21333 // us
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIODS                   2

// *TODO*: these belong somewhere else
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_CHANNELS                  2 // i.e. stereo
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_SAMPLE_RATE               48000 // Hz

#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_BOOST_NAME         "Mic Boost"
#define STREAM_LIB_ALSA_CAPTURE_DEFAULT_SELEM_VOLUME_NAME        "Capture"

// playback stream
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_ASYNCH                   true
// *IMPORTANT NOTE*: "...Latency is directly proportional with the buffer size
//                    on playback devices or the period siz on capture devices..."
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_BUFFER_SIZE              12288 // bytes (== periodsize * #period)
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_BUFFER_TIME              500000 // us
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_DEVICE_NAME              STREAM_LIB_ALSA_DEVICE_DEFAULT
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_MODE                     0
// *TODO*: number of frames between each interrupt
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_PERIOD_SIZE              4096 // frames
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_PERIOD_TIME              21333 // us
#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_PERIODS                  3

#define STREAM_LIB_ALSA_PLAYBACK_DEFAULT_SELEM_VOLUME_NAME        "Master"

#endif
