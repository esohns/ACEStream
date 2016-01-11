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

#ifndef STREAM_MODULE_DEV_TOOLS_H
#define STREAM_MODULE_DEV_TOOLS_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <list>
#include <map>
#endif
#include <string>

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dshow.h"
#endif

#include "stream_dev_exports.h"

class Stream_Dev_Export Stream_Module_Device_Tools
{
 public:
   static void initialize ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool clear (IGraphBuilder*); // graph handle
  // *NOTE*: see stream_dev_defines.h for supported filter names
  static bool connect (IGraphBuilder*,                  // graph handle
                       const std::list<std::wstring>&); // graph
  static bool disconnect (IGraphBuilder*); // graph handle
  static bool load (const std::string&,  // device ("FriendlyName")
                    IGraphBuilder*&,     // (capture) graph handle (in/out)
                    IAMStreamConfig*&);  // stream config handle (out)
  static bool load (IGraphBuilder*&, // graph handle (in/out)
                    const HWND);     // window handle [NULL: NullRenderer]
  static bool reset (IGraphBuilder*); // graph handle

  static bool getFormat (IGraphBuilder*,         // graph handle
                         struct _AMMediaType*&); // return value: media type
  static bool setFormat (IGraphBuilder*,              // graph handle
                         const struct _AMMediaType&); // media type
  static void deleteMediaType (struct _AMMediaType*&); // handle
  static std::string mediaSubTypeToString (const GUID&); // GUID
  static std::string mediaTypeToString (const struct _AMMediaType&); // media type
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools (const Stream_Module_Device_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools& operator= (const Stream_Module_Device_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct less_guid
  {
    bool operator() (const GUID& lhs_in, const GUID& rhs_in) const
    {
      //ACE_ASSERT (lhs_in.Data2 == rhs_in.Data2);
      //ACE_ASSERT (lhs_in.Data3 == rhs_in.Data3);
      //ACE_ASSERT (*(long long*)lhs_in.Data4 == *(long long*)rhs_in.Data4);

      return (lhs_in.Data1 < rhs_in.Data1);
    }
  };
  typedef std::map<GUID, std::string, less_guid> GUID2STRING_MAP_T;
  typedef GUID2STRING_MAP_T::const_iterator GUID2STRING_MAP_ITERATOR_T;
  static GUID2STRING_MAP_T Stream_MediaMajorType2StringMap;
  static GUID2STRING_MAP_T Stream_MediaSubType2StringMap;
  static GUID2STRING_MAP_T Stream_FormatType2StringMap;
#endif
};

#endif
