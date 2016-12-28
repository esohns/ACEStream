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
#include <alsa/asoundlib.h>
#endif

#include <ace/Log_Msg.h>

#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dev_tools.h"

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ()
 : inherited ()
 , inherited2 (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_CHANNELS,
               MODULE_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE,
               MODULE_VIS_SPECTRUMANALYZER_DEFAULT_SAMPLE_RATE)
 , cairoContext2D_ (NULL)
#if GTK_CHECK_VERSION (3,10,0)
 , cairoSurface2D_ (NULL)
#else
 , lock_ (NULL)
 , pixelBuffer2D_ (NULL)
#endif
#if defined (GTKGL_SUPPORT)
 , backgroundColor_ ()
 , foregroundColor_ ()
 , OpenGLContext_ (NULL)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
 , OpenGLWindow_ (NULL)
#endif
#else
 , OpenGLWindow_ (NULL)
#endif
 , OpenGLTextureID_ (0)
#endif
 , channelFactor_ (0.0)
 , scaleFactorX_ (0.0)
 , scaleFactorY_ (0.0)
 , height_ (0)
 , width_ (0)
 , mode2D_ (NULL)
#if defined (GTKGL_SUPPORT)
 , mode3D_ (NULL)
#endif
 , renderHandler_ (this)
 , renderHandlerTimerID_ (-1)
 , sampleIterator_ (NULL)
#if GTK_CHECK_VERSION (3,0,0)
 , randomDistribution_ (0, 255)
#else
 , randomDistribution_ (0, 65535)
#endif
 , randomEngine_ ()
 , randomGenerator_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T"));

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
  backgroundColor_ = { 0.0, 0.0, 0.0, 1.0 }; // opaque black
  foregroundColor_ = { 1.0, 1.0, 1.0, 1.0 }; // opaque white
#else
  backgroundColor_ = { 0, 0, 0, 0 };             // black
  foregroundColor_ = { 0, 65535, 65535, 65535 }; // white
#endif
#endif

  randomGenerator_ = std::bind (randomDistribution_, randomEngine_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::~Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::~Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T"));

#if GTK_CHECK_VERSION (3,10,0)
  if (cairoSurface2D_)
    cairo_surface_destroy (cairoSurface2D_);
#else
  if (pixelBuffer2D_)
    g_object_unref (pixelBuffer2D_);
#endif
  if (cairoContext2D_)
    cairo_destroy (cairoContext2D_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::initialize"));

//  int result = -1;

  if (inherited::isInitialized_)
  {
    // (re-)activate() the message queue
    // *NOTE*: as this is a 'passive' object, the queue needs to be explicitly
    //         (re-)activate()d (see below)
    inherited::msg_queue (NULL);

#if GTK_CHECK_VERSION (3,10,0)
    if (cairoSurface2D_)
    {
      cairo_surface_destroy (cairoSurface2D_);
      cairoSurface2D_ = NULL;
    } // end IF
#else
    lock_ = NULL;
    if (pixelBuffer2D_)
    {
      g_object_unref (pixelBuffer2D_);
      pixelBuffer2D_ = NULL;
    } // end IF
#endif
    if (cairoContext2D_)
    {
      cairo_destroy (cairoContext2D_);
      cairoContext2D_ = NULL;
    } // end IF
#if defined (GTKGL_SUPPORT)
    OpenGLContext_ = NULL;
#if GTK_CHECK_VERSION (3,0,0)
    backgroundColor_ = { 0.0, 0.0, 0.0, 1.0 }; // opaque black
    foregroundColor_ = { 1.0, 1.0, 1.0, 1.0 }; // opaque white
#if GTK_CHECK_VERSION (3,16,0)
#else
    OpenGLWindow_ = NULL;
#endif
#else
    backgroundColor_ = { 0, 0, 0, 0 };                // black
    foregroundColor_ = { 0, 065535, 065535, 065535 }; // white
    OpenGLWindow_ = NULL;
#endif
    OpenGLTextureID_ = 0;
#endif

    height_ = width_ = 0;

    mode2D_ = NULL;
#if defined (GTKGL_SUPPORT)
    mode3D_ = NULL;
#endif
  } // end IF

#if GTK_CHECK_VERSION (3,10,0)
  //lock_ = configuration_in.cairoSurfaceLock;

  if (configuration_in.cairoSurface2D)
    cairoSurface2D_ =
      cairo_surface_reference (configuration_in.cairoSurface2D);
#else
  lock_ = configuration_in.pixelBufferLock;

  if (configuration_in.pixelBuffer2D)
  {
    g_object_ref (configuration_in.pixelBuffer2D);
    pixelBuffer2D_ = configuration_in.pixelBuffer2D;
  } // end IF
#endif

#if defined (GTKGL_SUPPORT)
  OpenGLContext_ = configuration_in.OpenGLContext;
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
  OpenGLWindow_ = configuration_in.GdkWindow3D;
#endif
#else
  OpenGLWindow_ = configuration_in.GdkWindow3D;
#endif
  OpenGLTextureID_ = configuration_in.OpenGLTextureID;
#endif

  mode2D_ =
    &const_cast<ConfigurationType&> (configuration_in).spectrumAnalyzer2DMode;
#if defined (GTKGL_SUPPORT)
  mode3D_ =
    &const_cast<ConfigurationType&> (configuration_in).spectrumAnalyzer3DMode;
  if (!mode2D_ || !mode3D_)
#else
  if (!mode2D_)
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid mode, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  if (configuration_in.GdkWindow2D)
  {
#if GTK_CHECK_VERSION (3,10,0)
    if (!initialize_Cairo (configuration_in.GdkWindow2D,
                           cairoContext2D_,
                           cairoSurface2D_))
#else
    if (!initialize_Cairo (configuration_in.GdkWindow2D,
                           cairoContext2D_,
                           pixelBuffer2D_))
#endif
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo(), aborting\n")));
      return false;
    } // end IF

#if GTK_CHECK_VERSION (3,10,0)
    ACE_ASSERT (cairoSurface2D_);

    height_ = cairo_image_surface_get_height (cairoSurface2D_);
    width_ = cairo_image_surface_get_width (cairoSurface2D_);
#else
    ACE_ASSERT (pixelBuffer2D_);

    height_ = gdk_pixbuf_get_height (pixelBuffer2D_);
    width_ = gdk_pixbuf_get_width (pixelBuffer2D_);
#endif
    ACE_ASSERT (height_);
    ACE_ASSERT (width_);
  } // end IF

#if defined (GTKGL_SUPPORT)
  if (configuration_in.OpenGLContext)
  {
    ACE_ASSERT (OpenGLTextureID_ > 0);
  } // end IF
#endif

  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_TaskBaseSynch_T::initialize(), aborting\n")));
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType* media_type_p =
    getFormat (configuration_in.format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve media type, returning\n")));
    return false;
  } // end IF
  ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);

  ACE_ASSERT (media_type_p->pbFormat);
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
  ACE_ASSERT (waveformatex_p);
#endif

  // *TODO*: remove type inferences
  // sanity check(s)
  ACE_ASSERT (configuration_in.format);

  bool result_2 = false;
  unsigned int channels, sample_rate;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  channels = waveformatex_p->nChannels;
  sample_rate = waveformatex_p->nSamplesPerSec;
#else
  channels = configuration_in.format->channels;
  sample_rate = configuration_in.format->rate;
#endif
  result_2 =
    inherited2::Initialize (channels,
                            configuration_in.spectrumAnalyzerResolution,
                            sample_rate);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (media_type_p)
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
#endif

  // *TODO*: remove type inference
  const_cast<ConfigurationType&> (configuration_in).dispatch = this;

  return result_2;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // step1: process inbound samples
  // *NOTE*: a 'data sample' consists of #channel 'sound sample's, which may
  //         arrive interleaved (i.e. (16 bit resolution) LLRRLLRRLLRR...), or
  //         'chunked' (i.e. LL...LLRR...RR...|LL...LLRR...RR...|...) at some
  //         interval. Note how 'chunked' data may need to be split/assembled
  //         before processing
  // *NOTE*: n-byte (n<1) 'sound sample's may have non-platform endian encoding

  // *TODO*: the current implementation assumes that all multi-channel data is
  //         interleaved like so: LR...LR... and arrives unfragmented, i.e. that
  //         inbound messages contain only complete samples. However, this
  //         largely depends on upstream configuration (i.e. data may be
  //         arriving over the network) and the hardware/driver/source module
  //         implementation
  ACE_ASSERT (message_inout->length () % sampleIterator_.dataSampleSize_ == 0);
  ACE_ASSERT (message_inout->length () % sampleIterator_.soundSampleSize_ == 0);

  unsigned int number_of_samples =
    message_inout->length () / sampleIterator_.dataSampleSize_;
  unsigned int samples_to_write = 0;
  unsigned int offset = 0;
  unsigned int tail_slot = 0;
  sampleIterator_.buffer_ = message_inout->rd_ptr ();

  // sanity check(s)
  ACE_ASSERT (mode2D_);

  do
  {
    samples_to_write =
      (number_of_samples > inherited2::slots_ ? inherited2::slots_
                                              : number_of_samples);
    sampleIterator_.buffer_ = message_inout->rd_ptr () + offset;
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
                       tail_slot * sizeof (double));

      // copy the sample data to the tail end of the buffer, transform to double
      for (unsigned int j = 0; j < samples_to_write; ++j)
        inherited2::buffer_[i][tail_slot + j] = sampleIterator_.get (j, i);
      offset += (sampleIterator_.soundSampleSize_ * samples_to_write);

      // step1b: process sample data
      if (*mode2D_ > STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE)
      {
        // initialize the FFT working set buffer, transform to complex
        for (unsigned int j = 0; j < inherited2::slots_; ++j)
          X_[i][bitReverseMap_[j]] = std::complex<double> (buffer_[i][j]);

        // compute FFT
        inherited2::Compute (i);
      } // end IF
    } // end FOR
    number_of_samples -= samples_to_write;
    if (number_of_samples == 0) break; // done
  } while (true);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::handleSessionMessage"));

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
          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      bool result_2 = false;
      bool shutdown = false;

      unsigned int data_sample_size = 0;
      unsigned int sound_sample_size = 0;
      unsigned int channels, sample_rate;
//      bool is_signed_format = false;
      int sample_byte_order = ACE_BYTE_ORDER;
//      unsigned int maximum_value = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType* media_type_p = NULL;
      media_type_p = getFormat (session_data_r.format);
      if (!media_type_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve media type, returning\n")));
        return;
      } // end IF
      ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);
      ACE_ASSERT (media_type_p->pbFormat);

      // *NOTE*: apparently, all Win32 sound data is signed 16 bits
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
      ACE_ASSERT (waveformatex_p);
      data_sample_size = waveformatex_p->nBlockAlign;
      sound_sample_size = (data_sample_size * 8) /
                          waveformatex_p->wBitsPerSample;
//      // *NOTE*: Microsoft(TM) uses signed little endian
//      is_signed_format = true;
      sample_byte_order = ACE_LITTLE_ENDIAN;

      channels = waveformatex_p->nChannels;
      sample_rate = waveformatex_p->nSamplesPerSec;

      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
#else
      data_sample_size =
        ((snd_pcm_format_width (session_data_r.format.format) / 8) *
          session_data_r.format.channels);
      sound_sample_size = data_sample_size /
        session_data_r.format.channels;
//      is_signed_format = snd_pcm_format_signed (session_data_r.format.format);
      sample_byte_order =
          ((snd_pcm_format_little_endian (session_data_r.format.format) == 1) ? ACE_LITTLE_ENDIAN
                                                                              : -1);

      channels = session_data_r.format.channels;
      sample_rate = session_data_r.format.rate;
#endif
      result_2 = sampleIterator_.initialize (data_sample_size,
                                             sound_sample_size,
                                             true,
                                             sample_byte_order);
      if (!result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize sample iterator, aborting\n")));
        goto error;
      } // end IF

      result_2 =
        inherited2::Initialize (channels,
                                inherited::configuration_->spectrumAnalyzerResolution,
                                sample_rate);
      if (!result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_Math_FFT::initialize(), aborting\n")));
        goto error;
      } // end IF

      channelFactor_ = width_ / static_cast<double> (inherited2::channels_);
      scaleFactorX_ =
          width_ / static_cast<double> (inherited2::channels_ * inherited2::slots_);
      scaleFactorY_ =
          height_ / static_cast<double> (((1 << (sound_sample_size * 8)) - 1));

      // schedule the renderer
      if (inherited::configuration_->fps)
      {
        ACE_ASSERT (inherited::msg_queue_ == &(inherited::queue_));
        result = inherited::msg_queue_->activate ();
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
          goto error;
        } // end IF

        ACE_ASSERT (renderHandlerTimerID_ == -1);
        Common_Timer_Manager_t* timer_manager_p =
            COMMON_TIMERMANAGER_SINGLETON::instance ();
        ACE_ASSERT (timer_manager_p);
        // schedule the second-granularity timer
        ACE_Time_Value refresh_interval (0, 1000000 / inherited::configuration_->fps);
        ACE_Event_Handler* event_handler_p = &renderHandler_;
        renderHandlerTimerID_ =
          timer_manager_p->schedule_timer (event_handler_p,                    // event handler
                                           NULL,                               // ACT
                                           COMMON_TIME_NOW + refresh_interval, // first wakeup time
                                           refresh_interval);                  // interval
        if (renderHandlerTimerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      &refresh_interval));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled renderer dispatch (ID: %d)...\n"),
                    inherited::mod_->name (),
                    renderHandlerTimerID_));

        inherited::start ();
        shutdown = true;
      } // end IF

      break;

error:
      if (renderHandlerTimerID_ != -1)
      {
        const void* act_p = NULL;
        Common_ITimer* itimer_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
        ACE_ASSERT (itimer_p);
        result = itimer_p->cancel_timer (renderHandlerTimerID_,
                                         &act_p);
        if (result <= 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      renderHandlerTimerID_));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: cancelled renderer dispatch (ID: %d)...\n"),
                      ACE_TEXT (inherited::name ()),
                      renderHandlerTimerID_));
        renderHandlerTimerID_ = -1;
      } // end IF
      if (shutdown)
        inherited::stop (false); // wait ?

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (renderHandlerTimerID_ != -1)
      {
        const void* act_p = NULL;
        Common_ITimer* itimer_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
        ACE_ASSERT (itimer_p);
        result = itimer_p->cancel_timer (renderHandlerTimerID_,
                                         &act_p);
        if (result <= 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      renderHandlerTimerID_));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: cancelled renderer dispatch (ID: %d)...\n"),
                      ACE_TEXT (inherited::name ()),
                      renderHandlerTimerID_));
        renderHandlerTimerID_ = -1;
      } // end IF

      // *IMPORTANT NOTE*: at this stage no new data should be arriving (i.e.
      //                   the DirectShow graph should have stopped
      //                   --> join with the renderer thread
      if (inherited::thr_count_ > 0)
      {
        inherited::stop (true); // wait ?
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: joined renderer thread...\n"),
                    inherited::mod_->name ()));
      } // end IF

#if GTK_CHECK_VERSION (3,10,0)
      if (cairoSurface2D_)
      {
        cairo_surface_destroy (cairoSurface2D_);
        cairoSurface2D_ = NULL;
      } // end IF
#else
      if (pixelBuffer2D_)
      {
        g_object_unref (pixelBuffer2D_);
        pixelBuffer2D_ = NULL;
      } // end IF
#endif
      if (cairoContext2D_)
      {
        cairo_destroy (cairoContext2D_);
        cairoContext2D_ = NULL;
      } // end IF
#if defined (GTKGL_SUPPORT)
      OpenGLContext_ = NULL;
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
      OpenGLWindow_ = NULL;
#endif
#else
      OpenGLWindow_ = NULL;
#endif
      OpenGLTextureID_ = 0;
#endif

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
          typename SessionDataContainerType>
int
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (ID: %t) starting...\n"),
              inherited::mod_->name ()));

  int result = 0;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  int error = 0;

  // process update events
  do
  {
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p, NULL);
    if (result_2 == -1)
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
        // clean up
        message_block_p->release ();
        message_block_p = NULL;

        goto done;
      }
      case ACE_Message_Block::MB_EVENT:
        update ();
      default:
      {
        // clean up
        message_block_p->release ();
        message_block_p = NULL;

        break;
      }
    } // end SWITCH
  } while (true);
  result = -1;

done:
  return result;
}

#if GTK_CHECK_VERSION (3,10,0)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::initialize_Cairo (GdkWindow* window_in,
                                                                                            cairo_t*& cairoContext_out,
                                                                                            cairo_surface_t*& cairoSurface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo"));

  ACE_ASSERT (window_in);
  ACE_ASSERT (!cairoContext_out);
  //ACE_ASSERT (!cairoSurface_out);

  // initialize return value(s)
  cairoContext_out = NULL;
  //cairoSurface_out = NULL;

  int width, height;

  //gdk_threads_enter ();

  width = gdk_window_get_width (window_in);
  height = gdk_window_get_height (window_in);

  if (!cairoSurface_out)
  {
    cairoSurface_out =
#if GTK_CHECK_VERSION (3,10,0)
        gdk_window_create_similar_image_surface (window_in,
                                                 CAIRO_FORMAT_RGB24,
                                                 width, height,
                                                 1);
#else
        gdk_window_create_similar_surface (window_in,
                                           CAIRO_CONTENT_COLOR_ALPHA,
                                           width, height);
//    cairo_surface_type_t type = cairo_surface_get_type (cairoSurface_out);
//    ACE_ASSERT (type == CAIRO_SURFACE_TYPE_IMAGE);
#endif
    if (!cairoSurface_out)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gdk_window_create_similar_surface(), aborting\n")));

      // clean up
      //gdk_threads_leave ();

      return false;
    } // end IF
    ACE_ASSERT (cairo_image_surface_get_height (cairoSurface_out) == height);
    ACE_ASSERT (cairo_image_surface_get_width (cairoSurface_out) == width);
  } // end IF

  cairoContext_out = cairo_create (cairoSurface_out);
  if (!cairoContext_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to cairo_create(), aborting\n")));

    // clean up
    //gdk_threads_leave ();

    return false;
  } // end IF
  cairo_status_t status = cairo_status (cairoContext_out);
  ACE_ASSERT (status == CAIRO_STATUS_SUCCESS);

  cairo_set_source_surface (cairoContext_out,
                            cairoSurface_out,
                            0.0, 0.0);

  cairo_set_line_cap (cairoContext_out, CAIRO_LINE_CAP_BUTT);
  cairo_set_line_width (cairoContext_out, 1.0);
  cairo_set_line_join (cairoContext_out, CAIRO_LINE_JOIN_MITER);
  cairo_set_dash (cairoContext_out, NULL, 0, 0.0);

  //gdk_threads_leave ();

  return true;
}
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::initialize_Cairo (GdkWindow* window_in,
                                                                                            cairo_t*& cairoContext_out,
                                                                                            GdkPixbuf*& pixelBuffer_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo"));

  ACE_ASSERT (window_in);
  ACE_ASSERT (!cairoContext_out);
  //ACE_ASSERT (!pixelBuffer_out);

  // initialize return value(s)
  cairoContext_out = NULL;
  //pixelBuffer_out = NULL;

  int width, height;

  //gdk_threads_enter ();

  width = gdk_window_get_width (window_in);
  height = gdk_window_get_height (window_in);

  if (!pixelBuffer_out)
  {
    pixelBuffer_out =
        gdk_pixbuf_get_from_drawable (NULL,
                                      GDK_DRAWABLE (window_in),
                                      NULL,
                                      0, 0,
                                      0, 0, width, height);
    if (!pixelBuffer_out)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gdk_pixbuf_get_from_drawable(), aborting\n")));

      // clean up
      //gdk_threads_leave ();

      return false;
    } // end IF
    ACE_ASSERT (gdk_pixbuf_get_height (pixelBuffer_out) == height);
    ACE_ASSERT (gdk_pixbuf_get_width (pixelBuffer_out) == width);
  } // end IF

  cairoContext_out = gdk_cairo_create (window_in);
//  cairoContext_out = cairo_create ();
  if (!cairoContext_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));

    // clean up
    //gdk_threads_leave ();

    return false;
  } // end IF
  cairo_status_t status = cairo_status (cairoContext_out);
  ACE_ASSERT (status == CAIRO_STATUS_SUCCESS);

  gdk_cairo_set_source_pixbuf (cairoContext_out,
                               pixelBuffer_out,
                               0.0, 0.0);

  cairo_set_line_cap (cairoContext_out, CAIRO_LINE_CAP_BUTT);
  cairo_set_line_width (cairoContext_out, 1.0);
  cairo_set_line_join (cairoContext_out, CAIRO_LINE_JOIN_MITER);
  cairo_set_dash (cairoContext_out, NULL, 0, 0.0);

  //gdk_threads_leave ();

  return true;
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::dispatch (const Stream_Module_StatisticAnalysis_Event& event_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::dispatch"));

  switch (event_in)
  {
    case STREAM_MODULE_STATISTICANALYSIS_EVENT_ACTIVITY:
    {
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
      foregroundColor_.red   = randomGenerator_ () / 255.0;
      foregroundColor_.green = randomGenerator_ () / 255.0;
      foregroundColor_.blue  = randomGenerator_ () / 255.0;
      //foregroundColor_.alpha = ;
#else
      foregroundColor_.red   = randomGenerator_ ();
      foregroundColor_.green = randomGenerator_ ();
      foregroundColor_.blue  = randomGenerator_ ();
      //foregroundColor_.alpha = ;
#endif
#endif
      break;
    }
    case STREAM_MODULE_STATISTICANALYSIS_EVENT_PEAK:
    {
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
      backgroundColor_.red   = randomGenerator_ () / 255.0;
      backgroundColor_.green = randomGenerator_ () / 255.0;
      backgroundColor_.blue  = randomGenerator_ () / 255.0;
      //backgroundColor_.alpha = ;
#else
      backgroundColor_.red   = randomGenerator_ ();
      backgroundColor_.green = randomGenerator_ ();
      backgroundColor_.blue  = randomGenerator_ ();
      //backgroundColor_.alpha = ;
#endif
#endif
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
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::reset"));

  // trigger a render update
  // *NOTE*: (as long as it is single thread-based,) rendering a frame creates
  //         too much workload for the timer dispatch context and delays the
  //         dispatch of (relatively more important other) scheduled tasks
  //         --> avoid 'laggy' applications
  // *TODO*: depending on the platform (and the timer dispatch 'mode'), this may
  //         be unnecessary (i.e. if the timer mechanism is signal-handler
  //         based (, or the timer dispatch uses a thread pool itself))
  inherited::control (ACE_Message_Block::MB_EVENT);
}

#if GTK_CHECK_VERSION (3,10,0)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::set (cairo_surface_t* surface_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::set"));

  // sanity check(s)
  ACE_ASSERT (surface_in);

}
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::set (GdkPixbuf* pixelBuffer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::set"));

  // sanity check(s)
  ACE_ASSERT (pixelBuffer_in);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());

  int result = -1;
  bool release_lock = false;
  if (lock_)
  {
    result = lock_->acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
    else
      release_lock = true;
  } // end IF

  if (pixelBuffer2D_)
  {
    g_object_unref (pixelBuffer2D_);
    pixelBuffer2D_ = NULL;
  } // end IF
  g_object_ref (pixelBuffer_in);
  pixelBuffer2D_ = pixelBuffer_in;

  if (cairoContext2D_)
  {
    gdk_cairo_set_source_pixbuf (cairoContext2D_,
                                 pixelBuffer2D_,
                                 0.0, 0.0);
    cairo_reset_clip (cairoContext2D_);
  } // end IF

  unsigned int data_sample_size = 0;
  unsigned int sound_sample_size = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType* media_type_p = NULL;
  media_type_p = getFormat (session_data_r.format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve media type, returning\n")));
    return;
  } // end IF
  ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);
  ACE_ASSERT (media_type_p->pbFormat);

  // *NOTE*: apparently, all Win32 sound data is signed 16 bits
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
  ACE_ASSERT (waveformatex_p);
  data_sample_size = waveformatex_p->nBlockAlign;
  sound_sample_size = (data_sample_size * 8) /
                       waveformatex_p->wBitsPerSample;

  Stream_Module_Device_Tools::deleteMediaType (media_type_p);
#else
  data_sample_size =
    ((snd_pcm_format_width (session_data_r.format.format) / 8) *
      session_data_r.format.channels);
  sound_sample_size = data_sample_size /
    session_data_r.format.channels;
#endif

  height_ = gdk_pixbuf_get_height (pixelBuffer2D_);
  width_ = gdk_pixbuf_get_width (pixelBuffer2D_);

  channelFactor_ = width_ / static_cast<double> (inherited2::channels_);
  scaleFactorX_ =
      width_ / static_cast<double> (inherited2::channels_ * inherited2::slots_);
  scaleFactorY_ =
      height_ / static_cast<double> (((1 << (sound_sample_size * 8)) - 1));

//unlock:
  if (release_lock)
  {
    ACE_ASSERT (lock_);
    result = lock_->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::update ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::update"));

  // sanity check(s)
  ACE_ASSERT (mode2D_);
#if defined (GTKGL_SUPPORT)
  ACE_ASSERT (mode3D_);
#endif

  int result = -1;
#if GTK_CHECK_VERSION (3,10,0)
#else
  bool release_lock = false;
  if (lock_)
  {
    result = lock_.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
    else
      release_lock = true;
  } // end IF
#endif

  double half_height = height_ / 2.0;
  double x = 0.0;

  if (!cairoContext2D_)
    goto continue_;

  // step1: wipe the window(s)
  if (*mode2D_ < STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_MAX)
  {
    cairo_set_source_rgb (cairoContext2D_, 0.0, 0.0, 0.0);
    cairo_rectangle (cairoContext2D_, 0.0, 0.0, width_, height_);
    cairo_fill (cairoContext2D_);
  } // end IF

  // step2a: draw signal graphics
  for (unsigned int i = 0; i < inherited2::channels_; ++i)
  {
    switch (*mode2D_)
    {
      case STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE:
      {
        // step2aa: draw a thin, green polyline
        cairo_set_source_rgb (cairoContext2D_, 0.0, 1.0, 0.0);
        cairo_move_to (cairoContext2D_,
                       i * channelFactor_, half_height);
        for (unsigned int j = 0; j < inherited2::slots_; ++j)
          cairo_line_to (cairoContext2D_,
                         (i * channelFactor_) + (j * scaleFactorX_),
                         half_height - (inherited2::buffer_[i][j] * scaleFactorY_));
        cairo_stroke (cairoContext2D_);
        break;
      }
      case STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_SPECTRUM:
      {
        // step2aa: draw thin, white columns
        cairo_set_source_rgb (cairoContext2D_, 1.0, 1.0, 1.0);
        for (unsigned int j = 0; j < inherited2::slots_; ++j)
        {
          x = (i * channelFactor_) + (j * scaleFactorX_);
          cairo_move_to (cairoContext2D_,
                         x,
                         height_);
          cairo_line_to (cairoContext2D_,
                         x,
                         height_ - (inherited2::Intensity (j, i) * scaleFactorY_));
        } // end FOR
        cairo_stroke (cairoContext2D_);
        break;
      }
      case STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_INVALID:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid 2D mode (was: %d), continuing\n"),
                    mode2D_));
        goto continue_;
      }
    } // end SWITCH
  } // end FOR
//  cairo_surface_mark_dirty (cairoSurface2D_);
//  cairo_surface_flush (cairoSurface2D_);

continue_:
#if defined (GTKGL_SUPPORT)
  if (!OpenGLContext_ || (OpenGLTextureID_ == 0))
    goto unlock;

  // step2b: draw OpenGL graphics
  switch (*mode3D_)
  {
    case STREAM_MODULE_VIS_SPECTRUMANALYZER_3DMODE_DEFAULT:
    {
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
      gdk_gl_context_make_current (OpenGLContext_);
#else
      ggla_make_current (OpenGLWindow_,
                         OpenGLContext_);
#endif
#else
#if defined (GTKGLAREA_SUPPORT)
      gdk_gl_make_current (OpenGLWindow_,
                           OpenGLContext_);
#else
      gdk_gl_drawable_make_current (OpenGLWindow_,
                                    OpenGLContext_);
#endif
#endif

#if GTK_CHECK_VERSION (3,0,0)
      glClearColor ((GLclampf)backgroundColor_.red,
                    (GLclampf)backgroundColor_.green,
                    (GLclampf)backgroundColor_.blue,
                    1.0F);
      glColor4f ((GLclampf)foregroundColor_.red,
                 (GLclampf)foregroundColor_.green,
                 (GLclampf)foregroundColor_.blue,
                 1.0F);
#else
      glClearColor ((GLclampf)backgroundColor_.red   / 65535.0F,
                    (GLclampf)backgroundColor_.green / 65535.0F,
                    (GLclampf)backgroundColor_.blue  / 65535.0F,
                    1.0F);
      glColor4f (foregroundColor_.red   / 65535.0F,
                 foregroundColor_.green / 65535.0F,
                 foregroundColor_.blue  / 65535.0F,
                 1.0F);
#endif
      break;
    }
    case STREAM_MODULE_VIS_SPECTRUMANALYZER_3DMODE_INVALID:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid OpenGL mode (was: %d), continuing\n"),
                  mode3D_));
      goto unlock;
    }
  } // end SWITCH
#endif

unlock:
#if GTK_CHECK_VERSION (3,10,0)
#else
  if (release_lock)
  {
    result = lock_.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF
#endif
  return;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
AM_MEDIA_TYPE*
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;
  if (!Stream_Module_Device_Tools::copyMediaType (*format_in,
                                                  result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::copyMediaType(), aborting\n")));
    return NULL;
  } // end IF
  ACE_ASSERT (result_p);

  return result_p;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
AM_MEDIA_TYPE*
Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;

  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (format_in),
                                        GUID_NULL,
                                        &result_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (result_p);

  return result_p;
}
#endif
