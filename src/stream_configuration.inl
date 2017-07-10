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
          typename AllocatorConfigurationType,
          typename ConfigurationType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Stream_Configuration_T<//StreamName,
                       AllocatorConfigurationType,
                       ConfigurationType,
                       ModuleConfigurationType,
                       ModuleHandlerConfigurationType>::Stream_Configuration_T ()
 : allocatorConfiguration_ ()
 , configuration_ ()
 , moduleConfiguration_ ()
 //, name_ (StreamName)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Configuration_T::Stream_Configuration_T"));

  // *TODO*: remove type inferences
  configuration_.moduleConfiguration = &moduleConfiguration_;
  configuration_.moduleConfiguration->streamConfiguration = &configuration_;
}

template <//const char* StreamName,
          typename AllocatorConfigurationType,
          typename ConfigurationType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Stream_Configuration_T<//StreamName,
                       AllocatorConfigurationType,
                       ConfigurationType,
                       ModuleConfigurationType,
                       ModuleHandlerConfigurationType>::initialize (const AllocatorConfigurationType& allocatorConfiguration_in,
                                                                    const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Configuration_T::initialize"));

  allocatorConfiguration_ = allocatorConfiguration_in;
  configuration_ = configuration_in;

  // *TODO*: remove type inferences
  configuration_.moduleConfiguration = &moduleConfiguration_;
  configuration_.moduleConfiguration->streamConfiguration = &configuration_;

  return true;
}

template <//const char* StreamName,
          typename AllocatorConfigurationType,
          typename ConfigurationType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Stream_Configuration_T<//StreamName,
                       AllocatorConfigurationType,
                       ConfigurationType,
                       ModuleConfigurationType,
                       ModuleHandlerConfigurationType>::initialize (const ModuleHandlerConfigurationType& moduleHandlerConfiguration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Configuration_T::initialize"));

  inherited::insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                     moduleHandlerConfiguration_in));

  isInitialized_ = true;

  return true;
}

template <//const char* StreamName,
          typename AllocatorConfigurationType,
          typename ConfigurationType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Stream_Configuration_T<//StreamName,
                       AllocatorConfigurationType,
                       ConfigurationType,
                       ModuleConfigurationType,
                       ModuleHandlerConfigurationType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Configuration_T::dump_state"));

  for (CONST_ITERATOR_T iterator = inherited::begin ();
       iterator != inherited::end ();
       ++iterator)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("\"%s\": %@\n"),
                ACE_TEXT ((*iterator).first.c_str ()),
                &(*iterator).second));
}
