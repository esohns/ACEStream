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

#include "soci/mysql/soci-mysql.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_module_db_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_SOCI_Writer_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::Stream_Module_SOCI_Writer_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , session_ ()
 , transaction_ (session_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Writer_T::Stream_Module_SOCI_Writer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_SOCI_Writer_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::~Stream_Module_SOCI_Writer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Writer_T::~Stream_Module_SOCI_Writer_T"));

  if (session_.is_connected ())
    session_.close ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
bool
Stream_Module_SOCI_Writer_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Writer_T::initialize"));

  int result = -1;

  // step0: initialize library ?
  static bool first_run = true;
  if (first_run)
  {
    first_run = false;
  } // end IF

  if (inherited::isInitialized_)
  {
    if (session_.is_connected ())
      session_.close ();
  } // end IF

  const soci::backend_factory* backend_factory_p = NULL;
  switch (configuration_in.DBBackend)
  {
    case STREAM_DATABASE_BACKEND_MYSQL:
    {
      backend_factory_p = soci::factory_mysql ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unsupported database backend type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  configuration_in.DBBackend));
      return false;
    }
  } // end SWITCH
  if (unlikely (!backend_factory_p))
  { // *NOTE*: factory not registered ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve database backend handle (was: %d), aborting\n"),
                inherited::mod_->name (),
                configuration_in.DBBackend));
    return false;
  } // end IF

  if (!session_.open (*backend_factory_p,
                      configuration_in.DBConnectionString))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize database session (connection string was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.DBConnectionString.c_str ())));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);

error:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: failed to mysql_options(%d): \"%s\", aborting\n"),
              inherited::mod_->name (),
              option,
              ACE_TEXT (mysql_error (state_))));
  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Module_SOCI_Writer_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Writer_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (session_.is_connected ());

  std::string query_string = ACE_TEXT_ALWAYS_CHAR ("INSERT INTO ");
  query_string += inherited::configuration_->DBTable;
  query_string += ACE_TEXT_ALWAYS_CHAR ("VALUES (");
  //      for (Test_I_PageDataIterator_t iterator =
  //      session_data_r.data.pageData.begin ();
  //           iterator != session_data_r.data.pageData.end ();
  //           ++iterator)
  //      {
  //        query_string += ACE_TEXT_ALWAYS_CHAR (",");

  //        query_string += ACE_TEXT_ALWAYS_CHAR ("),(");
  //      } // end FOR
  query_string += ACE_TEXT_ALWAYS_CHAR (")");

  session_ << query_string;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Module_SOCI_Writer_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Writer_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (session_.is_connected ());

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      if (unlikely (!session_.is_connected ()))
        return; // nothing to do

commit:
      transaction_.commit ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: committed data record(s)\n"),
                  inherited::mod_->name ()));

close:
      session_.close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed database connection\n"),
                  inherited::mod_->name ()));

      break;
    }
    default:
      break;
  } // end SWITCH
}
