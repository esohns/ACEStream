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
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
                                                  ValueType>::Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_CHANNELS,
               STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE,
               STREAM_VIS_SPECTRUMANALYZER_DEFAULT_SAMPLE_RATE)
 , bufferedSamples_ (0)
 , CBData_ ()
 , channelFactor_ (0.0)
#if GTK_CHECK_VERSION (4,0,0)
 , drawingContext_ (NULL)
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
 , cairoRegion_ (NULL)
#endif // GTK_CHECK_VERSION (3,22,0)
 , scaleFactorX_ (0.0)
 , scaleFactorX_2 (0.0)
 , scaleFactorY_ (0.0)
 , scaleFactorY_2 (0.0)
 , halfHeight_ (0)
 , height_ (0)
 , width_ (0)
 //, queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
 //          NULL)                   // notification handle
 //, renderHandler_ (this)
 //, renderHandlerTimerId_ (-1)
 , sampleIterator_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T"));

  ACE_OS::memset (&CBData_, 0, sizeof (struct acestream_visualization_gtk_cairo_cbdata));
  //inherited::msg_queue (&queue_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
                                                  ValueType>::~Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::~Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T"));

  if (unlikely (CBData_.context))
    cairo_destroy (CBData_.context);

#if GTK_CHECK_VERSION (4,0,0)
  if (drawingContext_)
    g_object_unref (drawingContext_);
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
  if (cairoRegion_)
    cairo_region_destroy (cairoRegion_);
#endif // GTK_CHECK_VERSION (3,22,0)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
bool
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
                                                  ValueType>::initialize (const ConfigurationType& configuration_in,
                                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
#if GTK_CHECK_VERSION (4,0,0)
    if (drawingContext_)
    {
      g_object_unref (drawingContext_); drawingContext_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
    if (cairoRegion_)
    {
      cairo_region_destroy (cairoRegion_); cairoRegion_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION (3,22,0)

    // (re-)activate() the message queue
    // *NOTE*: as this is a 'passive' object, the queue needs to be explicitly
    //         (re-)activate()d (see below)
//    ACE_ASSERT (inherited::msg_queue_);
//    inherited::msg_queue_->deactivate ();

    bufferedSamples_ = 0;
    if (unlikely (CBData_.context))
      cairo_destroy (CBData_.context);
    ACE_OS::memset (&CBData_, 0, sizeof (struct acestream_visualization_gtk_cairo_cbdata));

    channelFactor_ = 0.0;
    scaleFactorX_ = 0.0;
    scaleFactorX_2 = 0.0;
    scaleFactorY_ = 0.0;
    scaleFactorY_2 = 0.0;
    halfHeight_ = 0;
    height_ = width_ = 0;
  } // end IF

  // initialize cairo context
  // *TODO*: remove type inferences
  if (!configuration_in.window)
  {
    // sanity check(s)
    if (unlikely (!Common_UI_GTK_Tools::GTKInitialized &&
                  !Common_UI_GTK_Tools::initialize (0, NULL)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_UI_GTK_Tools::initialize(), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF

    Common_Image_Resolution_t resolution_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    resolution_s.cy = STREAM_VIS_DEFAULT_WINDOW_HEIGHT;
    resolution_s.cx = STREAM_VIS_DEFAULT_WINDOW_WIDTH;
#else
    resolution_s.height = STREAM_VIS_DEFAULT_WINDOW_HEIGHT;
    resolution_s.width = STREAM_VIS_DEFAULT_WINDOW_WIDTH;
#endif // ACE_WIN32 || ACE_WIN64
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
    if (unlikely (!inherited::initialize_GTK (resolution_s)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Vis_GTK_Window_T::initialize_GTK(), aborting\n"),
                  inherited::mod_->name ()));
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
      return false;
    } // end IF
    ACE_ASSERT (inherited::window_);
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    CBData_.dispatch = this;
    CBData_.resizeNotification = this;
  } // end IF
  else
  {
    CBData_.window = configuration_in.window;

#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
#if GTK_CHECK_VERSION (4,0,0)
    width_ = gdk_surface_get_width (configuration_in.window);
    height_ = gdk_surface_get_height (configuration_in.window);
#elif GTK_CHECK_VERSION (3,0,0)
    gdk_window_get_geometry (configuration_in.window,
                             NULL,
                             NULL,
                             &width_,
                             &height_);
#elif GTK_CHECK_VERSION (2,0,0)
    gdk_window_get_geometry (configuration_in.window,
                             NULL,
                             NULL,
                             &width_,
                             &height_,
                             NULL);
#endif /* GTK_CHECK_VERSION (x,0,0) */

    ACE_ASSERT (CBData_.window);
    if (unlikely (!initialize_Cairo (CBData_.window,
                                     CBData_.context)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo(), aborting\n"),
                  inherited::mod_->name ()));
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
      return false;
    } // end IF

#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    ACE_ASSERT (height_ && width_);
    halfHeight_ = height_ / 2;
  } // end ELSE

  //ACE_ASSERT (inherited::msg_queue_);
  //int result = inherited::msg_queue_->activate ();
  //if (unlikely (result == -1))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
  //              inherited::mod_->name ()));
  //  return false;
  //} // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
void
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
                                                  ValueType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->spectrumAnalyzerConfiguration);

  // step1: process inbound samples
  // *NOTE*: a 'data sample' consists of #channel 'sound sample's, which may
  //         arrive interleaved (i.e. (16 bit resolution) LLRRLLRRLLRR...), or
  //         'chunked' (i.e. LL...LLRR...RR...|LL...LLRR...RR...|...) at some
  //         interval. Note how 'chunked' data may need to be split/assembled
  //         before processing
  // *NOTE*: n-byte (n>1) 'sound sample's may have non-platform endian encoding

  // *TODO*: the current implementation assumes that all multi-channel data is
  //         interleaved like so: LR...LR... and arrives unfragmented, i.e. that
  //         inbound messages contain only complete samples. However, this
  //         largely depends on upstream configuration (i.e. data may be
  //         arriving over the network) and the hardware/driver/source module
  //         implementation
  //ACE_ASSERT (!(message_inout->length () % sampleIterator_.dataSampleSize_));
  //ACE_ASSERT (!(message_inout->length () % sampleIterator_.soundSampleSize_));

  unsigned int number_of_samples = 0;
  unsigned int samples_to_write = 0;
  unsigned int offset = 0;
  unsigned int tail_slot = 0;
  ACE_Message_Block* message_block_p = message_inout;
  bool compute_fft_b =
    inherited::configuration_->spectrumAnalyzerConfiguration->mode == STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM;

next:
  number_of_samples =
    message_block_p->length () / sampleIterator_.dataSampleSize_;

  do
  {
    samples_to_write = std::min (inherited2::slots_, number_of_samples);
//    bufferedSamples_ += samples_to_write;
    sampleIterator_.buffer_ =
      reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ()) + offset;
    for (unsigned int i = 0; i < inherited2::channels_; ++i)
    {
      // step1a: copy the inbound sample data into the buffer array
      // *TODO*: in principle, this step can be avoided by keeping track of the
      //         'current' slot (see also: Common_Math_FFT::CopyIn())

      // make space for inbound samples at the end of the buffer,
      // shifting previous samples towards the beginning
      tail_slot = inherited2::slots_ - samples_to_write;
      ACE_OS::memmove (&(inherited2::buffer_[i][0]),
                       &(inherited2::buffer_[i][samples_to_write]),
                       tail_slot * sizeof (ValueType));

      // copy the sample data to the tail end of the buffer as ValueType
      for (unsigned int j = 0; j < samples_to_write; ++j)
        inherited2::buffer_[i][tail_slot + j] = sampleIterator_.get (j, i);

      // apply window function ?
      if (unlikely (inherited::configuration_->spectrumAnalyzerConfiguration->applyWindowFunction))
        for (unsigned int j = 0; j < samples_to_write; ++j)
        { // --> 'Hamming'-window
          ValueType factor =
            (0.54 - 0.46 * std::cos ((2.0 * M_PI * (tail_slot + j)) / static_cast<ValueType> (inherited2::slots_)));
          inherited2::buffer_[i][tail_slot + j] *= factor;
        } // end FOR

      // step1b: process sample data ?
      if (compute_fft_b)
      {
        // initialize the FFT working set buffer, transform to complex
        for (unsigned int j = 0; j < inherited2::slots_; ++j)
          inherited2::X_[i][inherited2::bitReverseMap_[j]] = std::complex<ValueType> (inherited2::buffer_[i][j], 0);

//        if (bufferedSamples_ >= inherited2::slots_)
//        {
          // compute FFT
        inherited2::Compute (i);
//        } // end IF
      } // end IF
    } // end FOR
//    if (bufferedSamples_ >= inherited2::slots_)
//      bufferedSamples_ -= inherited2::slots_;

    offset += (sampleIterator_.dataSampleSize_ * samples_to_write);
    number_of_samples -= samples_to_write;
    if (!number_of_samples)
    {
      message_block_p = message_block_p->cont ();
      if (message_block_p)
      {
        offset = 0;
        goto next;
      } // end IF
    } // end IF
  } while (message_block_p);

  //if (compute_fft_b)
  //  inherited2::ComputeMaxValue ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
void
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
                                                  ValueType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->spectrumAnalyzerConfiguration);
      ACE_ASSERT (inherited::sessionData_);
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());

      typename TimerManagerType::INTERFACE_T* itimer_manager_p = NULL;
      bool result_2 = false;
      bool shutdown = false;

      unsigned int data_sample_size = 0;
      unsigned int sound_sample_size = 0;
      unsigned int channels, sample_rate;
      int sample_byte_order = ACE_BYTE_ORDER;
      bool is_signed_format = false;
      bool is_floating_point_format = false;
      double max_value_d = 0.0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_AUDIO,
                               media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);
      sound_sample_size = (waveformatex_p->wBitsPerSample / 8);
      //data_sample_size = waveformatex_p->nBlockAlign;
      data_sample_size = waveformatex_p->nChannels * sound_sample_size;
      // *NOTE*: apparently, all Win32 sound data is little endian only
      sample_byte_order = ACE_LITTLE_ENDIAN;
      // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
      //         are unsigned values. (Each audio sample has the range 0255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      is_signed_format = !(sound_sample_size == 1);
      is_floating_point_format =
        Stream_MediaFramework_DirectSound_Tools::isFloat (*waveformatex_p);

      channels = waveformatex_p->nChannels;
      sample_rate = waveformatex_p->nSamplesPerSec;

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_AUDIO,
                               media_type_s);
      sound_sample_size = (snd_pcm_format_width (media_type_s.format) / 8);
      data_sample_size = sound_sample_size * media_type_s.channels;
      sample_byte_order =
        ((snd_pcm_format_little_endian (media_type_s.format) == 1) ? ACE_LITTLE_ENDIAN
                                                                   : -1);
      is_signed_format = (snd_pcm_format_signed (media_type_s.format) == 1);
      is_floating_point_format =
        (snd_pcm_format_linear (media_type_s.format) == 0);

      channels = media_type_s.channels;
      sample_rate = media_type_s.rate;
#endif // ACE_WIN32 || ACE_WIN64
      result_2 = sampleIterator_.initialize (data_sample_size,
                                             sound_sample_size,
                                             is_signed_format,
                                             is_floating_point_format,
                                             sample_byte_order);
      if (unlikely (!result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize sample iterator, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      result_2 =
        inherited2::Initialize (channels,
                                inherited::configuration_->spectrumAnalyzerConfiguration->resolution,
                                sample_rate);
      if (unlikely (!result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_Math_FFT::initialize(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      channelFactor_ = width_ / static_cast<double> (inherited2::channels_);
      scaleFactorX_ =
        width_ / static_cast<double> (inherited2::channels_ * inherited2::slots_);
      scaleFactorX_2 =
        width_ / static_cast<double> (inherited2::channels_ * ((inherited2::slots_ / 2) - 1));
      max_value_d = static_cast<double> (Common_Tools::max<ACE_UINT64> (sound_sample_size, is_signed_format));
      scaleFactorY_ =
        (is_floating_point_format ? static_cast<double> (height_)
                                  : static_cast<double> (height_) / (sampleIterator_.isSignedSampleFormat_ ? max_value_d * 2.0
                                                                                                           : max_value_d));
      scaleFactorY_2 = scaleFactorY_ * 2.0;

      // schedule the renderer
      //ACE_ASSERT (renderHandlerTimerId_ == -1);
      //itimer_manager_p =
      //    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
      //                                             : TIMER_MANAGER_SINGLETON_T::instance ());
      //ACE_ASSERT (itimer_manager_p);
      //// schedule the second-granularity timer
      ////ACE_Time_Value refresh_interval (0, 1000000 / inherited::configuration_->fps);
      //ACE_Time_Value refresh_interval (0,
      //                                 1000000 / STREAM_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE);
      //renderHandlerTimerId_ =
      //  itimer_manager_p->schedule_timer (&renderHandler_,                    // event handler handle
      //                                    NULL,                               // asynchronous completion token
      //                                    COMMON_TIME_NOW + refresh_interval, // first wakeup time
      //                                    refresh_interval);                  // interval
      //if (unlikely (renderHandlerTimerId_ == -1))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
      //              inherited::mod_->name (),
      //              &refresh_interval));
      //  goto error;
      //} // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: scheduled renderer dispatch (timer id: %d)\n"),
      //            inherited::mod_->name (),
      //            renderHandlerTimerId_));

      inherited::threadCount_ =
        //(inherited::window_ ? 2 : 1); // mainloop + renderer : renderer only
        (inherited::window_ ? 1 : 0); // mainloop : N/A
      if (inherited::threadCount_)
        inherited::start (NULL);
      inherited::threadCount_ = 0;
      shutdown = true;

      break;

error:
      //if (renderHandlerTimerId_ != -1)
      //{
      //  // sanity check(s)
      //  ACE_ASSERT (itimer_manager_p);

      //  const void* act_p = NULL;
      //  result = itimer_manager_p->cancel_timer (renderHandlerTimerId_,
      //                                           &act_p);
      //  if (unlikely (result <= 0))
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
      //                inherited::mod_->name (),
      //                renderHandlerTimerId_));
      //  else
      //    ACE_DEBUG ((LM_DEBUG,
      //                ACE_TEXT ("%s: cancelled renderer dispatch (timer id: %d)\n"),
      //                inherited::mod_->name (),
      //                renderHandlerTimerId_));
      //  renderHandlerTimerId_ = -1;
      //} // end IF

      if (shutdown)
      {
        //inherited::control (ACE_Message_Block::MB_STOP);
        if (inherited::window_)
        {
#if GTK_CHECK_VERSION (3,6,0)
#else
          gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
          //if (inherited::thr_count_ == 2)
#if GTK_CHECK_VERSION (3,10,0)
          gtk_window_close (inherited::window_); inherited::window_ = NULL;
#else
          gtk_widget_destroy (GTK_WIDGET (inherited::window_)); inherited::window_ = NULL;
#endif // GTK_CHECK_VERSION (3,10,0)
          gtk_main_quit ();
#if GTK_CHECK_VERSION (3,6,0)
#else
          gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
        } // end IF
      } // end IF

      inherited::TASK_BASE_T::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      //if (likely (renderHandlerTimerId_ != -1))
      //{
      //  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
      //      (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
      //                                               : TIMER_MANAGER_SINGLETON_T::instance ());
      //  ACE_ASSERT (itimer_manager_p);
      //  const void* act_p = NULL;
      //  result = itimer_manager_p->cancel_timer (renderHandlerTimerId_,
      //                                           &act_p);
      //  if (unlikely (result <= 0))
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
      //                inherited::mod_->name (),
      //                renderHandlerTimerId_));
      //  else
      //    ACE_DEBUG ((LM_DEBUG,
      //                ACE_TEXT ("%s: cancelled renderer dispatch (timer id: %d)\n"),
      //                inherited::mod_->name (),
      //                renderHandlerTimerId_));
      //  renderHandlerTimerId_ = -1;
      //} // end IF

      if (inherited::window_)
      {
#if GTK_CHECK_VERSION (3,6,0)
#else
        gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
        //if (inherited::thr_count_ == 2)
        if (inherited::thr_count_ > 0)
        {
#if GTK_CHECK_VERSION (3,10,0)
          gtk_window_close (inherited::window_); inherited::window_ = NULL;
#else
          gtk_widget_destroy (GTK_WIDGET (inherited::window_)); inherited::window_ = NULL;
#endif // GTK_CHECK_VERSION (3,10,0)
          gtk_main_quit ();
        } // end IF
#if GTK_CHECK_VERSION (3,6,0)
#else
        gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
      } // end IF
//      if (likely (inherited::mainLoop_ &&
//                  g_main_loop_is_running (inherited::mainLoop_)))
//        g_main_loop_quit (inherited::mainLoop_);

      // *IMPORTANT NOTE*: at this stage no new data should be arriving
      //                   --> join with the renderer thread
      //if (inherited::thr_count_ > 0)
      //{
      //  Common_ITask* itask_p = this;
      //  itask_p->stop (true,   // wait ?
      //                 false); // high priority ?
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("%s: joined renderer thread\n"),
      //              inherited::mod_->name ()));
      //} // end IF

//      if (likely (inherited::mainLoop_))
//      {
//        g_main_loop_unref (inherited::mainLoop_); inherited::mainLoop_ = NULL;
//      } // end IF

      if (likely (CBData_.context))
        cairo_destroy (CBData_.context);
      ACE_OS::memset (&CBData_, 0, sizeof (struct acestream_visualization_gtk_cairo_cbdata));

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
int
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
                                                  ValueType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_, NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_, 0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64

  static bool is_first_b = true;
  bool is_first_2 = false;
  int result = 0;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  int error = 0;

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    if (is_first_b)
    {
      is_first_b = false;
      is_first_2 = true; // *TODO*: there must be a better way to do this...
    } // end IF
  } // end lock scope
  if (is_first_2 && inherited::window_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: gtk dispatch thread (id: %t, group: %d) starting\n"),
                inherited::mod_->name (),
                inherited::grp_id_));

#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
    GtkWidget* widget_p = GTK_WIDGET (inherited::window_);

    // *NOTE*: GTK3 seems not to support drawing on a window itself
    //         --> draw into a drawing area instead
    // *TODO*: GTK2 does support drawing on a window, but resizing it does not
    //         extend the canvas...
#if GTK_CHECK_VERSION (3,0,0)
    GtkWidget* box_p = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (inherited::window_), box_p);
    GtkWidget* drawing_area_p = gtk_drawing_area_new ();
    gtk_box_pack_start (GTK_BOX (box_p), drawing_area_p, TRUE, TRUE, 0);
    widget_p = drawing_area_p;
#endif // GTK_CHECK_VERSION (3,0,0)
    gtk_widget_set_app_paintable (widget_p, TRUE);
    gtk_widget_set_double_buffered (widget_p, FALSE);
    gtk_widget_show_all (GTK_WIDGET (inherited::window_));

#if GTK_CHECK_VERSION (4,0,0)
    GtkNative* native_p = gtk_widget_get_native (widget_p);
    ACE_ASSERT (native_p);
    CBData_.window = gtk_native_get_surface (native_p);
#else
    CBData_.window = gtk_widget_get_window (widget_p);
#endif // GTK_CHECK_VERSION (4,0,0)
    ACE_ASSERT (CBData_.window);
    if (unlikely (!initialize_Cairo (CBData_.window,
                                     CBData_.context)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo(), aborting\n"),
                  inherited::mod_->name ()));
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
      result = -1;
      goto done;
    } // end IF
    setP (CBData_.window);

    gulong result_3 =
#if GTK_CHECK_VERSION(3,0,0)
      g_signal_connect (G_OBJECT (widget_p),
                        ACE_TEXT_ALWAYS_CHAR ("draw"),
                        G_CALLBACK (acestream_visualization_gtk_cairo_draw_cb),
                        &CBData_);
    ACE_ASSERT (result_3);
#else
      g_signal_connect (G_OBJECT (widget_p),
                        ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                        G_CALLBACK (acestream_visualization_gtk_cairo_expose_event_cb),
                        &CBData_);
    ACE_ASSERT (result_3);
//    result_3 =
//      g_signal_connect (G_OBJECT (widget_p),
//                        ACE_TEXT_ALWAYS_CHAR ("configure-event"),
//                        G_CALLBACK (acestream_visualization_gtk_cairo_configure_event_cb),
//                        &CBData_);
//    ACE_ASSERT (result_3);
#endif // GTK_CHECK_VERSION(3,0,0)
    result_3 =
      g_signal_connect (G_OBJECT (widget_p),
                        ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                        G_CALLBACK (acestream_visualization_gtk_cairo_size_allocate_cb),
                        &CBData_);
    ACE_ASSERT (result_3);

    g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                   acestream_visualization_gtk_cairo_idle_update_cb,
                   &CBData_);
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

    result = inherited::svc ();

    goto done;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              inherited::grp_id_));

  // process update events
  do
  {
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p, NULL);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        message_block_p->release (); message_block_p = NULL;
        goto done;
      }
      case ACE_Message_Block::MB_EVENT:
      {
        dispatch (&CBData_);
        // *WARNING*: control falls through
        ACE_FALLTHROUGH;
      }
      default:
      {
        message_block_p->release (); message_block_p = NULL;
        break;
      }
    } // end SWITCH
  } while (true);
  result = -1;

done:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %s thread (id: %t, group: %d) leaving\n"),
              ((is_first_2 && inherited::window_) ? ACE_TEXT ("gtk dispatch") : ACE_TEXT ("renderer")),
              inherited::mod_->name (),
              inherited::grp_id_));

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
bool
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
#if GTK_CHECK_VERSION (4,0,0)
                                                  ValueType>::initialize_Cairo (GdkSurface* window_in,
#else
                                                  ValueType>::initialize_Cairo (GdkWindow* window_in,
#endif // GTK_CHECK_VERSION (4,0,0)
                                                                                cairo_t*& cairoContext_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo"));

  // sanity check(s)
  ACE_ASSERT (window_in);
  ACE_ASSERT (!cairoContext_out);

#if GTK_CHECK_VERSION (3,22,0)
  if (cairoRegion_)
  {
    cairo_region_destroy (cairoRegion_); cairoRegion_ = NULL;
  } // end IF
#endif // GTK_CHECK_VERSION (3,22,0)

#if GTK_CHECK_VERSION (4,0,0)
  if (drawingContext_)
  {
    g_object_unref (drawingContext_); drawingContext_ = NULL;
  } // end IF

  drawingContext_ = gdk_surface_create_cairo_context (window_in);
  if (unlikely (!drawingContext_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_surface_create_cairo_context(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  cairoContext_out = gdk_drawing_context_get_cairo_context (drawingContext_);
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
  cairoRegion_ = cairo_region_create ();
  ACE_ASSERT (cairoRegion_);
#else
  cairoContext_out = gdk_cairo_create (window_in);
  if (unlikely (!cairoContext_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_cairo_create(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
#endif // GTK_CHECK_VERSION ()

  if (cairoContext_out)
  {
    //cairo_set_line_cap (cairoContext_out, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_width (cairoContext_out, 1.0);
    //cairo_set_line_join (cairoContext_out, CAIRO_LINE_JOIN_MITER);
    //cairo_set_dash (cairoContext_out, NULL, 0, 0.0);
  } // end IF

  return true;
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename TimerManagerType,
//          typename MediaType,
//          typename ValueType>
//void
//Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
//                                                  TimePolicyType,
//                                                  ConfigurationType,
//                                                  ControlMessageType,
//                                                  DataMessageType,
//                                                  SessionMessageType,
//                                                  SessionDataType,
//                                                  SessionDataContainerType,
//                                                  TimerManagerType,
//                                                  MediaType,
//                                                  ValueType>::reset ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::reset"));
//
//  // trigger a render update
//  // *NOTE*: (as long as it is single thread-based,) rendering a frame creates
//  //         too much workload for the timer dispatch context and delays the
//  //         dispatch of (relatively more important other) scheduled tasks
//  //         --> avoid 'laggy' applications
//  // *TODO*: depending on the platform (and the timer dispatch 'mode'), this may
//  //         be unnecessary (i.e. if the timer mechanism is signal-handler
//  //         based (, or the timer dispatch uses a thread pool itself))
//  int result = -1;
//  ACE_Message_Block* message_block_p = NULL;
//  ACE_NEW_NORETURN (message_block_p,
//                    ACE_Message_Block (0,                                  // size
//                                       ACE_Message_Block::MB_EVENT,        // type
//                                       NULL,                               // continuation
//                                       NULL,                               // data
//                                       NULL,                               // buffer allocator
//                                       NULL,                               // locking strategy
//                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
//                                       ACE_Time_Value::zero,               // execution time
//                                       ACE_Time_Value::max_time,           // deadline time
//                                       NULL,                               // data block allocator
//                                       NULL));                             // message allocator
//  if (unlikely (!message_block_p))
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
//                inherited::mod_->name ()));
//    return;
//  } // end IF
//
//  result = this->putq (message_block_p, NULL);
//  if (unlikely (result == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ACE_Task_Base::putq(): \"%m\", continuing\n"),
//                inherited::mod_->name ()));
//    message_block_p->release (); message_block_p = NULL;
//  } // end IF
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
void
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
#if GTK_CHECK_VERSION (4,0,0)
                                                  ValueType>::setP (GdkSurface* window_in)
#else
                                                  ValueType>::setP (GdkWindow* window_in)
#endif // GTK_CHECK_VERSION (4,0,0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::setP"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (!inherited::sessionData_)
    return;
  ACE_ASSERT (window_in);

//  int result = -1;
//  bool release_lock = false;
  unsigned int sound_sample_size = 0;
  bool is_floating_point_format = false;
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
  ACE_ASSERT (!session_data_r.formats.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited::getMediaType (session_data_r.formats.back (),
                           STREAM_MEDIATYPE_AUDIO,
                           media_type_s);
  ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
  ACE_ASSERT (waveformatex_p);
  sound_sample_size = waveformatex_p->wBitsPerSample / 8;
  is_floating_point_format =
    Stream_MediaFramework_DirectSound_Tools::isFloat (*waveformatex_p);
  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
  struct Stream_MediaFramework_ALSA_MediaType media_type_s;
  inherited::getMediaType (session_data_r.formats.back (),
                           STREAM_MEDIATYPE_AUDIO,
                           media_type_s);
  sound_sample_size = (snd_pcm_format_width (media_type_s.format) / 8);
  is_floating_point_format =
    (snd_pcm_format_linear (media_type_s.format) == 0);
#endif // ACE_WIN32 || ACE_WIN64

  //{ ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    //if (likely (CBData_.context))
    //{
    //  cairo_destroy (CBData_.context); CBData_.context = NULL;
    //} // end IF
    //if (unlikely (!initialize_Cairo (window_in,
    //                                 CBData_.context)))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("%s: failed to initialize_Cairo(), returning\n"),
    //              inherited::mod_->name ()));
    //  return;
    //} // end IF
    //ACE_ASSERT (CBData_.context);
    //CBData_.window = window_in;

#if GTK_CHECK_VERSION (4,0,0)
   width_ = gdk_surface_get_width (window_in);
   height_ = gdk_surface_get_height (window_in);
#elif GTK_CHECK_VERSION (3,0,0)
   gdk_window_get_geometry (window_in,
                            NULL,
                            NULL,
                            &width_,
                            &height_);
#elif GTK_CHECK_VERSION (2,0,0)
  gdk_window_get_geometry (window_in,
                            NULL,
                            NULL,
                            &width_,
                            &height_,
                            NULL);
#endif /* GTK_CHECK_VERSION (x,0,0) */
  ACE_ASSERT (height_); ACE_ASSERT (width_);
  halfHeight_ = height_ / 2;

  channelFactor_ = width_ / static_cast<double> (inherited2::channels_);
  scaleFactorX_ =
    width_ / static_cast<double> (inherited2::channels_ * inherited2::slots_);
  scaleFactorX_2 =
    width_ / static_cast<double> (inherited2::channels_ * ((inherited2::slots_ / 2) - 1));

  double max_value_d =
    static_cast<double> (Common_Tools::max<ACE_UINT64> (sound_sample_size, sampleIterator_.isSignedSampleFormat_));
  scaleFactorY_ =
    (is_floating_point_format ? static_cast<double> (height_)
                              : static_cast<double> (height_) / (sampleIterator_.isSignedSampleFormat_ ? max_value_d * 2.0
                                                                                                       : max_value_d));
  scaleFactorY_2 = scaleFactorY_ * 2.0;
//} // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType,
          typename ValueType>
void
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType,
                                                  ValueType>::dispatch (void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::dispatch"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  struct acestream_visualization_gtk_cairo_cbdata* cbdata_p =
    static_cast<struct acestream_visualization_gtk_cairo_cbdata*> (userData_in);
  ACE_ASSERT (cbdata_p);
  ACE_ASSERT (cbdata_p->context);
  ACE_ASSERT (cbdata_p->window);

  cairo_t* context_p = NULL;
#if GTK_CHECK_VERSION (4,0,0)
  ACE_ASSERT (cairoRegion_);
  ACE_ASSERT (drawingContext_);
  gdk_draw_context_begin_frame (drawingContext_,
                                cairoRegion_);
  context_p = gdk_cairo_context_cairo_create (drawingContext_);
#elif GTK_CHECK_VERSION (3,22,0)
  ACE_ASSERT (cairoRegion_);
  GdkDrawingContext* drawing_context_p =
    gdk_window_begin_draw_frame (cbdata_p->window,
                                 cairoRegion_);
  ACE_ASSERT (drawing_context_p);
  context_p = gdk_drawing_context_get_cairo_context (drawing_context_p);
#elif GTK_CHECK_VERSION (3,10,0)
  cairo_surface_t* surface_p = NULL;
  context_p = cbdata_p->context;
#else
#define CAIRO_ERROR_WORKAROUND(X)                                \
  if (cairo_status (X) != CAIRO_STATUS_SUCCESS) {                \
    cairo_destroy (cbdata_p->context); cbdata_p->context = NULL; \
    cbdata_p->context = gdk_cairo_create (cbdata_p->window);     \
    ACE_ASSERT (cbdata_p->context);                              \
    cairo_set_line_width (cbdata_p->context, 1.0);               \
  } // end IF

  context_p = cbdata_p->context;
#endif // GTK_CHECK_VERSION ()
  ACE_ASSERT (context_p);

  // step1: clear the window(s)
  // *NOTE*: not required if the widget is double-buffered...
  cairo_set_source_rgb (context_p, 0.0, 0.0, 0.0);
  cairo_rectangle (context_p, 0.0, 0.0, width_, height_);
  cairo_fill (context_p);

  // step2a: draw signal graphics
  cairo_set_source_rgb (context_p, 0.0, 1.0, 0.0); // green
  for (unsigned int i = 0; i < inherited2::channels_; ++i)
  {
    switch (inherited::configuration_->spectrumAnalyzerConfiguration->mode)
    {
      case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE:
      {
        // step2aa: draw a thin polyline
        cairo_move_to (context_p,
                       static_cast<double> (i) * channelFactor_,
                       (sampleIterator_.isSignedSampleFormat_ ? static_cast<double> (halfHeight_)
                                                              : static_cast<double> (height_)));
        for (unsigned int j = 0; j < inherited2::slots_; ++j)
          cairo_line_to (context_p,
                         (static_cast<double> (i) * channelFactor_) + (static_cast<double> (j) * scaleFactorX_),
                         (sampleIterator_.isSignedSampleFormat_ ? static_cast<double> (halfHeight_) - (inherited2::buffer_[i][j] * scaleFactorY_)
                                                                : static_cast<double> (height_) - (inherited2::buffer_[i][j] * scaleFactorY_)));

        break;
      }
      case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM:
      {
        double x, y, magnitude_d;
        // step2aa: draw thin columns
        // *IMPORTANT NOTE*: - the first ('DC'-)slot does not contain frequency
        //                     information --> j = 1
        //                   - the slots N/2 - N are mirrored and do not contain
        //                     additional information
        //                     --> there are only N/2 - 1 meaningful values

        //static double logMinFreq_d = log10 (100.0); // <-- min frequency to display
        for (unsigned int j = 1; j < inherited2::halfSlots_; ++j)
        {
          // method2: logarithmic scale
          //x =
          //  (static_cast<double> (i) * channelFactor_) + 
          //  (log10 (static_cast<double> (inherited2::Frequency (j))) - logMinFreq_d) * ((width_ - 1) / 2.0) / (log10 (inherited2::sampleRate_ / 2.0) - logMinFreq_d);
          // method1: linear scale
          x =
            (static_cast<double> (i) * channelFactor_) + (static_cast<double> (j - 1) * scaleFactorX_2);
          cairo_move_to (context_p,
                         x, static_cast<double> (height_));

          // method2: dB scale
          // *TODO*: needs normalization
          //magnitude_d = 10.0 * log10 (inherited2::SqMagnitude (j, i, false));
          //y = magnitude_d;
          // method1: (normalized-) magnitude
          //magnitude_d = inherited2::Magnitude (j, i, true);
          //y = static_cast<double> (height_) * magnitude_d;
          magnitude_d = inherited2::Magnitude2 (j, i, false);
          y = sampleIterator_.isSignedSampleFormat_ ? magnitude_d * scaleFactorY_2
                                                    : magnitude_d * scaleFactorY_;
          cairo_line_to (context_p,
                         x, static_cast<double> (height_) - y);
        } // end FOR

        break;
      }
      case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid 2D mode (was: %d), returning\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->spectrumAnalyzerConfiguration->mode));
        goto error;
      }
    } // end SWITCH
    cairo_stroke (context_p);

    // step2ab: draw a thin line between channels
    if (i > 0)
    {
      cairo_set_source_rgb (context_p, 1.0, 0.0, 0.0); //  red
      cairo_move_to (context_p,
                     i * channelFactor_,
                     0);
      cairo_line_to (context_p,
                     i * channelFactor_,
                     height_);
      cairo_stroke (context_p);
      cairo_set_source_rgb (context_p, 0.0, 1.0, 0.0); // green
    } // end IF
  } // end FOR

#if GTK_CHECK_VERSION (4,0,0)
  gdk_draw_context_end_frame (drawingContext_);
#elif GTK_CHECK_VERSION (3,22,0)
  gdk_window_end_draw_frame (cbdata_p->window,
                             drawing_context_p);
#elif GTK_CHECK_VERSION (3,10,0)
  surface_p = cairo_get_target (context_p);
  ACE_ASSERT (surface_p);
  cairo_surface_mark_dirty (surface_p);
  //ACE_ASSERT (cairo_surface_status (surface_p) == CAIRO_STATUS_SUCCESS);
  cairo_surface_flush (surface_p);
  //ACE_ASSERT (cairo_surface_status (surface_p) == CAIRO_STATUS_SUCCESS);
#else
  // *IMPORTANT NOTE*: this assert fails intermittently on Gtk2 Win32;
  //                   the result is CAIRO_STATUS_NO_MEMORY
  //ACE_ASSERT (cairo_status (cairoContext_) == CAIRO_STATUS_SUCCESS);
  CAIRO_ERROR_WORKAROUND (cbdata_p->context);
#endif // GTK_CHECK_VERSION (3,22,0)

error:
  ;
}
