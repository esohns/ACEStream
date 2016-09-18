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
#endif

#include "ace/Log_Msg.h"

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
 , cairoContext_ (NULL)
 , cairoSurface_ (NULL)
 , channelFactor_ (0.0)
 , scaleFactorX_ (0.0)
 , scaleFactorY_ (0.0)
 , height_ (0)
 , width_ (0)
 , lock_ (NULL)
 , mode_ (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_MODE)
 , renderHandler_ (this)
 , renderHandlerTimerID_ (-1)
 , sampleIterator_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T"));

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

  if (cairoSurface_)
    cairo_surface_destroy (cairoSurface_);
  if (cairoContext_)
    cairo_destroy (cairoContext_);
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

  if (inherited::isInitialized_)
  {
    if (cairoSurface_)
    {
      cairo_surface_destroy (cairoSurface_);
      cairoSurface_ = NULL;
    } // end IF
    if (cairoContext_)
    {
      cairo_destroy (cairoContext_);
      cairoContext_ = NULL;
    } // end IF

    height_ = width_ = 0;
    lock_ = NULL;
    mode_ = MODULE_VIS_SPECTRUMANALYZER_DEFAULT_MODE;
  } // end IF

  lock_ = configuration_in.cairoSurfaceLock;
  mode_ = configuration_in.spectrumAnalyzerMode;

  if (configuration_in.cairoSurface)
    cairoSurface_ = cairo_surface_reference (configuration_in.cairoSurface);

  if (configuration_in.gdkWindow)
  {
    if (!initialize_Cairo (configuration_in.gdkWindow,
                           cairoContext_,
                           cairoSurface_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::initialize_Cairo(), aborting\n")));
      return false;
    } // end IF
    ACE_ASSERT (cairoSurface_);

    height_ = cairo_image_surface_get_height (cairoSurface_);
    width_ = cairo_image_surface_get_width (cairoSurface_);
    ACE_ASSERT (height_);
    ACE_ASSERT (width_);
  } // end IF

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

  bool result = false;
  unsigned int channels, sample_rate;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  channels = waveformatex_p->nChannels;
  sample_rate = waveformatex_p->nSamplesPerSec;
#else
  channels = configuration_in.format->channels;
  sample_rate = configuration_in.format->rate;
#endif
  result =
    inherited2::Initialize (channels,
                            configuration_in.spectrumAnalyzerResolution,
                            sample_rate);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (media_type_p)
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
#endif

  return result;
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

  // sanity check(s)
  if (!cairoContext_)
    return;
  ACE_ASSERT (cairoSurface_);

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

  int result = 0;
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
      // copy the sample data to the tail end of the buffer
      for (unsigned int j = 0;
           j < samples_to_write;
           ++j, offset += sampleIterator_.soundSampleSize_)
        inherited2::buffer_[i][tail_slot + j] = sampleIterator_.get (j, i);

      // step1b: process sample data
      //if (mode_ > STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_MODE_OSCILLOSCOPE)
      //{
        // initialize the FFT working set buffer
        for (unsigned int j = 0; j < inherited2::slots_; ++j)
          X_[i][bitReverseMap_[j]] = std::complex<double> (buffer_[i][j]);

        // compute FFT
        inherited2::Transform (i);
      //} // end IF
    } // end FOR
    number_of_samples -= samples_to_write;

    if (result == -1)
      goto error;

    if (number_of_samples == 0) break; // done

    continue;

error:
    //if (leave_gdk)
    //  gdk_threads_leave ();

    break;
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

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      bool result = false;
      bool shutdown = false;

      unsigned int data_sample_size = 0;
      unsigned int sound_sample_size = 0;
      unsigned int channels, sample_rate;
      int sample_byte_order = ACE_BYTE_ORDER;
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

      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
      ACE_ASSERT (waveformatex_p);
      data_sample_size = waveformatex_p->nBlockAlign;
      sound_sample_size = (data_sample_size * 8) /
                          waveformatex_p->wBitsPerSample;
      // *NOTE*: apparently, all Win32 sound data is little endian only
      sample_byte_order = ACE_LITTLE_ENDIAN;

      channels = waveformatex_p->nChannels;
      sample_rate = waveformatex_p->nSamplesPerSec;

      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
#else
      data_sample_size =
        (snd_pcm_format_width (session_data_r.format.format) *
          session_data_r.format.channels);
      sound_sample_size = data_sample_size /
        session_data_r.format.channels;
      sample_byte_order =
          ((snd_pcm_format_little_endian (session_data_r.format.format) == 1) ? ACE_LITTLE_ENDIAN
                                                                              : -1);

      channels = session_data_r.format.channels;
      sample_rate = session_data_r.format.rate;
#endif
      // *NOTE*: apparently, all Win32 sound data is signed 16 bits
      result = sampleIterator_.initialize (data_sample_size,
                                           sound_sample_size,
                                           true,
                                           sample_byte_order);
      if (!result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize sample iterator, aborting\n")));
        goto error;
      } // end IF

      result =
        inherited2::Initialize (channels,
                                inherited::configuration_->spectrumAnalyzerResolution,
                                sample_rate);
      if (!result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_Math_FFT::initialize(), aborting\n")));
        goto error;
      } // end IF

      scaleFactorX_ =
        width_ / static_cast<double> (inherited2::channels_ * inherited2::slots_);
      // *TODO*: this works for signed (!) 16 bit samples only
      scaleFactorY_ =
        height_ / static_cast<double> (pow (2, sampleIterator_.soundSampleSize_ * 8) / 2);
      channelFactor_ = width_ / static_cast<double> (inherited2::channels_);

      // schedule the renderer
      if (inherited::configuration_->fps)
      { ACE_ASSERT (renderHandlerTimerID_ == -1);
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
                    ACE_TEXT (inherited::name ()),
                    renderHandlerTimerID_));

        if (!inherited::activate ())
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_TaskBase_T::activate(): \"%m\", aborting\n")));
          goto error;
        } // end IF
        shutdown = true;
      } // end IF

      break;

error:
      if (renderHandlerTimerID_ != -1)
      {
        const void* act_p = NULL;
        Common_ITimer* itimer_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
        ACE_ASSERT (itimer_p);
        int result = itimer_p->cancel_timer (renderHandlerTimerID_,
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
        inherited::shutdown ();

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
        int result = itimer_p->cancel_timer (renderHandlerTimerID_,
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
      inherited::shutdown ();

      if (cairoSurface_)
      {
        cairo_surface_destroy (cairoSurface_);
        cairoSurface_ = NULL;
      } // end IF
      if (cairoContext_)
      {
        cairo_destroy (cairoContext_);
        cairoContext_ = NULL;
      } // end IF

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
  //ACE_ASSERT (inherited::sessionData_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": worker thread (ID: %t) starting...\n"),
              inherited::mod_->name ()));

  int error = 0;
  ACE_Message_Block* message_block_p = NULL;
  int result = 0;
  int result_2 = -1;
  //const SessionDataType& session_data_r = inherited::sessionData_->get ();
  //  unsigned int queued, done = 0;

  // step1: (re-)activate() the message queue
  // *NOTE*: as this basically is a passive object, the queue needs to be
  //         explicitly (re-)open()ed
  result = inherited::queue_.activate ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // step2: process update events
  do
  {
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p, NULL);
    if (result_2 == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
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
      default:
      {
        // clean up
        message_block_p->release ();
        message_block_p = NULL;

        update ();

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

  gdk_threads_enter ();

  width = gdk_window_get_width (window_in);
  height = gdk_window_get_height (window_in);

  if (!cairoSurface_out)
  {
    cairoSurface_out =
      gdk_window_create_similar_surface (window_in,
                                         CAIRO_CONTENT_COLOR,
                                         width, height);
    if (!cairoSurface_out)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gdk_window_create_similar_surface(), aborting\n")));

      // clean up
      gdk_threads_leave ();

      return false;
    } // end IF
    ACE_ASSERT (cairo_image_surface_get_height (cairoSurface_) == height);
    ACE_ASSERT (cairo_image_surface_get_width (cairoSurface_) == width);
  } // end IF

  cairoContext_out = cairo_create (cairoSurface_out);
  if (!cairoContext_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to cairo_create(), aborting\n")));

    // clean up
    gdk_threads_leave ();

    return false;
  } // end IF
  cairo_status_t status = cairo_status (cairoContext_out);
  ACE_ASSERT (status == CAIRO_STATUS_SUCCESS);

  cairo_set_line_cap (cairoContext_, CAIRO_LINE_CAP_BUTT);
  cairo_set_line_width (cairoContext_, 1.0);
  cairo_set_line_join (cairoContext_, CAIRO_LINE_JOIN_MITER);
  cairo_set_dash (cairoContext_, NULL, 0, 0.0);

  gdk_threads_leave ();

  return true;
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
                                               SessionDataContainerType>::update ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::update"));

  // sanity check(s)
  ACE_ASSERT (cairoContext_);

  //int result_2 = -1;
//  bool release_lock = false;

  // step1: wipe the window
  cairo_set_source_rgb (cairoContext_, 0.0, 0.0, 0.0);
  cairo_rectangle (cairoContext_, 0.0, 0.0, width_, height_);
  cairo_fill (cairoContext_);

  //if (lock_)
  //{
  //  result_2 = lock_->acquire ();
  //  if (result_2 == -1)
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
  //  else
  //    release_lock = true;
  //} // end IF
  //cairo_surface_flush (cairoSurface_);

  // step1: draw graphics
  for (unsigned int i = 0; i < inherited2::channels_; ++i)
  {
    switch (mode_)
    {
      case STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_MODE_OSCILLOSCOPE:
      {
        //// debug info
        //static unsigned int color_counter = 0;
        //cairo_set_source_rgb (cairoContext_,
        //                      static_cast<double> (++color_counter % 256) * (1.0 / 255.0),
        //                      static_cast<double> (color_counter % 256) * (1.0 / 255.0),
        //                      static_cast<double> (color_counter % 256) * (1.0 / 255.0));
        //cairo_rectangle (cairoContext_, 0.0, 0.0, 10, 10);
        //cairo_fill (cairoContext_);

        // step2ba: draw a thin, green polyline
        cairo_set_source_rgb (cairoContext_, 0.0, 1.0, 0.0);
        cairo_move_to (cairoContext_, i * channelFactor_, height_ / 2);
        for (unsigned int j = 0; j < inherited2::slots_; ++j)
          cairo_line_to (cairoContext_,
                         (i * channelFactor_) + (j * scaleFactorX_),
                         (height_ / 2) - (inherited2::buffer_[i][j] * scaleFactorY_));
        cairo_stroke (cairoContext_);

        //cairo_surface_mark_dirty (cairoSurface_);

        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid mode (was: %d), continuing\n"),
                    mode_));
        goto unlock;
      }
    } // end SWITCH
  } // end FOR

unlock:
  //if (release_lock)
  //{
  //  ACE_ASSERT (lock_);
  //  result_2 = lock_->release ();
  //  if (result_2 == -1)
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n")));
  //  release_lock = false;
  //} // end IF

  // step5: schedule an 'expose' event
  // *NOTE*: gdk_window_invalidate_rect() is not thread-safe. It will race with
  //         the UI refresh and eventually crash (even though gdk_threads_enter/
  //         gdk_threads_leave is in effect)
  //         --> schedule a refresh with gtk_widget_queue_draw_area() instead
  // *NOTE*: this does not work either (same reason ?)...
  //         --> let the downstream event handler queue an idle request
  //gdk_threads_enter ();
  //gdk_window_invalidate_rect (inherited::configuration_->gdkWindow,
  //                            NULL,
  //                            false);
  //gdk_threads_leave ();
  //  GtkWidget* widget_p = NULL;
  //  gdk_window_get_user_data (configuration_->window,
  //                            reinterpret_cast<gpointer*> (&widget_p));
  //  ACE_ASSERT (widget_p);
  //  gtk_widget_queue_draw (widget_p);
  //  GtkAllocation allocation;
  //  gtk_widget_get_allocation (widget_p,
  //                             &allocation);
  //  gtk_widget_queue_draw_area (widget_p,
  //                              allocation.x, allocation.y,
  //                              allocation.width, allocation.height);

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
