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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename ConfigurationType>
Stream_AllocatorBase_T<ConfigurationType>::Stream_AllocatorBase_T ()
 : configuration_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::Stream_AllocatorBase_T"));

}

template <typename ConfigurationType>
Stream_AllocatorBase_T<ConfigurationType>::~Stream_AllocatorBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::~Stream_AllocatorBase_T"));

}

template <typename ConfigurationType>
bool
Stream_AllocatorBase_T<ConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  return true;
}

/////////////////////////////////////////

template <typename ConfigurationType>
void*
Stream_AllocatorBase_T<ConfigurationType>::calloc (size_t, size_t, char)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::calloc"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL;)
}


template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::remove (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::remove"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::bind (const char*, void*, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::bind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::trybind (const char*, void*&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::trybind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::find (const char*, void*&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::find"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::find (const char*)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::find"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::unbind (const char*)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::unbind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::unbind (const char*, void*&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::unbind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::sync (ssize_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::sync"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::sync (void*, size_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::sync"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::protect (ssize_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::protect"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <typename ConfigurationType>
int
Stream_AllocatorBase_T<ConfigurationType>::protect (void*, size_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::protect"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

/////////////////////////////////////////

#if defined (ACE_HAS_MALLOC_STATS)
template <typename ConfigurationType>
void
Stream_AllocatorBase_T<ConfigurationType>::print_stats (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::print_stats"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
#endif /* ACE_HAS_MALLOC_STATS */

template <typename ConfigurationType>
void
Stream_AllocatorBase_T<ConfigurationType>::dump (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorBase_T::dump"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
