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

#include "common_macros.h"
#include "common_tools.h"

#include "common_timer_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::Stream_SessionDataMediaBase_T ()
 : inherited ()
#if defined (FFMPEG_SUPPORT)
 , codecConfiguration ()
#endif // FFMPEG_SUPPORT
 , formats ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
 , state (NULL)
 , statistic ()
 , sourceFileName ()
 , targetFileName ()
 , userData (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::Stream_SessionDataMediaBase_T"));

}

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::Stream_SessionDataMediaBase_T (const OWN_TYPE_T& data_in)
 : inherited (data_in)
#if defined (FFMPEG_SUPPORT)
 , codecConfiguration ()
#endif // FFMPEG_SUPPORT
 , formats (data_in.formats)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , mediaFramework (data_in.mediaFramework)
#endif // ACE_WIN32 || ACE_WIN64
 , state (data_in.state)
 , statistic (data_in.statistic)
 , sourceFileName (data_in.sourceFileName)
 , targetFileName (data_in.targetFileName)
 , userData (data_in.userData)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::Stream_SessionDataMediaBase_T"));

}

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::~Stream_SessionDataMediaBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::~Stream_SessionDataMediaBase_T"));

#if defined (FFMPEG_SUPPORT)
  for (std::map<enum AVCodecID, struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration>::const_iterator iterator = codecConfiguration.begin ();
       iterator != codecConfiguration.end ();
       ++iterator)
    delete [] (*iterator).second.data;
#endif // FFMPEG_SUPPORT
}

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>&
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::operator+= (const Stream_SessionDataMediaBase_T<BaseType,
                                                                                             MediaFormatType,
                                                                                             StreamStateType,
                                                                                             StatisticType,
                                                                                             UserDataType>& rhs_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::operator+="));

  // *NOTE*: the idea is to 'merge' the data
  inherited::operator+= (rhs_in);

#if defined (FFMPEG_SUPPORT)
  // *TODO*: the idea is to 'merge' the data
  for (std::map<enum AVCodecID, struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration>::const_iterator iterator = codecConfiguration.begin ();
       iterator != codecConfiguration.end ();
       ++iterator)
    delete[] (*iterator).second.data;
  codecConfiguration.clear ();
  codecConfiguration = rhs_in.codecConfiguration;
  for (std::map<enum AVCodecID, struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration>::iterator iterator = codecConfiguration.begin ();
       iterator != codecConfiguration.end ();
       ++iterator)
  {
    ACE_UINT8* data_orig_p = (*iterator).second.data;
    (*iterator).second.data = NULL;
    ACE_NEW_NORETURN ((*iterator).second.data,
                      ACE_UINT8[(*iterator).second.size + AV_INPUT_BUFFER_PADDING_SIZE]);
    ACE_ASSERT ((*iterator).second.data);
    ACE_OS::memset ((*iterator).second.data, 0, (*iterator).second.size + AV_INPUT_BUFFER_PADDING_SIZE);
    ACE_OS::memcpy ((*iterator).second.data,
                    data_orig_p,
                    (*iterator).second.size);
  } // end FOR
#endif // FFMPEG_SUPPORT

  // *NOTE*: the idea is to 'merge' the data
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (formats.empty ())
  {
    // *CONSIDER*: use template specialization instead ?
    if (Common_Tools::equalType ((MediaFormatType*)NULL, (struct _AMMediaType*)NULL))
      for (MEDIAFORMATS_CONST_ITERATOR_T iterator = rhs_in.formats.begin ();
           iterator != rhs_in.formats.end ();
           ++iterator)
      { // *NOTE*: this should be safe because of the check above
        struct _AMMediaType* media_type_p = (struct _AMMediaType*)&(*iterator);
        struct _AMMediaType* media_type_2 =
          Stream_MediaFramework_DirectShow_Tools::copy (*media_type_p);
        formats.push_back (*(MediaFormatType*)media_type_2);
      } // end FOR
    else if (Common_Tools::equalType ((MediaFormatType*)NULL, (IMFMediaType**)NULL))
      for (MEDIAFORMATS_CONST_ITERATOR_T iterator = rhs_in.formats.begin ();
           iterator != rhs_in.formats.end ();
           ++iterator)
      { // *NOTE*: this should be safe because of the check above
        IMFMediaType* media_type_p = *(IMFMediaType**)&(*iterator);
        media_type_p->AddRef ();
        formats.push_back (*(MediaFormatType*)&media_type_p);
      } // end FOR
    else
     formats = rhs_in.formats;
  } // end IF
#else
  formats = (formats.empty () ? rhs_in.formats : formats);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework = rhs_in.mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  state = rhs_in.state;
  statistic =
    ((statistic.timeStamp >= rhs_in.statistic.timeStamp) ? statistic : rhs_in.statistic);
  sourceFileName = rhs_in.sourceFileName;
  targetFileName = rhs_in.targetFileName;
  userData = (userData ? userData : rhs_in.userData);

  return *this;
}

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>&
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::operator= (const Stream_SessionDataMediaBase_T<BaseType,
                                                                                            MediaFormatType,
                                                                                            StreamStateType,
                                                                                            StatisticType,
                                                                                            UserDataType>& rhs_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::operator="));

  inherited::operator= (rhs_in);

#if defined (FFMPEG_SUPPORT)
  for (std::map<enum AVCodecID, struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration>::const_iterator iterator = codecConfiguration.begin ();
       iterator != codecConfiguration.end ();
       ++iterator)
    delete[] (*iterator).second.data;
  codecConfiguration.clear ();
  codecConfiguration = rhs_in.codecConfiguration;
  for (std::map<enum AVCodecID, struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration>::iterator iterator = codecConfiguration.begin ();
       iterator != codecConfiguration.end ();
       ++iterator)
  {
    ACE_UINT8* data_orig_p = (*iterator).second.data;
    (*iterator).second.data = NULL;
    ACE_NEW_NORETURN ((*iterator).second.data,
                      ACE_UINT8[(*iterator).second.size + AV_INPUT_BUFFER_PADDING_SIZE]);
    ACE_ASSERT ((*iterator).second.data);
    ACE_OS::memset ((*iterator).second.data, 0, (*iterator).second.size + AV_INPUT_BUFFER_PADDING_SIZE);
    ACE_OS::memcpy ((*iterator).second.data,
                    data_orig_p,
                    (*iterator).second.size);
  } // end FOR
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool clear_b = false;
  // *CONSIDER*: use template specialization instead ?
  if (Common_Tools::equalType ((MediaFormatType*)NULL, (struct _AMMediaType*)NULL))
  { clear_b = true;
    for (MEDIAFORMATS_ITERATOR_T iterator = formats.begin ();
         iterator != formats.end ();
         ++iterator)
    { // *NOTE*: this should be safe because of the check above
      struct _AMMediaType* media_type_p = (struct _AMMediaType*)&(*iterator);
      Stream_MediaFramework_DirectShow_Tools::free (*media_type_p);
    } // end FOR
  } // end IF
  else if (Common_Tools::equalType ((MediaFormatType*)NULL, (IMFMediaType**)NULL))
  { clear_b = true;
    for (MEDIAFORMATS_ITERATOR_T iterator = formats.begin ();
         iterator != formats.end ();
         ++iterator)
    { // *NOTE*: this should be safe because of the check above
      IMFMediaType* media_type_p = *(IMFMediaType**)&(*iterator);
      media_type_p->Release ();
    } // end FOR
  } // end ELSE IF
  else
    formats = rhs_in.formats;
  if (clear_b)
    formats.clear ();
  // *CONSIDER*: use template specialization instead ?
  if (Common_Tools::equalType ((MediaFormatType*)NULL, (struct _AMMediaType*)NULL))
    for (MEDIAFORMATS_CONST_ITERATOR_T iterator = rhs_in.formats.begin ();
         iterator != rhs_in.formats.end ();
         ++iterator)
    { // *NOTE*: this should be safe because of the check above
      struct _AMMediaType* media_type_p = (struct _AMMediaType*)&(*iterator);
      struct _AMMediaType* media_type_2 =
        Stream_MediaFramework_DirectShow_Tools::copy (*media_type_p);
      formats.push_back (*(MediaFormatType*)media_type_2);
    } // end FOR
  else if (Common_Tools::equalType ((MediaFormatType*)NULL, (IMFMediaType**)NULL))
    for (MEDIAFORMATS_CONST_ITERATOR_T iterator = rhs_in.formats.begin ();
         iterator != rhs_in.formats.end ();
         ++iterator)
    { // *NOTE*: this should be safe because of the check above
      IMFMediaType* media_type_p = *(IMFMediaType**)&(*iterator);
      media_type_p->AddRef ();
      formats.push_back (*(MediaFormatType*)&media_type_p);
    } // end FOR
#else
  formats = rhs_in.formats;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework = rhs_in.mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  state = rhs_in.state;
  statistic = rhs_in.statistic;
  sourceFileName = rhs_in.sourceFileName;
  targetFileName = rhs_in.targetFileName;
  userData = (userData ? userData : rhs_in.userData);

  return *this;
}

template <typename BaseType,
          typename MediaFormatType,
          typename StreamStateType,
          typename StatisticType,
          typename UserDataType>
void
Stream_SessionDataMediaBase_T<BaseType,
                              MediaFormatType,
                              StreamStateType,
                              StatisticType,
                              UserDataType>::clear ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionDataMediaBase_T::clear"));

#if defined (FFMPEG_SUPPORT)
  for (std::map<enum AVCodecID, struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration>::const_iterator iterator = codecConfiguration.begin ();
       iterator != codecConfiguration.end ();
       ++iterator)
    delete[] (*iterator).second.data;
  codecConfiguration.clear ();
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *CONSIDER*: use template specialization instead ?
  if (Common_Tools::equalType ((MediaFormatType*)NULL, (struct _AMMediaType*)NULL))
    for (MEDIAFORMATS_ITERATOR_T iterator = formats.begin ();
         iterator != formats.end ();
         ++iterator)
    { // *NOTE*: this should be safe because of the check above
      struct _AMMediaType* media_type_p = (struct _AMMediaType*)&(*iterator);
      Stream_MediaFramework_DirectShow_Tools::free (*media_type_p);
    } // end FOR
  else if (Common_Tools::equalType ((MediaFormatType*)NULL, (IMFMediaType**)NULL))
    for (MEDIAFORMATS_ITERATOR_T iterator = formats.begin ();
         iterator != formats.end ();
         ++iterator)
    { // *NOTE*: this should be safe because of the check above
      IMFMediaType* media_type_p = *(IMFMediaType**)&(*iterator);
      media_type_p->Release ();
    } // end FOR
#endif // ACE_WIN32 || ACE_WIN64
  formats.clear ();

  statistic.reset ();

  inherited::clear ();
}

//////////////////////////////////////////

//template <typename DataType>
//Stream_SessionData_T<DataType>::Stream_SessionData_T ()
// : inherited (1,    // initial count
//              true) // delete 'this' on zero ?
// , data_ (NULL)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::Stream_SessionData_T"));

//}
template <typename DataType>
Stream_SessionData_T<DataType>::Stream_SessionData_T (DataType*& data_inout,
                                                      bool deleteDataInDtor_in)
 : inherited (1,    // initial count
              true) // delete 'this' on zero ?
 , data_ (data_inout)
 , delete_ (deleteDataInDtor_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::Stream_SessionData_T"));

  data_inout = NULL;
}

template <typename DataType>
Stream_SessionData_T<DataType>::~Stream_SessionData_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::~Stream_SessionData_T"));

  if (likely (data_ && delete_))
    delete data_;
}

template <typename DataType>
bool
Stream_SessionData_T<DataType>::lock (bool block_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::lock"));

  int result = -1;

  result = (block_in ? inherited::lock_.acquire ()
                     : inherited::lock_.tryacquire ());
  if (unlikely (result == -1))
  {
    if (block_in)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
    else
    { int error = ACE_OS::last_error ();
      if (error != EBUSY)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SYNCH_MUTEX::tryacquire(): \"%m\", aborting\n")));
    } // end ELSE
  } // end IF

  return (result == 0);
}

template <typename DataType>
const DataType&
Stream_SessionData_T<DataType>::getR () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::getR"));

  if (likely (data_))
    return *data_;

  static DataType dummy;
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (dummy);
  ACE_NOTREACHED (return dummy;)
}
template <typename DataType>
void
Stream_SessionData_T<DataType>::setR (const DataType& data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::setR"));

  // sanity check(s)
  ACE_ASSERT (data_);

  *data_ += data_in;
}

//template <typename DataType>
//void
//Stream_SessionData_T<DataType>::setP (DataType* data_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::set"));

//  // sanity check(s)
//  ACE_ASSERT (data_in);

//  // clean up
//#if defined (_DEBUG)
//  if (unlikely (data_ && (inherited::refcount_ != 1)))
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("resetting session (id: %d) data on-the-fly\n"),
//                data_->sessionId));
//#endif // _DEBUG
//  if (likely (data_))
//    delete data_;

//  data_ = data_in;
//}

template <typename DataType>
Stream_SessionData_T<DataType>*
Stream_SessionData_T<DataType>::clone () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::clone"));

  OWN_TYPE_T* result_p = NULL;

  DataType* data_p = NULL;
  if (data_)
  {
    ACE_NEW_NORETURN (data_p,
                      DataType (*data_)); // try to copy-construct
    if (unlikely (!data_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, aborting\n")));
      return NULL;
    } // end IF
  } // end IF

  ACE_NEW_NORETURN (result_p,
                    OWN_TYPE_T (data_p));
  if (unlikely (!result_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    if (data_p)
      delete data_p;
    return NULL;
  } // end IF

  return result_p;
}

template <typename DataType>
void
Stream_SessionData_T<DataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionData_T::dump_state"));

  // sanity check(s)
  ACE_ASSERT (data_);

  // *TODO*: remove type inferences
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("user data: %@, start of session: %s%s\n"),
              data_->userData,
              ACE_TEXT (Common_Timer_Tools::timeStampToLocalString (data_->startOfSession).c_str ()),
              (data_->aborted ? ACE_TEXT (" [aborted]") : ACE_TEXT (""))));
}
