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

#include "stream_module_llamacpp.h"

#include "stream_module_ml_defines.h"

const char libacestream_default_ml_llamacpp_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (MODULE_ML_LLAMA_CPP_DEFAULT_NAME_STRING);

//////////////////////////////////////////

void
acestream_ggml_log_callback (enum ggml_log_level level,
                             const char* text,
                             void* user_data)
{
#if defined (_DEBUG)
    if (level >= GGML_LOG_LEVEL_NONE)
#else
    if (level >= GGML_LOG_LEVEL_ERROR)
#endif // _DEBUG
      ACE_DEBUG (((level >= GGML_LOG_LEVEL_ERROR ? LM_ERROR : LM_DEBUG),
                  ACE_TEXT ("%s"),
                  ACE_TEXT (text)));
}
