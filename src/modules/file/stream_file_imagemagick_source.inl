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
#include "MagickWand/MagickWand.h"
#else
#include "wand/magick_wand.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "common_image_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_File_ImageMagick_Source_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 TimerManagerType,
                                 UserDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                 MediaType>::Stream_File_ImageMagick_Source_T (ISTREAM_T* stream_in,
#else
                                 MediaType>::Stream_File_ImageMagick_Source_T (typename inherited::ISTREAM_T* stream_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                                               bool autoStart_in,
                                                                               enum Stream_HeadModuleConcurrency concurrency_in,
                                                                               bool generateSessionMessages_in)
 : inherited (stream_in,
              autoStart_in,
              concurrency_in,
              generateSessionMessages_in)
 , inherited2 ()
 , context_ (NULL)
 , directory_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_File_ImageMagick_Source_T::Stream_File_ImageMagick_Source_T"));

  MagickWandGenesis ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_File_ImageMagick_Source_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 TimerManagerType,
                                 UserDataType,
                                 MediaType>::~Stream_File_ImageMagick_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_File_ImageMagick_Source_T::~Stream_File_ImageMagick_Source_T"));

  int result = -1;

  result = directory_.close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Dirent_Selector::close(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

  MagickWandTerminus ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_File_ImageMagick_Source_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 TimerManagerType,
                                 UserDataType,
                                 MediaType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_File_ImageMagick_Source_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    if (context_)
      DestroyMagickWand (context_);
    context_ = NULL;

    result = directory_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Dirent_Selector::close(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF

  context_ = NewMagickWand ();
  if (!context_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  MagickSetImageType (context_, TrueColorType);
  MagickSetImageColorspace (context_, sRGBColorspace);

  if (configuration_in.fileIdentifier.identifierDiscriminator == Common_File_Identifier::DIRECTORY)
  {
    result = directory_.open (ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ()),
                              configuration_in.fileIdentifier.selector,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              NULL);
#else
                              alphasort);
#endif // ACE_WIN32 || ACE_WIN64
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Dirent_Selector::open(\"%s\"): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ())));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: processing %d file(s) in \"%s\"\n"),
                inherited::mod_->name (),
                directory_.length (),
                ACE_TEXT (configuration_in.fileIdentifier.identifier.c_str ())));
#endif // _DEBUG
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
int
Stream_File_ImageMagick_Source_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 TimerManagerType,
                                 UserDataType,
                                 MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_File_ImageMagick_Source_T::svc"));

  int result = -1;
  int result_2 = -1;
  int error = 0;
//  ssize_t bytes_read = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool finished = false;
  bool stop_processing = false;
  int file_index_i = 0;
  std::string file_path_string;
  size_t file_size_i = 0;
  MagickBooleanType result_3= MagickTrue;
  unsigned char* data_p = NULL;
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;
  media_type_s.format = AV_PIX_FMT_RGB24;
  MediaType media_type_2;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      inherited::sessionData_->getR ();
//  ACE_ASSERT (session_data_r.lock);

next:
  file_path_string = inherited::configuration_->fileIdentifier.identifier;
  if (directory_.length ())
  {
    file_path_string += ACE_DIRECTORY_SEPARATOR_STR;
    file_path_string += directory_[file_index_i++]->d_name;
  } // end IF

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: processing file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (file_path_string.c_str ()),
              Common_File_Tools::size (file_path_string)));
#endif // _DEBUG

  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result >= 0)
    {
      ACE_ASSERT (message_block_p);
      message_type = message_block_p->msg_type ();
      switch (message_type)
      {
        case ACE_Message_Block::MB_STOP:
        {
          // clean up
          message_block_p->release (); message_block_p = NULL;

          // *NOTE*: when close()d manually (i.e. user abort), 'finished' will not
          //         have been set at this stage

          // signal the controller ?
          if (!finished)
          {
            finished = true;
            // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
            //         --> continue
            inherited::STATE_MACHINE_T::finished ();
            // *NOTE*: (if passive,) STREAM_SESSION_END has been processed
            //         --> done
            if (inherited::thr_count_ == 0)
              goto done; // finished processing

            continue;
          } // end IF

done:
          result_2 = 0;

          goto continue_; // STREAM_SESSION_END has been processed
        }
        default:
          break;
      } // end SWITCH

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing);
      if (stop_processing)
      {
        // *IMPORTANT NOTE*: message_block_p has already been released() !

        finished = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::STATE_MACHINE_T::finished ();

        continue;
      } // end IF
    } // end IF
    else if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

        if (!finished)
        {
          finished = true;
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::STATE_MACHINE_T::finished ();
        } // end IF

        break;
      } // end IF
    } // end ELSE IF

    // session aborted ?
    if (session_data_r.aborted)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("session aborted\n")));

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::STATE_MACHINE_T::finished ();

      continue;
    } // end IF

    // *TODO*: remove type inference
    result_3 = MagickReadImage (context_,
                                file_path_string.c_str ());
    ACE_ASSERT (result_3 == MagickTrue);

//    media_type_s.codec =
//        Common_Image_Tools::stringToCodecId (MagickGetImageFormat (context_));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    media_type_s.resolution.cx = MagickGetImageWidth (context_);
    media_type_s.resolution.cy = MagickGetImageHeight (context_);
#else
    media_type_s.resolution.width = MagickGetImageWidth (context_);
    media_type_s.resolution.height = MagickGetImageHeight (context_);
#endif // ACE_WIN32 || ACE_WIN64

    result_3 = MagickSetImageFormat (context_, "RGB");
    ACE_ASSERT (result_3 == MagickTrue);

    data_p = MagickGetImageBlob (context_,
                                 &file_size_i);
    ACE_ASSERT (data_p);

    message_p =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::STATE_MACHINE_T::finished ();

      continue;
    } // end IF
    message_p->base (reinterpret_cast<char*> (data_p),
                     file_size_i,
                     0); // own image datas
    message_p->wr_ptr (file_size_i);
    inherited2::getMediaType (media_type_s,
                              media_type_2);
    media_type_2.codec = AV_CODEC_ID_NONE;
    message_p->initialize (media_type_2,
                           message_p->sessionId (),
                           NULL);

    result = inherited::put_next (message_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));

      message_p->release (); message_p = NULL;

      finished = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::STATE_MACHINE_T::finished ();
    } // end IF
    message_p = NULL;

    if (directory_.length () &&
        file_index_i < directory_.length ())
      goto next;

    inherited::TASK_BASE_T::stop (false,
                                  true);
  } while (true);

continue_:
  return result_2;
}
