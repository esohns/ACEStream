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

#if defined (MYSQL_SUPPORT)
#include "soci/mysql/soci-mysql.h"
#endif // MYSQL_SUPPORT

#include "ace/Log_Msg.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
Stream_Module_SOCI_Reader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            StatisticContainerType,
                            SessionManagerType,
                            TimerManagerType>::Stream_Module_SOCI_Reader_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                            bool autoStart_in,
                                                                            bool generateSessionMessages_in)
 : inherited (lock_in,                    // lock handle
              autoStart_in,               // auto-start ?
              generateSessionMessages_in) // generate sesssion messages ?
 , session_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Reader_T::Stream_Module_SOCI_Reader_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
Stream_Module_SOCI_Reader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            StatisticContainerType,
                            SessionManagerType,
                            TimerManagerType>::~Stream_Module_SOCI_Reader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Reader_T::~Stream_Module_SOCI_Reader_T"));

  if (session_.is_connected ())
    session_.close ();
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
bool
Stream_Module_SOCI_Reader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            StatisticContainerType,
                            SessionManagerType,
                            TimerManagerType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Reader_T::initialize"));

  // step0: initialize library ?
  static bool first_run = true;
  if (first_run)
  {
    first_run = false;
  } // end IF

  if (inherited::initialized_)
  {
    if (session_.is_connected ())
      session_.close ();
  } // end IF

  const soci::backend_factory* backend_factory_p = NULL;
  switch (configuration_in.DBBackend)
  {
    case STREAM_DATABASE_BACKEND_MYSQL:
    {
#if defined (MYSQL_SUPPORT)
      backend_factory_p = soci::factory_mysql ();
#endif // MYSQL_SUPPORT
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
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
void
Stream_Module_SOCI_Reader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            StatisticContainerType,
                            SessionManagerType,
                            TimerManagerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Reader_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::initialized_);

  const typename SessionMessageType::DATA_T& session_data_container_r =
    message_inout->get ();
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      // schedule regular statistic collection ?
      if (inherited::configuration_.streamConfiguration->statisticReportingInterval != ACE_Time_Value::zero)
      { ACE_ASSERT (inherited::timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
        inherited::timerId_ =
            itimer_manager_p->schedule_timer (&inherited::statisticHandler_, // event handler handle
                                              NULL,                          // asynchronous completion token
                                              COMMON_TIME_NOW + interval,    // first wakeup time
                                              interval);                     // interval
        if (inherited::timerId_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (id: %d) for interval %#T\n"),
//                    inherited::timerId_,
//                    &interval));
      } // end IF

      // sanity check(s)
      ACE_ASSERT (session_.is_connected ());

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::timerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      if (session_.is_connected ())
      {
        session_.close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed database connection\n"),
                    inherited::mod_->name ()));
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
bool
Stream_Module_SOCI_Reader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            StatisticContainerType,
                            SessionManagerType,
                            TimerManagerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Reader_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0;
  data_out.timestamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
int
Stream_Module_SOCI_Reader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            StatisticContainerType,
                            SessionManagerType,
                            TimerManagerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SOCI_Reader_T::svc"));

  int result = -1;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = ACE_OS::gettimeofday ();
  DataMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (session_.is_connected ());

  size_t row_count_i = 0;
  std::string query_string = ACE_TEXT_ALWAYS_CHAR ("SELECT COUNT(*) FROM ");
  query_string += inherited::configuration_->DBTable;
  session_ << query_string, soci::into (row_count_i);

  query_string = ACE_TEXT_ALWAYS_CHAR ("SELECT * FROM ");
  query_string += inherited::configuration_->DBTable;
  soci::row row;
  soci::statement statement = (session_.prepare << query_string, soci::into (row));
  if (unlikely (!statement.execute ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to execute query \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (query_string.c_str ())));
    goto done;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: loading table \"%s\" (%Q record(s) in %Q field(s))...\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::configuration_.DBTable.c_str ()),
              row_count_i,
              row.size ()));

  // step1: start processing data...
  do
  {
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result == 0)
    {
      ACE_ASSERT (message_block_p);
      ACE_Message_Block::ACE_Message_Type message_type =
        message_block_p->msg_type ();
      if (message_type == ACE_Message_Block::MB_STOP)
      {
        // clean up
        message_block_p->release (); message_block_p = NULL;

        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will not
        //         have been set at this stage

        // signal the controller ?
        if (!finished)
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: session aborted\n"),
                      inherited::mod_->name ()));

          finished = true;
          inherited::finished ();

          continue;
        } // end IF

        result_2 = 0;

        break; // done
      } // end IF

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
    } // end IF
    else if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

        // signal the controller ?
        if (!finished)
        {
          finished = true;
          inherited::finished ();
        } // end IF

        break;
      } // end IF
    } // end IF

    // fetch next row
    if (!finished && !statement.fetch ())
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: no more data available, finishing\n"),
                  inherited::mod_->name ()));
      finished = true;
      inherited::finished ();
      continue;
    }
    if (unlikely (!session_.got_data ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to retrieve record data (was: #%Q), aborting\n"),
                  inherited::mod_->name (),
                  row_count_i));
      finished = true;
      inherited::finished ();
      continue;
    } // end IF
    ++row_count_i;

    // *TODO*: remove type inference
    message_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: allocateMessage(%Q) failed: \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      finished = true;
      inherited::finished ();
      continue;
    } // end IF

    typename DataMessageType::DATA_T& message_data_r = message_p->get ();
    message_data_r.row = row;

    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: ACE_Task::put_next() failed: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      finished = true;
      inherited::finished ();
      continue;
    } // end IF
    message_p = NULL;
  } while (!stop_processing);

done:
  return result_2;
}
