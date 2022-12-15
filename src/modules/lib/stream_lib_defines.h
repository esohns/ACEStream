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

#define STREAM_LIB_TAGGER_DEFAULT_NAME_STRING                          "Tagger"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_LIB_DIRECTSHOW_SOURCE_DEFAULT_NAME_STRING               "DirectShowSource"
#define STREAM_LIB_DIRECTSHOW_TARGET_DEFAULT_NAME_STRING               "DirectShowTarget"

#define STREAM_LIB_MEDIAFOUNDATION_SOURCE_DEFAULT_NAME_STRING          "MediaFoundationSource"
#define STREAM_LIB_MEDIAFOUNDATION_TARGET_DEFAULT_NAME_STRING          "MediaFoundationTarget"

#define STREAM_LIB_DEFAULT_MEDIAFRAMEWORK                              STREAM_MEDIAFRAMEWORK_DIRECTSHOW

// DirectDraw
#define STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT                        D3DFMT_X8R8G8B8
#define STREAM_LIB_DIRECTDRAW_3D_DEFAULT_BACK_BUFFERS                  D3DPRESENT_BACK_BUFFERS_MAX

// DirectSound
#define STREAM_LIB_DIRECTSOUND_MIC_BOOST_PART_DEFAULT_NAME             "Microphone Boost"

#define STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_BITS_PER_SAMPLE    32
#define STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_CHANNELS           2        // stereo
#define STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_FORMAT             WAVE_FORMAT_IEEE_FLOAT
#define STREAM_LIB_DIRECTSOUND_WAVEOUT_BEST_DEFAULT_SAMPLES_PER_SECOND 96000 // Hz

// DirectShow
// *NOTE*: max. in-flight #'frames' (== #((bits/sample / 8) * #channels))
//         --> (buffer size / frame size) * #SOURCE_BUFFERS
#define STREAM_LIB_DIRECTSHOW_AUDIO_DEFAULT_SOURCE_BUFFERS             15
#define STREAM_LIB_DIRECTSHOW_VIDEO_DEFAULT_SOURCE_BUFFERS             30 // --> max. in-flight #frames

// *IMPORTANT NOTE*: with DirectShow, lower buffer sizes result in lower latency
// *NOTE*: buffer size (bytes) is: avg.#bytes/s * MAX_LATENCY_MS / 1000
// *NOTE*: the maximum latency cannot be lower than the minimum period size
//         as returned by IAudioClient::GetDevicePeriod(); seems to be ~3ms
#define STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_MAX_LATENCY_MS             10 // ms
// *NOTE*: if the graph (i.e. usually the renderers'-) (default) allocator
//         supplies the sample buffers (instead of the (source) filter), and the
//         stream message type does not implement IMediaSample, the 'push'
//         strategy (FillBuffer/Receive) involves a(n additional) memcpy of
//         (inbound) frame data
// *NOTE*: the 'pull' strategy is implemented via IAsynchReader (Request/
//         WaitForNext)
// *IMPORTANT NOTE*: 'asynchronous' filters implement IAsyncReader (downstream
//                   filters 'pull' media samples), 'synchronous' filters
//                   implement IMemInputPin and 'push' media samples to
//                   downstream filters
#define STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH               false

#define STREAM_LIB_DIRECTSHOW_ALLOCATOR_NAME                           "Allocator"

#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE                "Asynch Source"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L              L"Asynch Source"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE                       "Source"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L                     L"Source"
#define STREAM_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME                   L"Output"
                                                                       
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO                L"Capture Audio"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO                L"Capture Video"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB                         L"Sample Grabber"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO                 L"Audio Renderer"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL                  L"Null Renderer"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO                 L"Video Renderer"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER                    L"Resampler"
#define STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER_2                  L"Resampler#2"
                                                                       
#define STREAM_LIB_DIRECTSHOW_FILTER_CLSID_AUDIO_RENDER_WAVEOUT        CLSID_AudioRender
#define STREAM_LIB_DIRECTSHOW_FILTER_CLSID_AUDIO_RENDER_DIRECTSOUND    CLSID_DSoundRender
#define STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_AUDIO_RENDER        STREAM_LIB_DIRECTSHOW_FILTER_CLSID_AUDIO_RENDER_DIRECTSOUND

// *IMPORTANT NOTE*: "...When the Video Renderer draws to a DirectDraw overlay
//                   surface, it allocates a single buffer for its input pin. If
//                   the upstream filter attempts to force a connection using
//                   multiple buffers, the Video Renderer will be unable to use
//                   the overlay surface. ..."
#define STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER                CLSID_VideoRenderer
//#define STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX7      CLSID_VideoMixingRenderer
#define STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX7            CLSID_VideoRendererDefault
#define STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX9            CLSID_VideoMixingRenderer9
#define STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_ENHANCED       CLSID_EnhancedVideoRenderer
#if (_WIN32_WINNT < _WIN32_WINNT_WINXP)                                
#define STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER        STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER
#elif (_WIN32_WINNT < _WIN32_WINNT_VISTA)                              
#if defined (VMR9_SUPPORT)                                             
#define STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER        STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX9
#elif defined (VMR7_SUPPORT)                                           
#define STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER        STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_DX7
#else                                                                  
#define STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER        STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER
#endif // VMR9_SUPPORT || VMR7_SUPPORT                                 
#else                                                                  
#define STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER        STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER_ENHANCED
#endif // _WIN32_WINNT < _WIN32_WINNT_WINXP || _WIN32_WINNT < _WIN32_WINNT_VISTA

#define STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT     MEDIASUBTYPE_RGB32

// properties
#define STREAM_LIB_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING            L"Description"
#define STREAM_LIB_DIRECTSHOW_PROPERTIES_PATH_STRING                   L"DevicePath"
#define STREAM_LIB_DIRECTSHOW_PROPERTIES_NAME_STRING                   L"FriendlyName"
#define STREAM_LIB_DIRECTSHOW_PROPERTIES_ID_STRING                     L"WaveInID"

#define STREAM_LIB_DIRECTSHOW_LOGFILE_NAME                             "ACEStream_DirectShow.log"

// user-defined message to notify applications of filtergraph events
#define STREAM_LIB_DIRECTSHOW_WM_GRAPHNOTIFY_EVENT                     WM_USER + 1

// MediaFoundation
// *NOTE*: transition from SetTopology() to MF_TOPOSTATUS_READY
#define STREAM_LIB_MEDIAFOUNDATION_MEDIASESSION_READY_TIMEOUT_S        10 // seconds
// *NOTE*: transition from Stop() to MESessionClosed
#define STREAM_LIB_MEDIAFOUNDATION_MEDIASESSION_STOP_TIMEOUT_S         10 // seconds
// *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
//         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
//         --> (try to) wait for the next MESessionTopologySet event
#define STREAM_LIB_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT_S              10 // seconds

#define STREAM_LIB_MEDIAFOUNDATION_MEDIASOURCE_FRIENDLY_NAME           L"MediaSource"
// *NOTE*: #samples each stream tries to hold in its queue
#define STREAM_LIB_MEDIAFOUNDATION_MEDIASOURCE_SAMPLE_QUEUE_SIZE       2;

#define STREAM_LIB_MEDIAFOUNDATION_MEDIASOURCE_DEFAULT_STREAM_NAME     L"ACEStream Source Stream"

#define STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_DESCRIPTION       "ACEStream Source ByteStreamHandler"
#define STREAM_LIB_MEDIAFOUNDATION_BYTESTREAMHANDLER_ROOTKEY           "Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers"

// WASAPI
// see also: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks
#define STREAM_LIB_WASAPI_CAPTURE_DEFAULT_TASKNAME                     "Pro Audio"
#define STREAM_LIB_WASAPI_RENDER_DEFAULT_TASKNAME                      "Playback"

#define STREAM_LIB_WASAPI_CAPTURE_DEFAULT_SHAREMODE                    AUDCLNT_SHAREMODE_EXCLUSIVE
#define STREAM_LIB_WASAPI_RENDER_DEFAULT_SHAREMODE                     AUDCLNT_SHAREMODE_SHARED
#endif // ACE_WIN32 || ACE_WIN64

// Noise generator(s)
// *IMPORTANT NOTE*: alpha >= 0 ('white' noise) && alpha <= 2.0 ('brown' noise)
#define STREAM_LIB_NOISE_GENERATOR_PINK_DEFAULT_ALPHA               1.0
// *IMPORTANT NOTE*: #IIR filters
#define STREAM_LIB_NOISE_GENERATOR_PINK_DEFAULT_POLES               5

#define STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_STEP              0.01
#define STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_X                 1.25
#define STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Y                 0.75
#define STREAM_LIB_NOISE_GENERATOR_PERLIN_DEFAULT_Z                 0.5
#endif
