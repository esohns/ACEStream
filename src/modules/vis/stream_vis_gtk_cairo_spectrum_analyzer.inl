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
          typename MediaType>
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                  MediaType>::Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (ISTREAM_T* stream_in)
#else
                                                  MediaType>::Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_CHANNELS,
               STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE,
               STREAM_VIS_SPECTRUMANALYZER_DEFAULT_SAMPLE_RATE)
 , cairoContext_ (NULL)
// , surfaceLock_ (NULL)
//#if GTK_CHECK_VERSION(3,10,0)
// , cairoSurface_ (NULL)
//#else
// , pixelBuffer_ (NULL)
//#endif // GTK_CHECK_VERSION (3,10,0)
#if defined (GTKGL_SUPPORT)
 , backgroundColor_ ()
 , foregroundColor_ ()
#endif /* GTKGL_SUPPORT */
 , channelFactor_ (0.0)
 , scaleFactorX_ (0.0)
 , scaleFactorX_2 (0.0)
 , scaleFactorY_ (0.0)
 , halfHeight_ (0)
 , height_ (0)
 , width_ (0)
 , mode2D_ (NULL)
//#if defined (GTKGL_SUPPORT)
// , mode3D_ (NULL)
//#endif // GTKGL_SUPPORT
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
 , renderHandler_ (this)
 , renderHandlerTimerId_ (-1)
 , sampleIterator_ (NULL)
#if GTK_CHECK_VERSION(3,0,0)
 , randomDistribution_ (0, 255)
#else
 , randomDistribution_ (0, 65535)
#endif // GTK_CHECK_VERSION(3,0,0)
 , randomEngine_ ()
 , randomGenerator_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T"));

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,0,0)
  gboolean result_2 =
    gdk_rgba_parse (&backgroundColor_,
                    ACE_TEXT_ALWAYS_CHAR ("rgba (0, 0, 0, 1.0)"));       // opaque black
  ACE_ASSERT (result_2);
  result_2 =
    gdk_rgba_parse (&foregroundColor_,
                    ACE_TEXT_ALWAYS_CHAR ("rgba (255, 255, 255, 1.0)")); // opaque white
  ACE_ASSERT (result_2);
#else
  ACE_OS::memset (&backgroundColor_, 0, sizeof (struct _GdkColor));                            // opaque black
  foregroundColor_.pixel = 0;
  foregroundColor_.red = 65535; foregroundColor_.green = 65535; foregroundColor_.blue = 65535; // opaque white
#endif // GTK_CHECK_VERSION (3,0,0)
#endif // GTKGL_SUPPORT

  inherited::msg_queue (&queue_);

  randomGenerator_ = std::bind (randomDistribution_, randomEngine_);
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
          typename MediaType>
Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                                  TimePolicyType,
                                                  ConfigurationType,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  SessionDataType,
                                                  SessionDataContainerType,
                                                  TimerManagerType,
                                                  MediaType>::~Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::~Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T"));

//#if GTK_CHECK_VERSION(3,10,0)
//  if (unlikely (cairoSurface_))
//    cairo_surface_destroy (cairoSurface_);
//#else
//  if (unlikely (pixelBuffer_))
//    g_object_unref (pixelBuffer_);
//#endif // GTK_CHECK_VERSION(3,10,0)
  if (unlikely (cairoContext_))
    cairo_destroy (cairoContext_);
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
          typename MediaType>
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
                                                  MediaType>::initialize (const ConfigurationType& configuration_in,
                                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    // (re-)activate() the message queue
    // *NOTE*: as this is a 'passive' object, the queue needs to be explicitly
    //         (re-)activate()d (see below)
    inherited::msg_queue (NULL);

    //surfaceLock_ = NULL;
//#if GTK_CHECK_VERSION(3,10,0)
//    if (cairoSurface_)
//    {
//      cairo_surface_destroy (cairoSurface_); cairoSurface_ = NULL;
//    } // end IF
//#else
//    if (pixelBuffer_)
//    {
//      g_object_unref (pixelBuffer_); pixelBuffer_ = NULL;
//    } // end IF
//#endif // GTK_CHECK_VERSION (3,10,0)
    if (cairoContext_)
    {
      cairo_destroy (cairoContext_); cairoContext_ = NULL;
    } // end IF
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,0,0)
    gboolean result_2 =
      gdk_rgba_parse (&backgroundColor_,
                      ACE_TEXT_ALWAYS_CHAR ("rgba (0, 0, 0, 1.0)"));       // opaque black
    ACE_ASSERT (result_2);
    result_2 =
      gdk_rgba_parse (&foregroundColor_,
                      ACE_TEXT_ALWAYS_CHAR ("rgba (255, 255, 255, 1.0)")); // opaque white
    ACE_ASSERT (result_2);
#else
    ACE_OS::memset (&backgroundColor_, 0, sizeof (struct _GdkColor));                            // opaque black
    foregroundColor_.pixel = 0;
    foregroundColor_.red = 65535; foregroundColor_.green = 65535; foregroundColor_.blue = 65535; // opaque white
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

    channelFactor_ = 0.0;
    scaleFactorX_ = 0.0;
    scaleFactorX_2 = 0.0;
    scaleFactorY_ = 0.0;
    halfHeight_ = 0;
    height_ = width_ = 0;

    mode2D_ = NULL;
  } // end IF

  mode2D_ =
    &const_cast<ConfigurationType&> (configuration_in).spectrumAnalyzer2DMode;

  // initialize cairo ?
  if (!configuration_in.window)
    goto continue_;

  //surfaceLock_ = configuration_in.surfaceLock;

  // *TODO*: remove type inferences
  if (unlikely (!initialize_Cairo (configuration_in.window,
                                   cairoContext_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  gdk_window_get_geometry (configuration_in.window,
                           NULL,
                           NULL,
                           &width_,
                           &height_,
                           NULL);
  ACE_ASSERT (height_); ACE_ASSERT (width_);
  halfHeight_ = height_ / 2;

continue_:
  if (unlikely (!mode2D_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: no graphics output\n"),
                inherited::mod_->name ()));
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename TimerManagerType,
          typename MediaType>
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
                                                  MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

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

  unsigned int number_of_samples =
    message_inout->length () / sampleIterator_.dataSampleSize_;
  unsigned int samples_to_write = 0;
  unsigned int offset = 0;
  unsigned int tail_slot = 0;
  //sampleIterator_.buffer_ = NULL;

  // sanity check(s)
  ACE_ASSERT (mode2D_);

  do
  {
    samples_to_write = std::min (inherited3::slots_, number_of_samples);
    sampleIterator_.buffer_ = message_inout->rd_ptr () + offset;
    for (unsigned int i = 0; i < inherited3::channels_; ++i)
    {
      // step1a: copy the inbound sample data into the buffer array
      // *TODO*: in principle, this step can be avoided by keeping track of the
      //         'current' slot (see also: Common_Math_FFT::CopyIn())

      // make space for inbound samples at the end of the buffer,
      // shifting previous samples towards the beginning
      tail_slot = inherited3::slots_ - samples_to_write;
      ACE_OS::memmove (&(inherited3::buffer_[i][0]),
                       &(inherited3::buffer_[i][samples_to_write]),
                       tail_slot * sizeof (double));

      // copy the sample data to the tail end of the buffer, transform to double
      for (unsigned int j = 0; j < samples_to_write; ++j)
        inherited3::buffer_[i][tail_slot + j] = sampleIterator_.get (j, i);
      offset += (sampleIterator_.soundSampleSize_ * samples_to_write);

      // step1b: process sample data
      if (*mode2D_ > STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE)
      {
        // initialize the FFT working set buffer, transform to complex
        for (unsigned int j = 0; j < inherited3::slots_; ++j)
          X_[i][bitReverseMap_[j]] = std::complex<double> (buffer_[i][j], 0.0);

        // compute FFT
        inherited3::Compute (i);
      } // end IF
    } // end FOR
    number_of_samples -= samples_to_write;
    if (!number_of_samples)
      break; // done
  } while (true);
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
          typename MediaType>
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
                                                  MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
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
      //         are unsigned values. (Each audio sample has the range 0–255.)
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
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      sound_sample_size = (snd_pcm_format_width (media_type_s.format) / 8);
      data_sample_size = sound_sample_size * media_type_s.channels;
      sample_byte_order =
          ((snd_pcm_format_little_endian (media_type_s.format) == 1) ? ACE_LITTLE_ENDIAN
                                                                     : -1);
      is_signed_format = snd_pcm_format_signed (media_type_s.format);
      is_floating_point_format =
        snd_pcm_format_real (media_type_s.format);

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
        inherited3::Initialize (channels,
                                inherited::configuration_->spectrumAnalyzerResolution,
                                sample_rate);
      if (unlikely (!result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_Math_FFT::initialize(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      channelFactor_ = width_ / static_cast<double> (inherited3::channels_);
      scaleFactorX_ =
        width_ / static_cast<double> (inherited3::channels_ * inherited3::slots_);
      scaleFactorX_2 =
        width_ / static_cast<double> (inherited3::channels_ * ((inherited3::slots_ / 2) - 1));
      scaleFactorY_ =
        (is_floating_point_format ? static_cast<double> (height_) / 2.0
                                  : static_cast<double> (height_) / static_cast<double> (Common_Tools::max<ACE_UINT64> (sound_sample_size, false)));

      // schedule the renderer
      if (inherited::configuration_->fps)
      {
        result = inherited::msg_queue_->activate ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF

        ACE_ASSERT (renderHandlerTimerId_ == -1);
        itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        // schedule the second-granularity timer
        ACE_Time_Value refresh_interval (0, 1000000 / inherited::configuration_->fps);
        renderHandlerTimerId_ =
          itimer_manager_p->schedule_timer (&renderHandler_,                    // event handler handle
                                            NULL,                               // asynchronous completion token
                                            COMMON_TIME_NOW + refresh_interval, // first wakeup time
                                            refresh_interval);                  // interval
        if (unlikely (renderHandlerTimerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      &refresh_interval));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled renderer dispatch (timer id: %d)\n"),
                    inherited::mod_->name (),
                    renderHandlerTimerId_));

        inherited::threadCount_ = 1;
        inherited::start (NULL);
        inherited::threadCount_ = 0;
        shutdown = true;
      } // end IF

      break;

error:
      if (renderHandlerTimerId_ != -1)
      {
        // sanity check(s)
        ACE_ASSERT (itimer_manager_p);

        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (renderHandlerTimerId_,
                                                 &act_p);
        if (unlikely (result <= 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      renderHandlerTimerId_));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: cancelled renderer dispatch (timer id: %d)\n"),
                      inherited::mod_->name (),
                      renderHandlerTimerId_));
        renderHandlerTimerId_ = -1;
      } // end IF
      if (shutdown)
        inherited::control (ACE_Message_Block::MB_STOP);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (renderHandlerTimerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (renderHandlerTimerId_,
                                                 &act_p);
        if (unlikely (result <= 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      renderHandlerTimerId_));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: cancelled renderer dispatch (timer id: %d)\n"),
                      inherited::mod_->name (),
                      renderHandlerTimerId_));
        renderHandlerTimerId_ = -1;
      } // end IF

      // *IMPORTANT NOTE*: at this stage no new data should be arriving
      //                   --> join with the renderer thread
      if (inherited::thr_count_ > 0)
      {
        Common_ITask* itask_p = this;
        itask_p->stop (true,   // wait ?
                       false); // high priority ?
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: joined renderer thread\n"),
                    inherited::mod_->name ()));
      } // end IF

//#if GTK_CHECK_VERSION(3,10,0)
//      if (cairoSurface_)
//      {
//        cairo_surface_destroy (cairoSurface_); cairoSurface_ = NULL;
//      } // end IF
//#else
//      if (pixelBuffer_)
//      {
//        g_object_unref (pixelBuffer_); pixelBuffer_ = NULL;
//      } // end IF
//#endif // GTK_CHECK_VERSION(3,10,0)
      if (cairoContext_)
      {
        cairo_destroy (cairoContext_); cairoContext_ = NULL;
      } // end IF
//#if defined (GTKGL_SUPPORT)
      //OpenGLTextureId_ = 0;
//      OpenGLWindow_ = NULL;
//#if GTK_CHECK_VERSION(3,0,0)
//#else
//#if defined (GTKGLAREA_SUPPORT)
//#else
//      OpenGLContext_ = NULL;
//#endif /* GTKGLAREA_SUPPORT */
//#endif /* GTK_CHECK_VERSION (3,0,0) */
//#endif /* GTKGL_SUPPORT */

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
          typename MediaType>
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
                                                  MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              inherited::grp_id_));

  int result = 0;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  int error = 0;

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
        update ();
      // *WARNING*: control falls through
      default:
      {
        message_block_p->release (); message_block_p = NULL;
        break;
      }
    } // end SWITCH
  } while (true);
  result = -1;

done:
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
          typename MediaType>
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
                                                  MediaType>::stop (bool waitForCompletion_in,
                                                                    bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::stop"));

  ACE_UNUSED_ARG (highPriority_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = this->putq (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::putq(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF

  if (likely (waitForCompletion_in))
  {
    result = inherited::TASK_T::wait ();
    if (unlikely (result == -1))
         ACE_DEBUG ((LM_ERROR,
                     ACE_TEXT ("%s: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                     inherited::mod_->name ()));
  } // end IF
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
          typename MediaType>
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
                                                  MediaType>::initialize_Cairo (GdkWindow* window_in,
                                                                                cairo_t*& cairoContext_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo"));

  // sanity check(s)
  ACE_ASSERT (window_in);
  ACE_ASSERT (!cairoContext_out);

  // initialize return value(s)
  cairoContext_out = NULL;

  int width, height;
  //gdk_threads_enter ();
  width = gdk_window_get_width (window_in);
  height = gdk_window_get_height (window_in);

//#if GTK_CHECK_VERSION(3,10,0)
//  if (!cairoSurface_out)
//#else
//  if (!pixelBuffer_out)
//#endif // GTK_CHECK_VERSION(3,10,0)
//  {
//#if GTK_CHECK_VERSION(3,10,0)
//    cairoSurface_out =
////#if GTK_CHECK_VERSION(3,10,0)
//        gdk_window_create_similar_image_surface (window_in,
//                                                 CAIRO_FORMAT_RGB24,
//                                                 width, height,
//                                                 1);
////#else
////        gdk_window_create_similar_surface (window_in,
////                                           CAIRO_CONTENT_COLOR_ALPHA,
////                                           width, height);
////    cairo_surface_type_t type = cairo_surface_get_type (cairoSurface_out);
////    ACE_ASSERT (type == CAIRO_SURFACE_TYPE_IMAGE);
////#endif // GTK_CHECK_VERSION(3,10,0)
//    if (unlikely (!cairoSurface_out))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to gdk_window_create_similar_surface(), aborting\n"),
//                  inherited::mod_->name ()));
//      return false;
//    } // end IF
//    ACE_ASSERT (cairo_image_surface_get_height (cairoSurface_out) == height);
//    ACE_ASSERT (cairo_image_surface_get_width (cairoSurface_out) == width);
//#else
//    pixelBuffer_out =
//#if GTK_CHECK_VERSION(3,0,0)
//        gdk_pixbuf_get_from_window (window_in,
//                                    0, 0, width, height);
//#else
//        gdk_pixbuf_get_from_drawable (NULL,
//                                      GDK_DRAWABLE (window_in),
//                                      NULL,
//                                      0, 0,
//                                      0, 0, width, height);
//#endif // GTK_CHECK_VERSION(3,0,0)
//    if (!pixelBuffer_out)
//    {
//#if GTK_CHECK_VERSION(3,0,0)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
//                  inherited::mod_->name ()));
//#else
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_drawable(), aborting\n"),
//                  inherited::mod_->name ()));
//#endif // GTK_CHECK_VERSION(3,0,0)
//      return false;
//    } // end IF
//    ACE_ASSERT (gdk_pixbuf_get_height (pixelBuffer_out) == height);
//    ACE_ASSERT (gdk_pixbuf_get_width (pixelBuffer_out) == width);
//#endif // GTK_CHECK_VERSION(3,10,0)
//  } // end IF
//#if GTK_CHECK_VERSION(3,10,0)
//  ACE_ASSERT (cairoSurface_out);
//  cairoContext_out = cairo_create (cairoSurface_out);
//  if (unlikely (!cairoContext_out))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to cairo_create(), aborting\n"),
//                inherited::mod_->name ()));
//    return false;
//  } // end IF
//#else
//  ACE_ASSERT (pixelBuffer_out);
  cairoContext_out = gdk_cairo_create (window_in);
//  cairoContext_out = cairo_create ();
  if (unlikely (!cairoContext_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_cairo_create(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
//#endif // GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (cairoContext_out);
  //ACE_ASSERT (cairo_status (cairoContext_out) == CAIRO_STATUS_SUCCESS);

//#if GTK_CHECK_VERSION(3,10,0)
//  cairo_set_source_surface (cairoContext_out,
//                            cairoSurface_out,
//                            0.0, 0.0);
//#else
//  //gdk_cairo_set_source_pixbuf (cairoContext_out,
//  //                             pixelBuffer_out,
//  //                             0.0, 0.0);
//#endif // GTK_CHECK_VERSION(3,10,0)

  //cairo_set_line_cap (cairoContext_out, CAIRO_LINE_CAP_BUTT);
  cairo_set_line_width (cairoContext_out, 1.0);
  //cairo_set_line_join (cairoContext_out, CAIRO_LINE_JOIN_MITER);
  //cairo_set_dash (cairoContext_out, NULL, 0, 0.0);

  return true;
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
          typename MediaType>
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
                                                  MediaType>::dispatch (const enum Stream_Statistic_AnalysisEventType& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::dispatch"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

#if defined (GTKGL_SUPPORT)
  // sanity check(s)
  ACE_ASSERT (inherited::configuration_->OpenGLInstructionsLock);
  ACE_ASSERT (inherited::configuration_->OpenGLInstructions);

  struct Stream_Visualization_OpenGL_Instruction opengl_instruction;

  switch (event_in)
  {
    case STREAM_STATISTIC_ANALYSIS_EVENT_ACTIVITY:
    {
//#if GTK_CHECK_VERSION(3,0,0)
//      foregroundColor_.red   = randomGenerator_ () / 255.0;
//      foregroundColor_.green = randomGenerator_ () / 255.0;
//      foregroundColor_.blue  = randomGenerator_ () / 255.0;
//      //foregroundColor_.alpha = ;
//#else
//      foregroundColor_.red   = randomGenerator_ ();
//      foregroundColor_.green = randomGenerator_ ();
//      foregroundColor_.blue  = randomGenerator_ ();
//      //foregroundColor_.alpha = ;
//#endif // GTK_CHECK_VERSION(3,0,0)
//      opengl_instruction.color = foregroundColor_;
//      opengl_instruction.type =
//        STREAM_VISUALIZATION_OPENGL_INSTRUCTION_SET_COLOR_FG;
      opengl_instruction.type =
        STREAM_VISUALIZATION_OPENGL_INSTRUCTION_CHANGE_ROTATION;
      break;
    }
    case STREAM_STATISTIC_ANALYSIS_EVENT_PEAK:
    {
#if GTK_CHECK_VERSION(3,0,0)
      backgroundColor_.red   = randomGenerator_ () / 255.0;
      backgroundColor_.green = randomGenerator_ () / 255.0;
      backgroundColor_.blue  = randomGenerator_ () / 255.0;
      //backgroundColor_.alpha = ;
#else
      backgroundColor_.red   = randomGenerator_ ();
      backgroundColor_.green = randomGenerator_ ();
      backgroundColor_.blue  = randomGenerator_ ();
      //backgroundColor_.alpha = ;
#endif // GTK_CHECK_VERSION(3,0,0)
      opengl_instruction.color = backgroundColor_;
      opengl_instruction.type =
        STREAM_VISUALIZATION_OPENGL_INSTRUCTION_SET_COLOR_BG;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown event (was: %d), returning\n"),
                  inherited::mod_->name (),
                  event_in));
      return;
    }
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *inherited::configuration_->OpenGLInstructionsLock);
    inherited::configuration_->OpenGLInstructions->push_back (opengl_instruction);
  } // end lock scope
#endif // GTKGL_SUPPORT
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
          typename MediaType>
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
                                                  MediaType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::reset"));

  // trigger a render update
  // *NOTE*: (as long as it is single thread-based,) rendering a frame creates
  //         too much workload for the timer dispatch context and delays the
  //         dispatch of (relatively more important other) scheduled tasks
  //         --> avoid 'laggy' applications
  // *TODO*: depending on the platform (and the timer dispatch 'mode'), this may
  //         be unnecessary (i.e. if the timer mechanism is signal-handler
  //         based (, or the timer dispatch uses a thread pool itself))
  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_EVENT,        // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = this->putq (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::putq(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
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
          typename MediaType>
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
                                                  MediaType>::setP (GdkWindow* window_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::setP"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (!inherited::sessionData_)
    return;
  ACE_ASSERT (window_in);

  int result = -1;
  bool release_lock = false;
  unsigned int sound_sample_size = 0;
  bool is_floating_point_format = false;
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
  ACE_ASSERT (!session_data_r.formats.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited2::getMediaType (session_data_r.formats.back (),
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
  MediaType media_type_s;
  inherited2::getMediaType (session_data_r.formats.back (),
                            media_type_s);
  sound_sample_size = (snd_pcm_format_width (media_type_s.format) / 8);
  is_floating_point_format =
    (snd_pcm_format_real (media_type_s.format) == 1);
#endif // ACE_WIN32 || ACE_WIN64

  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    inherited::configuration_->window = window_in;
    if (likely (cairoContext_))
    {
      cairo_destroy (cairoContext_); cairoContext_ = NULL;
    } // end IF
    if (unlikely (!initialize_Cairo (window_in,
                                     cairoContext_)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize_Cairo(), returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    ACE_ASSERT (cairoContext_);

    gdk_window_get_geometry (window_in,
                             NULL,
                             NULL,
                             &width_,
                             &height_,
                             NULL);
    ACE_ASSERT (height_); ACE_ASSERT (width_);
    halfHeight_ = height_ / 2;

    channelFactor_ = width_ / static_cast<double> (inherited3::channels_);
    scaleFactorX_ =
      width_ / static_cast<double> (inherited3::channels_ * inherited3::slots_);
    scaleFactorX_2 =
      width_ / static_cast<double> (inherited3::channels_ * ((inherited3::slots_ / 2) - 1));
    scaleFactorY_ =
      (is_floating_point_format ? static_cast<double> (height_) / 2.0
                                : static_cast<double> (height_) / static_cast<double> (Common_Tools::max<ACE_UINT64> (sound_sample_size, false)));
  } // end lock scope
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
          typename MediaType>
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
                                                  MediaType>::update ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::update"));

  ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);

  // sanity check(s)
  ACE_ASSERT (cairoContext_);
  ACE_ASSERT (mode2D_);

#define CAIRO_ERROR_WORKAROUND(X)                                        \
 if (cairo_status (X) != CAIRO_STATUS_SUCCESS) {                         \
   cairo_destroy (cairoContext_); cairoContext_ = NULL;                  \
   cairoContext_ = gdk_cairo_create (inherited::configuration_->window); \
   ACE_ASSERT (cairoContext_);                                           \
   cairo_set_line_width (cairoContext_, 1.0);                            \
 } // end IF

  // step1: clear the window(s)
  if (*mode2D_ < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX)
  {
    cairo_set_source_rgb (cairoContext_, 0.0, 0.0, 0.0);
    cairo_rectangle (cairoContext_, 0.0, 0.0, width_, height_);
    cairo_fill (cairoContext_);
  } // end IF

  // step2a: draw signal graphics
  for (unsigned int i = 0; i < inherited3::channels_; ++i)
  {
    switch (*mode2D_)
    {
      case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE:
      {
        // step2aa: draw a thin, green polyline
        cairo_set_source_rgb (cairoContext_, 0.0, 1.0, 0.0);
        cairo_move_to (cairoContext_,
                       static_cast<double> (i) * channelFactor_,
                       static_cast<double> (halfHeight_));
        for (unsigned int j = 0; j < inherited3::slots_; ++j)
          cairo_line_to (cairoContext_,
                         (static_cast<double> (i) * channelFactor_) + (static_cast<double> (j) * scaleFactorX_),
                         static_cast<double> (halfHeight_) - (inherited3::buffer_[i][j] * scaleFactorY_));
        break;
      }
      case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM:
      {
        double x = 0.0;
        // step2aa: draw thin, white columns
        cairo_set_source_rgb (cairoContext_, 1.0, 1.0, 1.0);
        // *IMPORTANT NOTE*: - the first ('DC'-)slot does not contain frequency
        //                     information --> j = 1
        //                   - the slots N/2 - N are mirrored and do not contain
        //                     additional information
        //                     --> there are only N/2 - 1 meaningful values
        for (unsigned int j = 1; j < inherited3::halfSlots_; ++j)
        {
          x =
            (static_cast<double> (i) * channelFactor_) + (static_cast<double> (j) * scaleFactorX_2);
          cairo_move_to (cairoContext_,
                         x,
                         static_cast<double> (height_));
          // *NOTE*: it is 2x the scale factor for signed (!) values, because the
          //         magnitudes are absolute values
          cairo_line_to (cairoContext_,
                         x,
                         static_cast<double> (height_) - (inherited3::Magnitude (j, i, true) * 2.0 * scaleFactorY_));
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
                    mode2D_));
        return;
      }
    } // end SWITCH
    cairo_stroke (cairoContext_);

    // step2ab: draw a thin red line between channels
    if (i > 0)
    {
      cairo_set_source_rgb (cairoContext_, 1.0, 0.0, 0.0);
      cairo_move_to (cairoContext_,
                     i * channelFactor_,
                     0);
      cairo_line_to (cairoContext_,
                     i * channelFactor_,
                     height_);
      cairo_stroke (cairoContext_);
    } // end IF
  } // end FOR
#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_mark_dirty (cairoSurface_);
  //ACE_ASSERT (cairo_status (cairoSurface_) == CAIRO_STATUS_SUCCESS);
  cairo_surface_flush (cairoSurface_);
  //ACE_ASSERT (cairo_status (cairoSurface_) == CAIRO_STATUS_SUCCESS);
#endif // GTK_CHECK_VERSION(3,10,0)
  // *IMPORTANT NOTE*: this assert fails intermittently on Gtk2 Win32;
  //                   the result is CAIRO_STATUS_NO_MEMORY
  //ACE_ASSERT (cairo_status (cairoContext_) == CAIRO_STATUS_SUCCESS);
  CAIRO_ERROR_WORKAROUND (cairoContext_);

  //gdk_threads_enter ();
  //gdk_window_invalidate_rect (inherited::configuration_->window,
  //                            NULL,
  //                            false);
  //gdk_threads_leave ();
}
