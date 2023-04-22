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

#include "MagickWand/MagickWand.h"

#include "ace/Log_Msg.h"

#include "common_file_common.h"
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
                                 MediaType>::Stream_File_ImageMagick_Source_T (ISTREAM_T* stream_in)
#else
                                 MediaType>::Stream_File_ImageMagick_Source_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , context_ (NULL)
 , directory_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_File_ImageMagick_Source_T::Stream_File_ImageMagick_Source_T"));

  //MagickWandGenesis ();
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

  //MagickWandTerminus ();
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
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  typename DataMessageType::DATA_T message_data_s;
  bool finished = false;
  bool stop_processing = false;
  int file_index_i = 0;
  std::string file_path_string;
  size_t file_size_i = 0;
  unsigned int result_3 = MagickTrue;
  unsigned char* data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
  media_type_s.format = AV_PIX_FMT_BGRA;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  MediaType media_type_2;
  ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
//  ACE_ASSERT (session_data_r.lock);
  session_data_r.statistic.totalFrames = directory_.length ();

next:
  file_path_string = inherited::configuration_->fileIdentifier.identifier;
  if (directory_.length ())
  {
    file_path_string += ACE_DIRECTORY_SEPARATOR_STR;
    file_path_string += directory_[file_index_i++]->d_name;
  } // end IF
  else if (!Common_File_Tools::isValidFilename (inherited::configuration_->fileIdentifier.identifier)) // empty directory ?
    goto continue_;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: processing file \"%s\" (%u byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (file_path_string.c_str ()),
              Common_File_Tools::size (file_path_string)));

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
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished (false); // recurse upstream ?

            continue;
          } // end IF

          result_2 = 0;

          goto continue_2; // STREAM_SESSION_END has been processed
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
        inherited::stop (false, false, false);

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
          inherited::stop (false, false, false);
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
      inherited::stop (false, false, false);

      continue;
    } // end IF
    else if (finished) // stop / session begin / end
      continue;

    // *TODO*: remove type inference
    //result = MagickReadImage (context_,
    //                          ACE_TEXT_ALWAYS_CHAR ("xc:black"));
    //ACE_ASSERT (result == MagickTrue);

    result_3 = MagickReadImage (context_,
                                file_path_string.c_str ());
    if (unlikely (result_3 != MagickTrue))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to MagickReadImage(\"%s\"): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (file_path_string.c_str ()),
                  ACE_TEXT (Common_Image_Tools::errorToString (context_).c_str ())));

      finished = true;
      inherited::stop (false, false, false);

      continue;
    } // end IF

//    media_type_s.codec =
//        Common_Image_Tools::stringToCodecId (MagickGetImageFormat (context_));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ASSERT (!session_data_r.formats.empty ());
    inherited2::getMediaType (session_data_r.formats.back (),
                              STREAM_MEDIATYPE_VIDEO,
                              media_type_s);
    Common_Image_Resolution_t resolution_s;
    resolution_s.cx = static_cast<LONG> (MagickGetImageWidth (context_));
    resolution_s.cy = static_cast<LONG> (MagickGetImageHeight (context_));
    Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_s,
                                                           media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
    media_type_s.resolution.width = MagickGetImageWidth (context_);
    media_type_s.resolution.height = MagickGetImageHeight (context_);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

    result_3 =
      MagickSetImageFormat (context_,
                            ACE_TEXT_ALWAYS_CHAR ("RGBA"));
    ACE_ASSERT (result_3 == MagickTrue);

    data_p = MagickGetImageBlob (context_, // was: MagickWriteImageBlob
                                 &file_size_i);
    ACE_ASSERT (data_p);

//#if defined (_DEBUG)
//  Common_File_Tools::store (ACE_TEXT_ALWAYS_CHAR ("output.rgba"),
//                            reinterpret_cast<uint8_t*> (data_p),
//                            file_size_i);
//#endif // _DEBUG

    message_p =
        inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));

      finished = true;
      inherited::stop (false, false, false);

      continue;
    } // end IF
    // *TODO*: crashes in release()...(needs MagickRelinquishMemory())
    message_p->base (reinterpret_cast<char*> (data_p),
                     file_size_i,
                     ACE_Message_Block::DONT_DELETE); // own image datas
    message_p->wr_ptr (file_size_i);
    inherited2::getMediaType (media_type_s,
                              STREAM_MEDIATYPE_VIDEO,
                              media_type_2);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
    media_type_2.codec = AV_CODEC_ID_NONE;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
    message_data_s.format = media_type_2;
    message_data_s.relinquishMemory = data_p;
    message_p->initialize (message_data_s,
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
      inherited::stop (false, false, false);
    } // end IF
    message_p = NULL;

continue_:
    if (directory_.length () &&
        file_index_i < directory_.length ())
      goto next;

    this->stop (false, // wait ?
                true,  // high priority ?
                true); // locked access ?
  } while (true);

continue_2:
  return result_2;
}
