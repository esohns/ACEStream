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
#include "alsa/asoundlib.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
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
 , surfaceLock_ (NULL)
#if GTK_CHECK_VERSION(3,10,0)
 , cairoSurface_ (NULL)
#else
 , pixelBuffer_ (NULL)
#endif // GTK_CHECK_VERSION (3,10,0)
#if defined (GTKGL_SUPPORT)
 , OpenGLInstructions_ (NULL)
 , OpenGLInstructionsLock_ (NULL)
 //, OpenGLTextureId_ (0)
 , backgroundColor_ ()
 , foregroundColor_ ()
 //, OpenGLWindow_ (NULL)
#if GTK_CHECK_VERSION(3,0,0)
#else /* GTK_CHECK_VERSION (3,0,0) */
#if defined (GTKGLAREA_SUPPORT)
#else
 //, OpenGLContext_ (NULL)
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */
 , channelFactor_ (0.0)
 , scaleFactorX_ (0.0)
 , scaleFactorY_ (0.0)
 , height_ (0)
 , width_ (0)
 , mode2D_ (NULL)
#if defined (GTKGL_SUPPORT)
 , mode3D_ (NULL)
#endif // GTKGL_SUPPORT
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

#if GTK_CHECK_VERSION(3,10,0)
  if (unlikely (cairoSurface_))
    cairo_surface_destroy (cairoSurface_);
#else
  if (unlikely (pixelBuffer_))
    g_object_unref (pixelBuffer_);
#endif // GTK_CHECK_VERSION(3,10,0)
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

    surfaceLock_ = NULL;
#if GTK_CHECK_VERSION(3,10,0)
    if (cairoSurface_)
    {
      cairo_surface_destroy (cairoSurface_); cairoSurface_ = NULL;
    } // end IF
#else
    if (pixelBuffer_)
    {
      g_object_unref (pixelBuffer_); pixelBuffer_ = NULL;
    } // end IF
#endif // GTK_CHECK_VERSION (3,10,0)
    if (cairoContext_)
    {
      cairo_destroy (cairoContext_); cairoContext_ = NULL;
    } // end IF
#if defined (GTKGL_SUPPORT)
    OpenGLInstructions_ = NULL;
    OpenGLInstructionsLock_ = NULL;
    //OpenGLTextureId_ = 0;
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
    //OpenGLWindow_ = NULL;
//#if GTK_CHECK_VERSION(3,0,0)
//#else
//#if defined (GTKGLAREA_SUPPORT)
//#else
//    OpenGLContext_ = NULL;
//#endif /* GTKGLAREA_SUPPORT */
//#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

    height_ = width_ = 0;

    mode2D_ = NULL;
#if defined (GTKGL_SUPPORT)
    mode3D_ = NULL;
#endif // GTKGL_SUPPORT
  } // end IF

  mode2D_ =
    &const_cast<ConfigurationType&> (configuration_in).spectrumAnalyzer2DMode;

  // initialize cairo ?
  if (!configuration_in.window)
    goto continue_;

  surfaceLock_ = configuration_in.surfaceLock;
#if GTK_CHECK_VERSION(3,10,0)
  if (configuration_in.cairoSurface2D)
    cairoSurface_ =
      cairo_surface_reference (configuration_in.cairoSurface2D);
#else
  if (configuration_in.pixelBuffer2D)
  {
    g_object_ref (configuration_in.pixelBuffer2D);
    pixelBuffer_ = configuration_in.pixelBuffer2D;
  } // end IF
#endif // GTK_CHECK_VERSION(3,10,0)

  // *TODO*: remove type inferences
#if GTK_CHECK_VERSION(3,10,0)
  if (unlikely (!initialize_Cairo (configuration_in.window,
                                   cairoContext_,
                                   cairoSurface_)))
#else
  if (unlikely (!initialize_Cairo (configuration_in.window,
                                   cairoContext_,
                                   pixelBuffer_)))
#endif // GTK_CHECK_VERSION(3,10,0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (cairoSurface_);

  height_ = cairo_image_surface_get_height (cairoSurface_);
  width_ = cairo_image_surface_get_width (cairoSurface_);
#else
  ACE_ASSERT (pixelBuffer_);

  height_ = gdk_pixbuf_get_height (pixelBuffer_);
  width_ = gdk_pixbuf_get_width (pixelBuffer_);
#endif // GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (height_); ACE_ASSERT (width_);

continue_:
#if defined (GTKGL_SUPPORT)
  mode3D_ =
    &const_cast<ConfigurationType&> (configuration_in).spectrumAnalyzer3DMode;
#endif // GTKGL_SUPPORT

#if defined (GTKGL_SUPPORT)
  // initialize OpenGL ?
  //if (!configuration_in.OpenGLWindow)
  //  goto continue_2;

  OpenGLInstructions_ = configuration_in.OpenGLInstructions;
  OpenGLInstructionsLock_ = configuration_in.OpenGLInstructionsLock;
  //OpenGLTextureId_ = configuration_in.OpenGLTextureId;
//  OpenGLWindow_ = configuration_in.OpenGLWindow;
//#if GTK_CHECK_VERSION(3,0,0)
//#else /* GTK_CHECK_VERSION (3,0,0) */
//#if defined (GTKGLAREA_SUPPORT)
//#else
//  OpenGLContext_ = configuration_in.OpenGLContext;
//#endif /* GTKGLAREA_SUPPORT */
//#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

#if defined (GTKGL_SUPPORT)
  //if (OpenGLWindow_)
  //{
  //  //ACE_ASSERT (OpenGLTextureId_ > 0);
  //} // end IF
//#if GTK_CHECK_VERSION(3,0,0)
//#else /* GTK_CHECK_VERSION (3,0,0) */
//#if defined (GTKGLAREA_SUPPORT)
//#else
//  if (OpenGLContext_)
//  {
//    //ACE_ASSERT (OpenGLTextureId_ > 0);
//  } // end IF
//#endif /* GTKGLAREA_SUPPORT */
//#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

#if defined (GTKGL_SUPPORT)
//continue_2:
#endif /* GTKGL_SUPPORT */
#if defined (_DEBUG)
#if defined (GTKGL_SUPPORT)
  if (unlikely (!mode2D_ && !mode3D_))
#else
  if (unlikely (!mode2D_))
#endif // GTKGL_SUPPORT
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: no graphics output\n"),
                inherited::mod_->name ()));
#endif // _DEBUG

  if (unlikely (!inherited::initialize (configuration_in,
                                        allocator_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBaseSynch_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  struct _AMMediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            media_type_s);
  ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (media_type_s.pbFormat);
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
#endif // ACE_WIN32 || ACE_WIN64

  bool result_2 = false;
  unsigned int channels, sample_rate;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (waveformatex_p);
  channels = waveformatex_p->nChannels;
  sample_rate = waveformatex_p->nSamplesPerSec;

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
  struct Stream_MediaFramework_ALSA_MediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            media_type_s);
  channels = media_type_s.channels;
  sample_rate = media_type_s.rate;
#endif // ACE_WIN32 || ACE_WIN64
  result_2 =
    inherited3::Initialize (channels,
                            configuration_in.spectrumAnalyzerResolution,
                            sample_rate);

  return result_2;
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
      (number_of_samples > inherited3::slots_ ? inherited3::slots_
                                              : number_of_samples);
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
          X_[i][bitReverseMap_[j]] = std::complex<double> (buffer_[i][j]);

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
//      bool is_signed_format = false;
      int sample_byte_order = ACE_BYTE_ORDER;
//      unsigned int maximum_value = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.front (),
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      ACE_ASSERT (media_type_s.pbFormat);

      // *NOTE*: apparently, all Win32 sound data is signed 16 bits
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);
      data_sample_size = waveformatex_p->nBlockAlign;
      sound_sample_size = (data_sample_size * 8) /
                          waveformatex_p->wBitsPerSample;
//      // *NOTE*: Microsoft(TM) uses signed little endian
//      is_signed_format = true;
      sample_byte_order = ACE_LITTLE_ENDIAN;

      channels = waveformatex_p->nChannels;
      sample_rate = waveformatex_p->nSamplesPerSec;

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.front (),
                                media_type_s);
      data_sample_size =
        ((snd_pcm_format_width (media_type_s.format) / 8) *
          media_type_s.channels);
      sound_sample_size = data_sample_size / media_type_s.channels;
//      is_signed_format = snd_pcm_format_signed (session_data_r.inputFormat.format);
      sample_byte_order =
          ((snd_pcm_format_little_endian (media_type_s.format) == 1) ? ACE_LITTLE_ENDIAN
                                                                     : -1);

      channels = media_type_s.channels;
      sample_rate = media_type_s.rate;
#endif // ACE_WIN32 || ACE_WIN64
      result_2 = sampleIterator_.initialize (data_sample_size,
                                             sound_sample_size,
                                             true,
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
      scaleFactorY_ =
          height_ / static_cast<double> (((1 << (sound_sample_size * 8)) - 1));

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
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled renderer dispatch (timer id: %d)\n"),
                    inherited::mod_->name (),
                    renderHandlerTimerId_));
#endif // _DEBUG

        ACE_thread_t thread_id = 0;
        inherited::start (thread_id);
        ACE_UNUSED_ARG (thread_id);
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
        inherited::stop (false); // wait ?

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
#if defined (_DEBUG)
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: cancelled renderer dispatch (timer id: %d)\n"),
                      inherited::mod_->name (),
                      renderHandlerTimerId_));
#endif // _DEBUG
        renderHandlerTimerId_ = -1;
      } // end IF

      // *IMPORTANT NOTE*: at this stage no new data should be arriving (i.e.
      //                   the DirectShow graph should have stopped
      //                   --> join with the renderer thread
      if (inherited::thr_count_ > 0)
      {
        inherited::stop (true,  // wait ?
                         true); // N/A
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: joined renderer thread\n"),
                    inherited::mod_->name ()));
#endif // _DEBUG
      } // end IF

#if GTK_CHECK_VERSION(3,10,0)
      if (cairoSurface_)
      {
        cairo_surface_destroy (cairoSurface_); cairoSurface_ = NULL;
      } // end IF
#else
      if (pixelBuffer_)
      {
        g_object_unref (pixelBuffer_); pixelBuffer_ = NULL;
      } // end IF
#endif // GTK_CHECK_VERSION(3,10,0)
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

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (ID: %t) starting\n"),
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
                                                                                cairo_t*& cairoContext_out,
#if GTK_CHECK_VERSION(3,10,0)
                                                                                cairo_surface_t*& cairoSurface_out)
#else
                                                                                GdkPixbuf*& pixelBuffer_out)
#endif // GTK_CHECK_VERSION(3,10,0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo"));

  // sanity check(s)
  ACE_ASSERT (window_in);
  ACE_ASSERT (!cairoContext_out);
#if GTK_CHECK_VERSION(3,10,0)
  //ACE_ASSERT (!cairoSurface_out);
#else
  //ACE_ASSERT (!pixelBuffer_out);
#endif // GTK_CHECK_VERSION(3,10,0)

  // initialize return value(s)
  cairoContext_out = NULL;
#if GTK_CHECK_VERSION(3,10,0)
  //cairoSurface_out = NULL;
#else
  //pixelBuffer_out = NULL;
#endif // GTK_CHECK_VERSION(3,10,0)

  int width, height;
  //gdk_threads_enter ();
  width = gdk_window_get_width (window_in);
  height = gdk_window_get_height (window_in);

#if GTK_CHECK_VERSION(3,10,0)
  if (!cairoSurface_out)
#else
  if (!pixelBuffer_out)
#endif // GTK_CHECK_VERSION(3,10,0)
  {
#if GTK_CHECK_VERSION(3,10,0)
    cairoSurface_out =
//#if GTK_CHECK_VERSION(3,10,0)
        gdk_window_create_similar_image_surface (window_in,
                                                 CAIRO_FORMAT_RGB24,
                                                 width, height,
                                                 1);
//#else
//        gdk_window_create_similar_surface (window_in,
//                                           CAIRO_CONTENT_COLOR_ALPHA,
//                                           width, height);
//    cairo_surface_type_t type = cairo_surface_get_type (cairoSurface_out);
//    ACE_ASSERT (type == CAIRO_SURFACE_TYPE_IMAGE);
//#endif // GTK_CHECK_VERSION(3,10,0)
    if (unlikely (!cairoSurface_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to gdk_window_create_similar_surface(), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
    ACE_ASSERT (cairo_image_surface_get_height (cairoSurface_out) == height);
    ACE_ASSERT (cairo_image_surface_get_width (cairoSurface_out) == width);
#else
    pixelBuffer_out =
#if GTK_CHECK_VERSION(3,0,0)
        gdk_pixbuf_get_from_window (window_in,
                                    0, 0, width, height);
#else
        gdk_pixbuf_get_from_drawable (NULL,
                                      GDK_DRAWABLE (window_in),
                                      NULL,
                                      0, 0,
                                      0, 0, width, height);
#endif // GTK_CHECK_VERSION(3,0,0)
    if (!pixelBuffer_out)
    {
#if GTK_CHECK_VERSION(3,0,0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_window(), aborting\n"),
                  inherited::mod_->name ()));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to gdk_pixbuf_get_from_drawable(), aborting\n"),
                  inherited::mod_->name ()));
#endif // GTK_CHECK_VERSION(3,0,0)
      return false;
    } // end IF
    ACE_ASSERT (gdk_pixbuf_get_height (pixelBuffer_out) == height);
    ACE_ASSERT (gdk_pixbuf_get_width (pixelBuffer_out) == width);
#endif // GTK_CHECK_VERSION(3,10,0)
  } // end IF
#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (cairoSurface_out);
  cairoContext_out = cairo_create (cairoSurface_out);
  if (unlikely (!cairoContext_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to cairo_create(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
#else
  ACE_ASSERT (pixelBuffer_out);
  cairoContext_out = gdk_cairo_create (window_in);
//  cairoContext_out = cairo_create ();
  if (unlikely (!cairoContext_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to gdk_cairo_create(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
#endif // GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (cairoContext_out);
  ACE_ASSERT (cairo_status (cairoContext_out) == CAIRO_STATUS_SUCCESS);

#if GTK_CHECK_VERSION(3,10,0)
  cairo_set_source_surface (cairoContext_out,
                            cairoSurface_out,
                            0.0, 0.0);
#else
  gdk_cairo_set_source_pixbuf (cairoContext_out,
                               pixelBuffer_out,
                               0.0, 0.0);
#endif // GTK_CHECK_VERSION(3,10,0)

  cairo_set_line_cap (cairoContext_out, CAIRO_LINE_CAP_BUTT);
  cairo_set_line_width (cairoContext_out, 1.0);
  cairo_set_line_join (cairoContext_out, CAIRO_LINE_JOIN_MITER);
  cairo_set_dash (cairoContext_out, NULL, 0, 0.0);

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

  switch (event_in)
  {
    case STREAM_STATISTIC_ANALYSIS_EVENT_ACTIVITY:
    {
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,0,0)
      foregroundColor_.red   = randomGenerator_ () / 255.0;
      foregroundColor_.green = randomGenerator_ () / 255.0;
      foregroundColor_.blue  = randomGenerator_ () / 255.0;
      //foregroundColor_.alpha = ;
#else
      foregroundColor_.red   = randomGenerator_ ();
      foregroundColor_.green = randomGenerator_ ();
      foregroundColor_.blue  = randomGenerator_ ();
      //foregroundColor_.alpha = ;
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT
      break;
    }
    case STREAM_STATISTIC_ANALYSIS_EVENT_PEAK:
    {
#if defined (GTKGL_SUPPORT)
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
#endif // GTKGL_SUPPORT
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
  inherited::control (ACE_Message_Block::MB_EVENT);
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
#if GTK_CHECK_VERSION(3,10,0)
                                                  MediaType>::setP (cairo_surface_t* surface_in)
#else
                                                  MediaType>::setP (GdkPixbuf* pixelBuffer_in)
#endif // GTK_CHECK_VERSION (3,10,0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T::setP"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (surface_in);
#else
  ACE_ASSERT (pixelBuffer_in);
#endif // GTK_CHECK_VERSION (3,10,0)

  int result = -1;
  bool release_lock = false;
  unsigned int data_sample_size = 0;
  unsigned int sound_sample_size = 0;
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
#else
  MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64

  if (likely (surfaceLock_))
  {
    result = surfaceLock_->acquire ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    else
      release_lock = true;
  } // end IF

#if GTK_CHECK_VERSION (3,10,0)
  if (cairoSurface_)
    cairo_surface_destroy (cairoSurface_);
  cairoSurface_ = cairo_surface_reference (surface_in);
#else
  if (pixelBuffer_)
  {
    g_object_unref (pixelBuffer_); pixelBuffer_ = NULL;
  } // end IF
  g_object_ref (pixelBuffer_in);
  pixelBuffer_ = pixelBuffer_in;
#endif // GTK_CHECK_VERSION (3,10,0)

  if (cairoContext_)
  {
    cairo_destroy (cairoContext_); cairoContext_ = NULL;
  } // end IF
#if GTK_CHECK_VERSION (3,10,0)
  if (unlikely (!initialize_Cairo (NULL,
                                   cairoContext_,
                                   cairoSurface_)))
#else
  if (unlikely (!initialize_Cairo (NULL,
                                   cairoContext_,
                                   pixelBuffer_)))
#endif // GTK_CHECK_VERSION (3,10,0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize_Cairo(), aborting\n"),
                inherited::mod_->name ()));
    goto unlock;
  } // end IF
#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (cairoSurface_);

  height_ = cairo_image_surface_get_height (cairoSurface_);
  width_ = cairo_image_surface_get_width (cairoSurface_);
#else
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (cairoContext_);
  ACE_ASSERT (pixelBuffer_);
  ACE_ASSERT (!session_data_r.formats.empty ());

  gdk_cairo_set_source_pixbuf (cairoContext_,
                               pixelBuffer_,
                               0.0, 0.0);
  cairo_reset_clip (cairoContext_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited2::getMediaType (session_data_r.formats.front (),
                            media_type_s);
  ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (media_type_s.pbFormat);
  // *NOTE*: apparently, all Win32 sound data is signed 16 bits
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
  ACE_ASSERT (waveformatex_p);
  data_sample_size = waveformatex_p->nBlockAlign;
  sound_sample_size = (data_sample_size * 8) / waveformatex_p->wBitsPerSample;

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
  inherited2::getMediaType (session_data_r.formats.front (),
                            media_type_s);
  data_sample_size =
    ((snd_pcm_format_width (media_type_s.format) / 8) *
      media_type_s.channels);
  sound_sample_size = data_sample_size / media_type_s.channels;
#endif // ACE_WIN32 || ACE_WIN64

  height_ = gdk_pixbuf_get_height (pixelBuffer_);
  width_ = gdk_pixbuf_get_width (pixelBuffer_);

  channelFactor_ = width_ / static_cast<double> (inherited3::channels_);
  scaleFactorX_ =
      width_ / static_cast<double> (inherited3::channels_ * inherited3::slots_);
  scaleFactorY_ =
      height_ / static_cast<double> (((1 << (sound_sample_size * 8)) - 1));
#endif // GTK_CHECK_VERSION (3,10,0)
  ACE_ASSERT (height_); ACE_ASSERT (width_);

unlock:
  if (likely (release_lock))
  {
    ACE_ASSERT (surfaceLock_);
    result = surfaceLock_->release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
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

  // sanity check(s)
  ACE_ASSERT (mode2D_);
#if defined (GTKGL_SUPPORT)
  ACE_ASSERT (mode3D_);
#endif // GTKGL_SUPPORT

  int result = -1;
  bool release_lock = false;
  double half_height = height_ / 2.0;
  double x = 0.0;

  if (!cairoContext_)
    goto unlock;

  if (surfaceLock_)
  {
    result = surfaceLock_->acquire ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    else
      release_lock = true;
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
                       i * channelFactor_, half_height);
        for (unsigned int j = 0; j < inherited3::slots_; ++j)
          cairo_line_to (cairoContext_,
                         (i * channelFactor_) + (j * scaleFactorX_),
                         half_height - (inherited3::buffer_[i][j] * scaleFactorY_));
        cairo_stroke (cairoContext_);
        break;
      }
      case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM:
      {
        // step2aa: draw thin, white columns
        cairo_set_source_rgb (cairoContext_, 1.0, 1.0, 1.0);
        for (unsigned int j = 0; j < inherited3::slots_; ++j)
        {
          x = (i * channelFactor_) + (j * scaleFactorX_);
          cairo_move_to (cairoContext_,
                         x,
                         height_);
          cairo_line_to (cairoContext_,
                         x,
                         height_ - (inherited3::Intensity (j, i) * scaleFactorY_));
        } // end FOR
        cairo_stroke (cairoContext_);
        break;
      }
      case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid 2D mode (was: %d), continuing\n"),
                    inherited::mod_->name (),
                    mode2D_));
        goto unlock;
      }
    } // end SWITCH
  } // end FOR
#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_mark_dirty (cairoSurface_);
  cairo_surface_flush (cairoSurface_);
#endif // GTK_CHECK_VERSION(3,10,0)

unlock:
  if (release_lock)
  { ACE_ASSERT (surfaceLock_);
    result = surfaceLock_->release ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
  } // end IF

#if defined (GTKGL_SUPPORT)
  struct Stream_Visualization_OpenGL_Instruction opengl_instruction;

//#if GTK_CHECK_VERSION(3,0,0)
//  if (!OpenGLWindow_)// ||
//      //(OpenGLTextureId_ == 0))
//#else
//#if defined (GTKGLAREA_SUPPORT)
//  if (!OpenGLWindow_)// ||
//      //(OpenGLTextureId_ == 0))
//#else
//  if (!OpenGLContext_ ||
//      !OpenGLWindow_)//  ||
//      //(OpenGLTextureId_ == 0))
//#endif /* GTKGLAREA_SUPPORT */
//#endif /* GTK_CHECK_VERSION (3,0,0) */
//    goto continue_;

  // step2b: draw OpenGL graphics
  switch (*mode3D_)
  {
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT:
    {
      //gdk_threads_enter ();
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
      // *TODO*: (for reasons yet unkown,) gtk_gl_area_make_current() crashes.
      //         Most probably, it needs to be called from a (the) gtk context
      //         --> move everything OpenGL into a callback function that can be
      //             scheduled with g_idle_add()
      //gtk_gl_area_make_current (OpenGLWindow_);
      //gdk_gl_context_make_current (OpenGLContext_);
#else /* GTK_CHECK_VERSION (3,16,0) */
#if defined (GTKGLAREA_SUPPORT)
      //ggla_area_make_current (OpenGLWindow_);
#else
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else /* GTK_CHECK_VERSION (3,0,0) */
#if defined (GTKGLAREA_SUPPORT)
//      gdk_gl_make_current (OpenGLWindow_,
//                           OpenGLContext_);
//      ggla_area_make_current (OpenGLWindow_);
      //gtk_gl_area_make_current (OpenGLWindow_);
#else
      //gdk_gl_drawable_make_current (OpenGLWindow_,
      //                              OpenGLContext_);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */

      ACE_ASSERT (OpenGLInstructions_);
      ACE_ASSERT (OpenGLInstructionsLock_);
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *OpenGLInstructionsLock_);
        opengl_instruction.color = backgroundColor_;
        opengl_instruction.type =
          STREAM_VISUALIZATION_OPENGL_INSTRUCTION_SET_COLOR_BG;
        OpenGLInstructions_->push_back (opengl_instruction);
        opengl_instruction.color = foregroundColor_;
        opengl_instruction.type =
          STREAM_VISUALIZATION_OPENGL_INSTRUCTION_SET_COLOR_FG;
        OpenGLInstructions_->push_back (opengl_instruction);
      } // end lock scope
#else
      glClearColor (static_cast<GLclampf> (backgroundColor_.red)   / 65535.0F,
                    static_cast<GLclampf> (backgroundColor_.green) / 65535.0F,
                    static_cast<GLclampf> (backgroundColor_.blue)  / 65535.0F,
                    1.0F);
      glColor4f (static_cast<GLfloat> (foregroundColor_.red)   / 65535.0F,
                 static_cast<GLfloat> (foregroundColor_.green) / 65535.0F,
                 static_cast<GLfloat> (foregroundColor_.blue)  / 65535.0F,
                 1.0F);
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */
      //gdk_threads_leave ();

#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
#else /* GTK_CHECK_VERSION (3,16,0) */
      //ggla_area_swap_buffers (OpenGLWindow_);
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */
      break;
    }
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid OpenGL mode (was: %d), continuing\n"),
                  inherited::mod_->name (),
                  mode3D_));
      goto continue_;
    }
  } // end SWITCH
#endif /* GTKGL_SUPPORT */

#if defined (GTKGL_SUPPORT)
continue_:
#endif /* GTKGL_SUPPORT */

  return;
}
