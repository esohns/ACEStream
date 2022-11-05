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

#ifndef STREAM_DOCUMENT_TOOLS_H
#define STREAM_DOCUMENT_TOOLS_H

#include "ace/Global_Macros.h"

#if defined (LIBREOFFICE_SUPPORT)
#include "com/sun/star/uno/Reference.h"

#include "com/sun/star/table/XCellRange.hpp"

using namespace ::com::sun::star;
#endif // LIBREOFFICE_SUPPORT

class Stream_Document_Tools
{
 public:
#if defined (LIBREOFFICE_SUPPORT)
  static unsigned int firstFreeRow (uno::Reference<table::XCellRange>&, // cell range
                                    unsigned int,                       // column
                                    unsigned int);                      // row
#endif // LIBREOFFICE_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Document_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Document_Tools (const Stream_Document_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Document_Tools& operator= (const Stream_Document_Tools&))
};

#endif
