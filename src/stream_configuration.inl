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

template <//const char* StreamName,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType>
Stream_Configuration_T<//StreamName,
                       ConfigurationType,
                       ModuleHandlerConfigurationType>::Stream_Configuration_T ()
 : inherited ()
 , configuration_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Configuration_T::Stream_Configuration_T"));

}

template <//const char* StreamName,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Stream_Configuration_T<//StreamName,
                       ConfigurationType,
                       ModuleHandlerConfigurationType>::initialize (const struct Stream_ModuleConfiguration& moduleConfiguration_in,
                                                                    const ModuleHandlerConfigurationType& moduleHandlerConfiguration_in,
                                                                    const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Configuration_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isInitialized_);

  inherited::insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                     std::make_pair (&const_cast<struct Stream_ModuleConfiguration&> (moduleConfiguration_in),
                                                     &const_cast<ModuleHandlerConfigurationType&> (moduleHandlerConfiguration_in))));
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isInitialized_ = true;

  return true;
}

template <//const char* StreamName,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Stream_Configuration_T<//StreamName,
                       ConfigurationType,
                       ModuleHandlerConfigurationType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Configuration_T::dump_state"));

  for (CONST_ITERATOR_T iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("\"%s\": 0x%@,0x%@\n"),
                ACE_TEXT ((*iterator).first.c_str ()),
                &(*iterator).second.first, &(*iterator).second.second));
}
