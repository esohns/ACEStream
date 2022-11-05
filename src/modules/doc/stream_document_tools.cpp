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
#include "stdafx.h"

#include "stream_document_tools.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#if defined (LIBREOFFICE_SUPPORT)
unsigned int
Stream_Document_Tools::firstFreeRow (uno::Reference<table::XCellRange>& range_in,
                                     unsigned int column_in,
                                     unsigned int row_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Document_Tools::firstFreeRow"));

  unsigned int result = row_in;

  uno::Reference<table::XCell> cell_p = NULL;
  do
  {
    cell_p = range_in->getCellByPosition (column_in, result);
    ACE_ASSERT (cell_p.is ());
    if (cell_p->getType () == table::CellContentType_EMPTY)
      break;
    ++result;
  } while (true);

  return result;
}
#endif // LIBREOFFICE_SUPPORT
