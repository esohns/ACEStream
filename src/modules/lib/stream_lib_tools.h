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

#ifndef STREAM_LIB_TOOLS_H
#define STREAM_LIB_TOOLS_H

#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <map>

#include <guiddef.h>
#include <mfobjects.h>
#include <strmif.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Basic_Types.h"
#include "ace/Global_Macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#endif // ACE_WIN32 || ACE_WIN64

class Stream_MediaFramework_Tools
{
  friend class Stream_MediaFramework_DirectShow_Tools;
  friend class Stream_MediaFramework_MediaFoundation_Tools;

 public:
  inline static std::string FOURCCToString (ACE_UINT32 fourCC_in) { return std::string (reinterpret_cast<char*> (&fourCC_in), 4); }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool initialize (enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);
  static void finalize (enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);

  static bool isChromaLuminance (REFGUID,                          // media subtype
                                 enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);
  static bool isRGB (REFGUID,                          // media subtype
                     enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);

  static std::string mediaFormatTypeToString (REFGUID); // media format type
  static std::string mediaSubTypeToString (REFGUID,                          // media subtype
                                           enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);
  static std::string mediaMajorTypeToString (REFGUID, // media major type
                                             enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);

  static unsigned int frameSize (const struct _AMMediaType&);
  static unsigned int frameSize (const IMFMediaType*);
#else
  static bool initialize ();
  static void finalize ();
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools (const Stream_MediaFramework_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools& operator= (const Stream_MediaFramework_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct less_guid
  {
    inline bool operator () (const struct _GUID& lhs_in, const struct _GUID& rhs_in) const { return (lhs_in.Data1 < rhs_in.Data1); }
  };
  typedef std::map<struct _GUID, std::string, less_guid> GUID_TO_STRING_MAP_T;
  typedef GUID_TO_STRING_MAP_T::const_iterator GUID_TO_STRING_MAP_ITERATOR_T;

  static GUID_TO_STRING_MAP_T Stream_MediaFramework_FormatTypeToStringMap;
  static GUID_TO_STRING_MAP_T Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap;
  static GUID_TO_STRING_MAP_T Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap;
#endif // ACE_WIN32 || ACE_WIN64
};

#endif
