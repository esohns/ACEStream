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

#ifndef STREAM_DOCUMENT_DEFINES_H
#define STREAM_DOCUMENT_DEFINES_H

#include "ace/config-lite.h"

#define MODULE_DOCUMENT_LIBREOFFICE_WRITER_DEFAULT_NAME_STRING        "LibreOfficeWriter"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define MODULE_DOCUMENT_MSOFFICE_WRITER_DEFAULT_NAME_STRING           "MSOfficeWriter"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_START_SH                  "start_soffice.bat"
#else
#define STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_START_SH                  "start_soffice.sh"
#endif // ACE_WIN32 || ACE_WIN64
#define STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_PROCESS_EXE               "soffice.bin"

#define STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_SERVER_HOST               ACE_LOCALHOST
#define STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_SERVER_PORT               2083

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define STREAM_DOCUMENT_LIBREOFFICE_BOOTSTRAP_FILE_SUFFIX             ".ini"
#else
#define STREAM_DOCUMENT_LIBREOFFICE_BOOTSTRAP_FILE_SUFFIX             "rc"
#endif // ACE_WIN32 || ACE_WIN64

#define STREAM_DOCUMENT_LIBREOFFICE_FRAME_BLANK                       "_blank"
#define STREAM_DOCUMENT_LIBREOFFICE_FRAME_SPREADSHEET_NEW             "private:factory/scalc"

// libreoffice properties
#define STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_DEFAULT_CONTEXT          "DefaultContext"

#define STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_INTERACTIONHANDLER  "InteractionHandler"
#define STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_HIDDEN              "Hidden"
#define STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_MACROEXECCUTIONMODE "MacroExecutionMode"
#define STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_OVERWRITE           "Overwrite"

// libreoffice calc properties
#define STREAM_DOCUMENT_LIBREOFFICE_CALC_DEFAULT_TABLE_NAME           "Tabelle1"

#endif
