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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <limits>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"
#include "ace/Thread_Manager.h"

#include "stream_macros.h"

template <typename ThreadDataType,
          typename CallbackDataType>
bool
Test_U_Tools::spawn (const std::string& threadName_in,
                     ACE_THR_FUNC function_in,
                     int groupId_in,
                     const CallbackDataType& callbackData_in,
                     ACE_Thread_ID& thread_id_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_Tools::spawn"));

  ThreadDataType* thread_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;

  ACE_NEW_NORETURN (thread_data_p,
                    ThreadDataType ());
  if (unlikely (!thread_data_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return false;
  } // end IF
  thread_data_p->CBData = &const_cast<CallbackDataType&> (callbackData_in);
  ACE_TCHAR thread_name[BUFSIZ];
  ACE_OS::memset (thread_name, 0, sizeof (ACE_TCHAR[BUFSIZ]));
  //  char* thread_name_p = NULL;
//  ACE_NEW_NORETURN (thread_name_p,
//                    ACE_TCHAR[BUFSIZ]);
//  if (!thread_name_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
//    delete thread_data_p; thread_data_p = NULL;
//    return;
//  } // end IF
//  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
//  ACE_OS::strcpy (thread_name_p,
//                  ACE_TEXT (TEST_U_Stream_CamSave_THREAD_NAME));
//  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (threadName_in.c_str ()));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  int result =
    thread_manager_p->spawn (function_in,                 // function
                             thread_data_p,               // argument
                             (THR_NEW_LWP      |
                              THR_JOINABLE     |
                              THR_INHERIT_SCHED),         // flags
                             &thread_id,                  // thread id
                             &thread_handle,              // thread handle
                             ACE_DEFAULT_THREAD_PRIORITY, // priority
                             groupId_in,                  // group id
                             NULL,                        // stack
                             0,                           // stack size
                             &thread_name_2);             // name
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", aborting\n")));
    delete thread_data_p; thread_data_p = NULL;
    return false;
  } // end IF
  thread_id_out.id (thread_id);
  thread_id_out.handle (thread_handle);

  return true;
}
