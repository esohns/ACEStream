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

#ifndef STREAM_LIB_ALSA_COMMON_H
#define STREAM_LIB_ALSA_COMMON_H

#include <deque>

extern "C"
{
#include "alsa/asoundlib.h"
}

#include "stream_dev_defines.h"

struct Stream_MediaFramework_ALSA_MediaType
{
  Stream_MediaFramework_ALSA_MediaType ()
   : access (MODULE_DEV_MIC_ALSA_DEFAULT_ACCESS)
   , bufferSize (MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_SIZE)
   , bufferTime (MODULE_DEV_MIC_ALSA_DEFAULT_BUFFER_TIME)
   , format (MODULE_DEV_MIC_ALSA_DEFAULT_FORMAT)
   , subFormat (SND_PCM_SUBFORMAT_STD)
   , channels (MODULE_DEV_MIC_ALSA_DEFAULT_CHANNELS)
   , periods (MODULE_DEV_MIC_ALSA_DEFAULT_PERIODS)
   , periodSize (MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_SIZE)
   , periodTime (MODULE_DEV_MIC_ALSA_DEFAULT_PERIOD_TIME)
   , rate (MODULE_DEV_MIC_ALSA_DEFAULT_SAMPLE_RATE)
  {}

  enum _snd_pcm_access    access;
  snd_pcm_uframes_t       bufferSize;
  unsigned int            bufferTime;
  enum _snd_pcm_format    format;
  enum _snd_pcm_subformat subFormat;
  unsigned int            channels;
  unsigned int            periods;
  snd_pcm_uframes_t       periodSize;
  unsigned int            periodTime;
  unsigned int            rate;
};
typedef std::deque<struct Stream_MediaFramework_ALSA_MediaType> Stream_MediaFramework_ALSA_Formats_t;
typedef Stream_MediaFramework_ALSA_Formats_t::iterator Stream_MediaFramework_ALSA_FormatsIterator_t;

#endif
