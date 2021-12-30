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

#ifndef TEST_I_STREAM_COMMON_H
#define TEST_I_STREAM_COMMON_H

#include "stream_configuration.h"

#include "test_i_configuration.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_ModuleHandlerConfiguration;
struct Test_I_MediaFoundation_ModuleHandlerConfiguration;
#else
struct Test_I_ALSA_ModuleHandlerConfiguration;
#endif // ACE_WIN32 || ACE_WIN64

//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_StreamConfiguration,
                               struct Test_I_DirectShow_ModuleHandlerConfiguration> Test_I_DirectShow_StreamConfiguration_t;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_StreamConfiguration,
                               struct Test_I_MediaFoundation_ModuleHandlerConfiguration> Test_I_MediaFoundation_StreamConfiguration_t;
#else
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_StreamConfiguration,
                               struct Test_I_ALSA_ModuleHandlerConfiguration> Test_I_ALSA_StreamConfiguration_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
