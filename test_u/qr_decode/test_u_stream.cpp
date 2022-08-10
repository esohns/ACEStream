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

#include "test_u_stream.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "uuids.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

#include "stream_stat_defines.h"

#include "test_u_common_modules.h"

Test_U_Stream::Test_U_Stream ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::Test_U_Stream"));

}

bool
Test_U_Stream::load (Stream_ILayout* layout_inout,
                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::load"));

  Stream_Module_t* module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_U_DirectShow_Source_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_DIRECTSHOW_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_U_Source_Module (this,
                                        ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_DEFAULT_NAME_STRING)),
                  false);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //ACE_ASSERT (module_p);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_U_DirectShow_LibAVConverter_Module (this,
                                                           ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_U_Distributor_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_U_DirectShow_QRDecoder_Module (this,
                                                      ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_OPENCV_QR_DECODER_DEFAULT_NAME_STRING)),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_U_Distributor_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_DISTRIBUTOR_DEFAULT_NAME_STRING)),
                  false);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (module_p);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

bool
Test_U_Stream::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Stream::initialize"));

  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
    const_cast<inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  //session_data_p->targetFileName = (*iterator).second.second->targetFileName;

  //inherited::dump_state ();

  return inherited::initialize (configuration_in);
}
